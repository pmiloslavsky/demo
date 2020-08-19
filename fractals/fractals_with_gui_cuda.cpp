#include <math.h>
#include <signal.h>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <chrono>
#include <cmath>
#include <complex>
#include <ctime>
#include <future>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <utility>

#include "buddha_cuda_kernel.h"
#include "fractals.h"

// #include "tinycolormap.hpp"

// TODO
// More fractals:
//  Mandlebrot
//  Nebulabrot (3 colors)
//  Mandelbrot with Power parameter
//  Julia with power parameter and start parameter
//  Buddhabrot (one color)
//  Buddha with power (non cuda)
//  Nebulabrot cuda
//  Spiral Septagon
// Mandelbrot coloring improvements
//  Work on smooth coloring and colormaps more
// Select box with left mouse and display fractal inside box

using namespace std;

// The overall goal is to create some mildly zoomable models
// of fractals where its easy to see interesting regions

// Model:
//  Store 2D vector of checked points for size of zoomed and panned region
//  Gather hits from subthreads for buddhabrot
//

// Visual Elements (View):
// Zoomed region of the fractal
// Gui status elements
// Gui control elements (fractal selection)
//

// Controls:
//  Center via mouse click (mandelbrot only)
//  Zoom via mouse (mandelbrot only)
//  Hide/Show all GUI controls via hotkey
//  screenshot hotkey
//  full screen hotkey
//  cuda on hotkey

void signal_callback_handler(int signum) {
  cout << "Caught signal " << signum << endl;
  // Terminate program
  exit(signum);
}

// For really good pictures
const int IMAGE_WIDTH = 2560;
const int IMAGE_HEIGHT = 1600;

// const int IMAGE_WIDTH = 1600;
// const int IMAGE_HEIGHT = 1200;

// The zoom and rotation and panning of the fractal (mandelbrot only so far)
class ReferenceFrame {
 public:
  // x-y rotation angle
  float theta;

  // View
  //
  double xstart; //in fractal coordinates
  double ystart; //in fractal coordinates
  double xdelta; //delta per pixel in fractal coordinates
  double ydelta; //delta per pixel in fractal coordinates
  double current_width; //in pixels
  double current_height; //in pixels

  // Zoom Level
  double displayed_zoom;
  double requested_zoom;

  // Original total Pixels
  double original_width;
  double original_height;

  ReferenceFrame(float _thetaxy, double _zoom)
      : theta{_thetaxy}, displayed_zoom{_zoom} {};

};  // ReferenceFrame

ReferenceFrame R(0, 1.0);

// Submodel for different fractals
//#include "fractals.h" SupportedFractal
// struct SupportedFractal {
//   std::string name;
//   bool cuda_mode;
//   std::vector<double> xMinMax;  // x min max
//   std::vector<double> yMinMax;  // y min max
//   unsigned int color_scheme; //not yet used
//   std::vector<unsigned int> max_iters; //not yet used
// };

vector<SupportedFractal> FRAC = {
    {string("Mandelbrot"),
     false,
     false,
     false, //julia
     {-2.2, 1.0},
     {-1.2, 1.2},
     0, //colormap
     {256, 128, 32},
     2,
     complex<double>{0,0}
    },
    {string("Mandelbrot Pow"),
     false,
     false,
     false,
     {-2.2, 1.0},
     {-1.2, 1.2},
     0,
     {256, 128, 32},
     3,
     complex<double>{0,0}
    },
    {string("Julia Pow,Start"),
     false,
     false,
     true, //julia
     {-2.2, 2.2},
     {-1.2, 1.2},
     0,
     {256, 128, 32},
     2,
     complex<double>{-0.79,0.15}
    },
    {string("Spiral Septagon"),
     false,
     false,
     false,
     {-2.6, 2.6},
     {-1.4, 1.4},
     0, //colormap
     {256, 128, 32},
     7,
     complex<double>{0,0}
    },
    {string("Buddhabrot"),  // not going to be zoomable and pannable
     true,
     true,
     false,
     {-2.2, 1.0},
     {-1.2, 1.2},
     0,
     {10000, 1000, 100},
     2,
     complex<double>{0,0}
    },
    {string("Buddhabrot BW"),  // not going to be zoomable and pannable
     true,
     true,
     false,
     {-2.2, 1.0},
     {-1.2, 1.2},
     0,
     {10000, 10000, 10000},
     2,
     complex<double>{0,0}
    }
};





//#include "fractals.h" SampleStats
// struct SampleStats {
//   unsigned long long rejected; // skipInSet check
//   unsigned long long in_set;
//   unsigned long long escaped_set;
//   unsigned long long total;
//   //stats for thread efficiency and total thread progress
//   unsigned long long samples_per_second; //i.e. "total" - not hits
//   std::chrono::time_point<chrono::steady_clock> next_second_start;
//   unsigned long long samples_last_second;
// };

void mandelbrot_iterations_to_escape(double x, double y, unsigned int iters_max,
                                     int *p_rcolor, int *p_gcolor, int *p_bcolor,
                                     double power, complex<double> zconst, bool julia,
                                     unsigned long long &in,
                                     unsigned long long &out) {
  complex<double> point(x, y);
  complex<double> z(0, 0);
  unsigned int iter_ix = 0;

  if (julia)
    z = point;
  
  while (abs(z) < 2 && iter_ix <= iters_max) {
    if (julia)
      z = pow(z,power) + zconst; //With Julia you dont add Point
    else
      z = pow(z,power) + point;
    //z = z*z + point;
    iter_ix++;
  }

  if (iter_ix < iters_max)
    ++out;
  else
    ++in;

  if (iter_ix < iters_max){
    //    return (255 * iter_ix) / (iters_max - 1);
    //  smooth coloring ????    mu = N + 1 - log (log  |Z(N)|) / log 2
    //double smooth = ((iter_ix + 1 - log(log2(abs(z))))/iters_max; //0 -> 1
    
    // *p_rcolor = (255*iter_ix) / (iters_max);
    // *p_gcolor = 255 - (255*iter_ix) / (iters_max);
    // *p_bcolor = clamp(128 + (255*iter_ix) / (iters_max),(unsigned int)0,(unsigned int)255);

    //Ultra Fractal Default
    int i = iter_ix % 16;
    vector<vector<int>> mapping(16, std::vector<int>(3));
    mapping[0] = {66, 30, 15};
    mapping[1] = {25, 7, 26};
    mapping[2] = {9, 1, 47};
    mapping[3] = {4, 4, 73};
    mapping[4] = {0, 7, 100};
    mapping[5] = {12, 44, 138};
    mapping[6] = {24, 82, 177};
    mapping[7] = {57, 125, 209};
    mapping[8] = {134, 181, 229};
    mapping[9] = {211, 236, 248};
    mapping[10] = {241, 233, 191};
    mapping[11] = {248, 201, 95};
    mapping[12] = {255, 170, 0};
    mapping[13] = {204, 128, 0};
    mapping[14] = {153, 87, 0};
    mapping[15] = {106, 52, 3};

    //non smooth
    *p_rcolor = mapping[i][0];
    *p_gcolor = mapping[i][1];
    *p_bcolor = mapping[i][2];

    //smooth
    // double smooth = ((iter_ix + 1 - log(log2(abs(z))))); //0 -> im

    // const tinycolormap::Color color = tinycolormap::GetColor(smooth/iters_max, tinycolormap::ColormapType::Viridis);

    // *p_rcolor = 255*color.r();
    // *p_gcolor = 255*color.g();
    // *p_bcolor = 255*color.b();


  }
  else  // set interior set color
  {
    if (p_rcolor != 0) *p_rcolor = 0;
    if (p_gcolor != 0) *p_gcolor = 0;
    if (p_bcolor != 0) *p_bcolor = 0;
  }
}

void spiral_septagon_iterations_to_escape(double x, double y, unsigned int iters_max,
					  int *p_rcolor, int *p_gcolor, int *p_bcolor,
					  double power, complex<double> zconst, bool julia,
					  unsigned long long &in,
					  unsigned long long &out) {
  complex<double> point(x, y);
  complex<double> z(x, y);
  unsigned int iter_ix = 0;
  
  while (abs(z) < 40 && iter_ix <= iters_max) {
    z = (pow(z,7) - (0.7/5))/z;
    iter_ix++;
  }

  if (iter_ix < iters_max)
    ++out;
  else
    ++in;

  if (iter_ix < iters_max){
    //    return (255 * iter_ix) / (iters_max - 1);
    //  smooth coloring ????    mu = N + 1 - log (log  |Z(N)|) / log 2

    // *p_rcolor = (255*iter_ix) / (iters_max);
    // *p_gcolor = 255 - (255*iter_ix) / (iters_max);
    // *p_bcolor = clamp(128 + (255*iter_ix) / (iters_max),(unsigned int)0,(unsigned int)255);

    //Ultra Fractal Default
    int i = iter_ix % 16;
    vector<vector<int>> mapping(16, std::vector<int>(3));
    mapping[0] = {66, 30, 15};
    mapping[1] = {25, 7, 26};
    mapping[2] = {9, 1, 47};
    mapping[3] = {4, 4, 73};
    mapping[4] = {0, 7, 100};
    mapping[5] = {12, 44, 138};
    mapping[6] = {24, 82, 177};
    mapping[7] = {57, 125, 209};
    mapping[8] = {134, 181, 229};
    mapping[9] = {211, 236, 248};
    mapping[10] = {241, 233, 191};
    mapping[11] = {248, 201, 95};
    mapping[12] = {255, 170, 0};
    mapping[13] = {204, 128, 0};
    mapping[14] = {153, 87, 0};
    mapping[15] = {106, 52, 3};

    *p_rcolor = mapping[i][0];
    *p_gcolor = mapping[i][1];
    *p_bcolor = mapping[i][2];
    
  }
  else  // set interior set color
  {
    if (p_rcolor != 0) *p_rcolor = 0;
    if (p_gcolor != 0) *p_gcolor = 0;
    if (p_bcolor != 0) *p_bcolor = 0;
  }
}

void generate_buddhabrot_trail(const complex<double> &c, unsigned int iters_max,
                               vector<complex<double>> &trail,
                               int power, complex<double> zconst, bool julia,
                               unsigned long long &in,
                               unsigned long long &out) {
  unsigned int iter_ix = 0;
  complex<double> z(0, 0);

  if (julia)
    z = c;

  trail.clear();
  trail.reserve(iters_max + 1);

  while (iter_ix < iters_max && abs(z) < 2.0) {
    if (julia)
      z = pow(z,power) + zconst; //With Julia you dont add Point
    else
      z = pow(z,power) + c;
    //z = z*z + c;
    ++iter_ix;

    trail.push_back(z);
  }

  // If point is in the set we wont use it to color
  if (iter_ix == iters_max) {
    ++in;
    trail.clear();
  } else {
    ++out;
  }

  // return trail
}

// need buddhabrot threads not to mess up model
std::mutex thread_result_report_mutex;

// Overall Model that gets drawn each cycle
class FractalModel : public sf::Drawable, public sf::Transformable {
 public:
  FractalModel(unsigned int _view_width, unsigned int _view_height)
      : view_width{_view_width}, view_height{_view_height} {
    current_fractal = 0;
    hitsums = 0;
    maxred = 0;
    maxgreen = 0;
    maxblue = 0;
    cuda_detected = false;
    memset(&stats, 0, sizeof(stats));
    R.displayed_zoom = 1.0;
    R.requested_zoom = 1.0;
    R.xstart = FRAC[current_fractal].xMinMax[0];
    R.ystart = FRAC[current_fractal].yMinMax[0];
    R.current_height = R.original_height;
    R.current_width = R.original_width;
    
    original_view_width = view_width;
    original_view_height = view_height;
    image.create(view_width, view_height, sf::Color(0, 0, 0));

    if (FRAC[current_fractal].probabalistic != true)
      panFractal(view_width / 2.0, view_height / 2.0);

    createBuddhabrot();

    color.resize(IMAGE_WIDTH);
    for (auto &v : color) v.resize(IMAGE_HEIGHT);
    

    stats[current_fractal].next_second_start = chrono::steady_clock::now();
  }

  ~FractalModel() {}

  void resetDefaults() {
    R.displayed_zoom = 1.0;
    R.requested_zoom = 1.0;
    R.xstart = FRAC[current_fractal].xMinMax[0];
    R.ystart = FRAC[current_fractal].yMinMax[0];
    R.current_height = R.original_height;
    R.current_width = R.original_width;
    hitsums = 0;
    maxred = 0;
    maxgreen = 0;
    maxblue = 0;
    memset(&stats, 0, sizeof(stats));
    if (FRAC[current_fractal].probabalistic != true) {
      zoomFractal(1.0);
    }
    
    {
      //TODO zero the per thread hits as well
      std::lock_guard<std::mutex> guard(thread_result_report_mutex);  // keep out other threads
      redTrailHits.resize(0);
      greenTrailHits.resize(0);
      blueTrailHits.resize(0);
      createBuddhabrot();
    }
  }

  

  // thread pool is currently started outside the model
  void fractal_thread(int tix, std::future<void> terminate, bool * p_reset) {
    cout << "fractal thread running: " << tix << endl;

    vector<vector<unsigned long long>> redHits;
    vector<vector<unsigned long long>> greenHits;
    vector<vector<unsigned long long>> blueHits;

    bool reset_detected = false;

    redHits.resize(IMAGE_WIDTH);
    for (auto &v : redHits) v.resize(IMAGE_HEIGHT);
    greenHits.resize(IMAGE_WIDTH);
    for (auto &v : greenHits) v.resize(IMAGE_HEIGHT);
    blueHits.resize(IMAGE_WIDTH);
    for (auto &v : blueHits) v.resize(IMAGE_HEIGHT);

    while (1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));

      // see if we should terminate - since we're exiting anyway dont bother to pass
      // it into the threads
      if (terminate.wait_for(std::chrono::nanoseconds(0)) !=
          std::future_status::timeout) {
        std::cout << "Terminate thread requested: " << tix << std::endl;
        break;
      }

      // do work if we didnt terminate

      //Non Probabalistic fractals
      if (FRAC[current_fractal].probabalistic != true) {
        reset_detected = getImagePixels(R.xstart, R.ystart, R.xdelta, R.ydelta, tix, p_reset);
        if (reset_detected == true)
        {
          reset_detected = false;
          //Clear any data generated so far
          for (auto &v : color) v.clear();
        }
        continue;
      }

      
      //Probabalistic fractals

      if ((FRAC[current_fractal].cuda_mode == true) &&
          (cuda_detected == true)) {
        // It we are in cuda mode only allow one thread to do something
        if (tix != 0) continue;
      }

      // get trail hits - has to be much longer than sleep time above to be
      // efficient auto start = chrono::high_resolution_clock::now();

      if ((FRAC[current_fractal].cuda_mode == true) &&
          (cuda_detected == true)) {
        SampleStats cudastats{0, 0, 0, 0};
        // device kernel doesnt have context of model object or this c file
        cuda_generate_buddhabrot_hits(IMAGE_WIDTH, IMAGE_HEIGHT,
                                      FRAC[current_fractal], cudastats, redHits,
                                      greenHits, blueHits);
        stats[current_fractal].total += cudastats.total;
        stats[current_fractal].rejected += cudastats.rejected;
        stats[current_fractal].in_set += cudastats.in_set;
        stats[current_fractal].escaped_set += cudastats.escaped_set;

      } else {
        // threaded version of generate hits (we take advantage of being inside
        // model object)
        reset_detected = generateMoreTrailHits(redHits, greenHits, blueHits, &p_reset[tix]);
      }

      if (reset_detected == true)
      {
        reset_detected = false;
        //Clear any data generated so far
        for (auto &v : redHits) v.clear();
        for (auto &v : greenHits) v.clear();
        for (auto &v : blueHits) v.clear();
        continue; //dont merge fractal per thread trails
      }

      // auto end = chrono::high_resolution_clock::now();
      // cout << "sample time " << tix << " " <<
      // chrono::duration_cast<chrono::milliseconds>(end - start).count() << "
      // ms" << endl;

      // merge trail hits into the instance of the class (but dont make image)
      {
        mergeHits(redHits, greenHits,
                  blueHits);  // mutex inside
      }
    }

    cout << "fractal thread exiting: " << tix << endl;
  };  // buddhabrot_thread

  // for each color
  // generate a random sampling of points inside the -2 2 -2, 2 region
  // for each sample
  // get the vector trail using the per color max_iters
  // increment the point in the image size 2Darray every time it shows up in a
  // vector trail color each pixel according to the amount of times the point
  // has shown up in all the trails
  void createBuddhabrot() {
    // Initializing the 2-D vector
    redTrailHits.resize(IMAGE_WIDTH);
    for (auto &v : redTrailHits) v.resize(IMAGE_HEIGHT);
    greenTrailHits.resize(IMAGE_WIDTH);
    for (auto &v : greenTrailHits) v.resize(IMAGE_HEIGHT);
    blueTrailHits.resize(IMAGE_WIDTH);
    for (auto &v : blueTrailHits) v.resize(IMAGE_HEIGHT);

  }  // createBuddhabrot

  void cudaTest() {
    int count = cuda_info();

    if (count == 0) {
      cuda_detected = false;
      return;
    }
    cuda_detected = true;

    // test some cuda vector addition and return of sum vector to host
    cuda_vec_add(IMAGE_WIDTH, IMAGE_HEIGHT);

    // test some cuda hit generation and return of hits to the host
    cuda_generate_hits_no_fractal(IMAGE_WIDTH, IMAGE_HEIGHT);

    // test some cuda hit generation and return of hits to the host
    SampleStats fakestat;
    cuda_generate_buddhabrot_hits(IMAGE_WIDTH, IMAGE_HEIGHT,
                                  FRAC[current_fractal], fakestat,
                                  redTrailHits, greenTrailHits, blueTrailHits);
  }

  void saveBuddhabrotTrailToColor(
      vector<complex<double>> &trail,
      vector<vector<long long unsigned int>> &colorTrailHits) {
    for (complex<double> &c : trail) {
      // if point is plottable, scale it to be on a pixel and increment the
      // value for the pixel

      double minx = FRAC[current_fractal].xMinMax[0];
      double maxx = FRAC[current_fractal].xMinMax[1];
      double miny = FRAC[current_fractal].yMinMax[0];
      double maxy = FRAC[current_fractal].yMinMax[1];
      if ((c.real() <= maxx) && (c.real() >= minx) && (c.imag() <= maxy) &&
          (c.imag() >= miny)) {
        // depending on the cast here you might get a faint gridline in your
        // image so be careful
        int x = ((c.real() - minx) * R.original_width) / (maxx - minx);
        int y = ((c.imag() - miny) * R.original_height) / (maxy - miny);

        colorTrailHits[x][y]++;
      }
    }
  }

  bool skipInSet(complex<double> sample) {
    if ((abs(sample - complex<double>(-1, 0)) < 0.25) ||
        (abs(1.0 - sqrt(1.0 - 4.0 * sample))) < 1.0)
      return true;
    return false;
  }

  bool generateMoreTrailHits(vector<vector<unsigned long long>> &redHits,
                             vector<vector<unsigned long long>> &greenHits,
                             vector<vector<unsigned long long>> &blueHits,
                             bool * p_reset) {
    bool reset_detected = false;
    std::random_device rd;
    static std::mt19937_64 re(rd());
    uniform_real_distribution<double> xDistribution(
        FRAC[current_fractal].xMinMax[0], FRAC[current_fractal].xMinMax[1]);
    uniform_real_distribution<double> yDistribution(
        FRAC[current_fractal].yMinMax[0], FRAC[current_fractal].yMinMax[1]);
    re.seed(chrono::high_resolution_clock::now().time_since_epoch().count());

    unsigned long long max_samples =
        100000;  // large enought to overcome thread sleep time

    for (unsigned long long s_ix = 0; s_ix < max_samples; ++s_ix) {
      // Random pixels
      complex<double> sample(xDistribution(re), yDistribution(re));
      stats[current_fractal].total++;  // not atomic....

      // see if we should reset
      if (*p_reset == true) {
        *p_reset = false;
        //std::cout << "Reset hits thread requested: " << std::endl;
        reset_detected = true;
        break;
      }

      if (true == skipInSet(sample)) {
        stats[current_fractal].rejected++;  // not atomic....
        continue;
      }

      // Alternative: Try to hit all the pixels
      // double deltax = 4.0/R.original_width;
      // double deltay = 4.0/R.original_height;
      // complex<double> sample(FRAC[current_fractal].xMinMax[0] + deltax/2.0 +
      // deltax*(s_ix%static_cast<int>(R.original_width)),
      //                        FRAC[current_fractal].yMinMax[0] + deltay/2.0 +
      //                        deltay*((s_ix/static_cast<int>(R.original_width))%(static_cast<int>(R.original_height))));

      vector<complex<double>> trail;

      // unsigned int red_max_iters = 100000;
      // unsigned int green_max_iters = 10000;
      // unsigned int blue_max_iters = 1000;
      unsigned int red_max_iters = FRAC[current_fractal].max_iters[0];
      unsigned int green_max_iters = FRAC[current_fractal].max_iters[1];
      unsigned int blue_max_iters = FRAC[current_fractal].max_iters[2];

      generate_buddhabrot_trail(sample, red_max_iters, trail,
                                FRAC[current_fractal].power,
                                FRAC[current_fractal].zconst,
                                FRAC[current_fractal].julia,
                                stats[current_fractal].in_set,
                                stats[current_fractal].escaped_set);
      saveBuddhabrotTrailToColor(trail, redHits);
      if (0 != trail.size()) {
        sample = complex<double>(sample.real(), -sample.imag());
        generate_buddhabrot_trail(sample, red_max_iters, trail,
                                  FRAC[current_fractal].power,
                                  FRAC[current_fractal].zconst,
                                  FRAC[current_fractal].julia,
                                  stats[current_fractal].in_set,
                                  stats[current_fractal].escaped_set);
        saveBuddhabrotTrailToColor(trail, redHits);
      }

      generate_buddhabrot_trail(sample, green_max_iters, trail,
                                FRAC[current_fractal].power,
                                FRAC[current_fractal].zconst,
                                FRAC[current_fractal].julia,
                                stats[current_fractal].in_set,
                                stats[current_fractal].escaped_set);
      saveBuddhabrotTrailToColor(trail, greenHits);
      if (0 != trail.size()) {
        sample = complex<double>(sample.real(), -sample.imag());
        generate_buddhabrot_trail(sample, green_max_iters, trail,
                                  FRAC[current_fractal].power,
                                  FRAC[current_fractal].zconst,
                                  FRAC[current_fractal].julia,
                                  stats[current_fractal].in_set,
                                  stats[current_fractal].escaped_set);
        saveBuddhabrotTrailToColor(trail, greenHits);
      }

      generate_buddhabrot_trail(sample, blue_max_iters, trail,
                                FRAC[current_fractal].power,
                                FRAC[current_fractal].zconst,
                                FRAC[current_fractal].julia,
                                stats[current_fractal].in_set,
                                stats[current_fractal].escaped_set);
      saveBuddhabrotTrailToColor(trail, blueHits);
      if (0 != trail.size()) {
        sample = complex<double>(sample.real(), -sample.imag());
        generate_buddhabrot_trail(sample, blue_max_iters, trail,
                                  FRAC[current_fractal].power,
                                  FRAC[current_fractal].zconst,
                                  FRAC[current_fractal].julia,
                                  stats[current_fractal].in_set,
                                  stats[current_fractal].escaped_set);
        saveBuddhabrotTrailToColor(trail, blueHits);
      }
    }
    return reset_detected;
  }

  // each thread does this under mutex
  void mergeHits(vector<vector<unsigned long long>> &redHits,
                 vector<vector<unsigned long long>> &greenHits,
                 vector<vector<unsigned long long>> &blueHits) {
    // auto start = chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> guard(
        thread_result_report_mutex);  // keep out other threads
    // auto end = chrono::high_resolution_clock::now();
    // cout << "mutex lock time " <<
    // chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms"
    // << endl; //65 ms
    unsigned long long total_merged = 0;

    for (unsigned int i = 0; i < R.original_width; i++) {
      for (unsigned int j = 0; j < R.original_height; j++) {
        total_merged += redHits[i][j];
        redTrailHits[i][j] += redHits[i][j];
        redHits[i][j] = 0;

        total_merged += greenHits[i][j];
        greenTrailHits[i][j] += greenHits[i][j];
        greenHits[i][j] = 0;

        total_merged += blueHits[i][j];
        blueTrailHits[i][j] += blueHits[i][j];
        blueHits[i][j] = 0;
      }
    }
    // auto mend = chrono::high_resolution_clock::now();
    // cout << "merge time " << chrono::duration_cast<chrono::milliseconds>(mend
    // - end).count()<< " ms for: " << total_merged << endl; //65 ms
  }

  // we wont take the mutex here since it doesnt matter
  void rebuildImageFromHits() {
    hitsums = 0;
    // for every pixel
    for (unsigned int i = 0; i < R.original_width; i++) {
      for (unsigned int j = 0; j < R.original_height; j++) {
        sf::Color pcolor{0, 0, 0};

        if (redTrailHits[i][j] > maxred) maxred = redTrailHits[i][j];

        if (greenTrailHits[i][j] > maxgreen) maxgreen = greenTrailHits[i][j];

        if (blueTrailHits[i][j] > maxblue) maxblue = blueTrailHits[i][j];

        // sqrt normalized coloring - more detail
        double rratio =
            static_cast<double>(255) / sqrt(static_cast<double>(maxred));
        int rcolor = sqrt(redTrailHits[i][j]) * rratio;
        pcolor = pcolor + sf::Color(rcolor, 0, 0);
        hitsums += redTrailHits[i][j];

        double gratio =
            static_cast<double>(255) / sqrt(static_cast<double>(maxgreen));
        int gcolor = sqrt(greenTrailHits[i][j]) * gratio;
        pcolor = pcolor + sf::Color(0, gcolor, 0);
        hitsums += greenTrailHits[i][j];

        double bratio =
            static_cast<double>(255) / sqrt(static_cast<double>(maxblue));
        int bcolor = sqrt(blueTrailHits[i][j]) * bratio;
        pcolor = pcolor + sf::Color(0, 0, bcolor);
        hitsums += blueTrailHits[i][j];

        // ratio coloring
        // double rratio = static_cast<double>(255) / maxred;
        // int rcolor = redTrailHits[i][j]*rratio;
        // pcolor = pcolor + sf::Color(rcolor,0,0);
        // hitsums += redTrailHits[i][j];

        // double gratio = static_cast<double>(255) / maxgreen;
        // int gcolor = greenTrailHits[i][j]*gratio;
        // pcolor = pcolor + sf::Color(0,gcolor,0);
        // hitsums += greenTrailHits[i][j];

        // double bratio = static_cast<double>(255) / maxblue;
        // int bcolor = blueTrailHits[i][j]*bratio;
        // pcolor = pcolor + sf::Color(0,0,bcolor);
        // hitsums += blueTrailHits[i][j];

        image.setPixel(i, j, pcolor);
      }
    }

    texture.loadFromImage(image);
    sprite.setTexture(texture);

    // sprite.setOrigin(800,600);
    // sprite.rotate(90.f);
  }

  bool getImagePixels(double xstart, double ystart, double xdelta,
                      double ydelta, unsigned int tix, bool * p_reset) {
    bool reset_detected = false;

    //Subdivide x range by tix and num_threads
    unsigned int xrange = R.original_width / num_threads;
    unsigned int xs = tix*xrange;
    unsigned int xe = (tix+1)*xrange;
    if (tix == num_threads - 1)
      xe = R.original_width;

    
    for (unsigned int i = xs; i < xe; i++) {
      for (unsigned int j = 0; j < R.original_height; j++) {
        // see if we should reset
        if (p_reset[tix] == true) {
          p_reset[tix] = false;
          //std::cout << "Reset thread requested: " << tix << std::endl;
          reset_detected = true;
          break;
        }
        
        double xi = xstart + i * xdelta;
        double yj = ystart + j * ydelta;
        stats[current_fractal].total++;

        int rcolor = 0;
        int gcolor = 0;
        int bcolor = 0;

	if (FRAC[current_fractal].name  == string("Spiral Septagon"))
	  spiral_septagon_iterations_to_escape(xi, yj, 300, &rcolor, &gcolor, &bcolor,
					       FRAC[current_fractal].power,
					       FRAC[current_fractal].zconst,
					       FRAC[current_fractal].julia,
					       stats[current_fractal].in_set,
					       stats[current_fractal].escaped_set);
	else
	  mandelbrot_iterations_to_escape(xi, yj, 300, &rcolor, &gcolor, &bcolor,
					  FRAC[current_fractal].power,
					  FRAC[current_fractal].zconst,
					  FRAC[current_fractal].julia,
					  stats[current_fractal].in_set,
					  stats[current_fractal].escaped_set);
    
        color[i][j] = sf::Color(rcolor, gcolor, bcolor);
      }

      if (reset_detected == true)
        break;
    }
    hitsums = R.original_width * R.original_height;

    return reset_detected;
  }

  void setImagePixels(double xstart, double ystart, double xdelta,
                      double ydelta) {
    // for every pixel
    for (unsigned int i = 0; i < R.original_width; i++) {
      for (unsigned int j = 0; j < R.original_height; j++) {
        image.setPixel(i, j, color[i][j]);
      }
    }

    texture.loadFromImage(image);
    sprite.setTexture(texture);
  }

  void calculateZoomWindow(double newzoom) {
    double xratio = newzoom;
    double yratio = newzoom;

    double minx = FRAC[current_fractal].xMinMax[0];
    double maxx = FRAC[current_fractal].xMinMax[1];
    double miny = FRAC[current_fractal].yMinMax[0];
    double maxy = FRAC[current_fractal].yMinMax[1];

    // mandelbrot_coordinates
    double xdelta = (maxx - minx) * xratio / R.original_width;
    double ydelta = (maxy - miny) * yratio / R.original_height;
    R.xdelta = xdelta;
    R.ydelta = ydelta;

    // after we zoom, we want to start here
    // mandelbrot coordinates
    double xstart =
        R.xstart + (maxx - minx) * (R.displayed_zoom - newzoom) / 2.0;  // pixels

    R.xstart = xstart;


    double ystart =
        R.ystart + (maxy - miny) * (R.displayed_zoom - newzoom) / 2.0;  // pixels

    R.ystart = ystart;



    R.current_width = newzoom * R.original_width;
    R.current_height = newzoom * R.original_height;

    R.displayed_zoom = newzoom;

    cout << "zoom: " << R.displayed_zoom;
    cout << "  cdims: " << R.current_width << " " << R.current_height << endl;
    cout.precision(17);
    cout << scientific << " starts: " << R.xstart << " " << R.ystart << endl;
    cout << "x range: " << scientific << xstart << " -> "
         << xstart + (R.original_width - 1) * xdelta;
    cout << "  y range: " << ystart << " -> "
         << ystart + (R.original_height - 1) * ydelta << fixed << endl;
  }

  // Assumes the user doesnt resize the window to give it different pixels
  void calculatePanWindow(double xcenter, double ycenter) {
    double xratio = R.displayed_zoom;
    double yratio = R.displayed_zoom;

    double minx = FRAC[current_fractal].xMinMax[0];
    double maxx = FRAC[current_fractal].xMinMax[1];
    double miny = FRAC[current_fractal].yMinMax[0];
    double maxy = FRAC[current_fractal].yMinMax[1];

    // mandelbrot_coordinates
    double xdelta = (maxx - minx) * xratio / R.original_width;
    double ydelta = (maxy - miny) * yratio / R.original_height;
    R.xdelta = xdelta;
    R.ydelta = ydelta;

    // after we pan we want to start here
    // mandelbrot coordinates
    double xstart = R.xstart - ((maxx - minx) / R.original_width)*(R.original_width / 2.0 - xcenter) * R.displayed_zoom;  // pixels

    R.xstart = xstart;


    double ystart = R.ystart - ((maxy - miny) / R.original_height)*(R.original_height / 2.0 - ycenter) * R.displayed_zoom;  // pixels

    R.ystart = ystart;


    cout << "pan: " << xcenter << " " << ycenter <<endl;
    cout << "  cdims: " << R.current_width << " " << R.current_height;
    cout.precision(17);
    cout << scientific << " starts: " << R.xstart << " " << R.ystart << endl;
    cout << "x range: " << scientific << xstart << " -> "
         << xstart + (R.original_width - 1) * xdelta;
    cout << "  y range: " << ystart << " -> "
         << ystart + (R.original_height - 1) * ydelta << fixed << endl;
  }

  void zoomFractal(double newzoom) {
    if (FRAC[current_fractal].probabalistic == true) return;
    calculateZoomWindow(newzoom);
  }

  void panFractal(double xcenter, double ycenter) {
    if (FRAC[current_fractal].probabalistic == true) return;
    calculatePanWindow(xcenter, ycenter);
  }

  void update(sf::Time elapsed) {
    //draw image from latest data
    if (FRAC[current_fractal].probabalistic == true) {
      {
        std::lock_guard<std::mutex> guard(thread_result_report_mutex);
        rebuildImageFromHits();  // SetImagePixels
      }
    }
    else
    {
      setImagePixels(R.xstart, R.ystart,
                     R.xdelta, R.ydelta);
    }
    

    // Update stats in Model to track how effective fractal threads,cuda are
    auto now = chrono::steady_clock::now();
    unsigned long long samples_now = stats[current_fractal].total;
    if ((now > stats[current_fractal].next_second_start) &&
        (0 != (samples_now - stats[current_fractal].samples_last_second))) {
      stats[current_fractal].samples_per_second =
          (1000 * (samples_now - stats[current_fractal].samples_last_second)) /
          (1000 + chrono::duration_cast<chrono::milliseconds>(
              now - stats[current_fractal].next_second_start)
           .count());
      stats[current_fractal].next_second_start =
          chrono::steady_clock::now() + std::chrono::milliseconds(1000);
      
      // cout << stats[current_fractal].samples_per_second << " " <<
      // (samples_now - stats[current_fractal].samples_last_second) << endl;
      
      stats[current_fractal].samples_last_second = samples_now;
    }
  }

 private:
  virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
    // apply the transform
    states.transform *= getTransform();

    // our particles don't use a texture
    states.texture = NULL;

    // draw the image (just one for now - could do multiple fractals blended)
    target.draw(sprite, states);
  }

 public:
  unsigned int current_fractal;
  bool cuda_detected;
  unsigned int view_width;
  unsigned int view_height;
  unsigned long long maxred = 0;
  unsigned long long maxgreen = 0;
  unsigned long long maxblue = 0;
  unsigned long long hitsums = 0;

  SampleStats stats[10];  // indexed by fractal

  unsigned int num_threads;

 private:

  double original_view_width;
  double original_view_height;
  sf::Image image;
  sf::Texture texture;
  sf::Sprite sprite;

  // merged hits from threads
  vector<vector<unsigned long long>> redTrailHits;
  vector<vector<unsigned long long>> greenTrailHits;
  vector<vector<unsigned long long>> blueTrailHits;

  //Non buddha fractals
  vector<vector<sf::Color>> color;
};

// Now we try to do the control elements displayed inside the view GUI that
// control both the model and the view (not just the view)

void setGuiElementsFromModel(shared_ptr<tgui::Gui> &pgui,
                             shared_ptr<FractalModel> &p_model);

void updateGuiElements(shared_ptr<tgui::Gui> &pgui,
                       shared_ptr<FractalModel> &p_model);

void signalFractalMenu(shared_ptr<FractalModel> p_model,
                       shared_ptr<tgui::Gui> pgui, const sf::String &selected) {
  for (size_t i = 0; i < FRAC.size(); ++i) {
    if (selected == FRAC[i].name) {
      p_model->current_fractal = i;
    }
  }
  updateGuiElements(pgui, p_model);
  p_model->resetDefaults();
  setGuiElementsFromModel(pgui, p_model);
}

void signalPower(shared_ptr<FractalModel> p_model,
                 shared_ptr<tgui::Gui> pgui, const sf::String &value) {
  updateGuiElements(pgui, p_model);

  double input = 2.0;
  try {
    input = std::stod(value.toAnsiString());
  }
  catch (const std::invalid_argument &ia) {
    cout << "Invalid " << ia.what() <<endl;
    input = 2.0;
  }
  FRAC[p_model->current_fractal].power = input;
  p_model->resetDefaults();
  setGuiElementsFromModel(pgui, p_model);
}

void signalZconstr(shared_ptr<FractalModel> p_model,
                   shared_ptr<tgui::Gui> pgui, const sf::String &value) {
  updateGuiElements(pgui, p_model);

  double input = 0.0;
  try {
    input = std::stod(value.toAnsiString());
  }
  catch (const std::invalid_argument &ia) {
    cout << "Invalid " << ia.what() <<endl;
    input = 0.0;
  }
  FRAC[p_model->current_fractal].zconst = complex<double>(input, FRAC[p_model->current_fractal].zconst.imag());
  p_model->resetDefaults();
  setGuiElementsFromModel(pgui, p_model);
}

void signalZconsti(shared_ptr<FractalModel> p_model,
                   shared_ptr<tgui::Gui> pgui, const sf::String &value) {
  updateGuiElements(pgui, p_model);

  double input = 0.0;
  try {
    input = std::stod(value.toAnsiString());
  }
  catch (const std::invalid_argument &ia) {
    cout << "Invalid " << ia.what() <<endl;
    input = 0.0;
  }
  FRAC[p_model->current_fractal].zconst = complex<double>(FRAC[p_model->current_fractal].zconst.real(), input);
  p_model->resetDefaults();
  setGuiElementsFromModel(pgui, p_model);
}

// The main GUI elements inside the view
void createGuiElements(shared_ptr<tgui::Gui> pgui,
                       shared_ptr<FractalModel> &p_model) {
  // Current and Menu Column
  auto current = tgui::Label::create();
  current->setPosition("parent.left", "parent.bottom - 300");
  current->setTextSize(14);
  pgui->add(current, "fractal_label");

  current = tgui::Label::create();
  current->setPosition("parent.left + 250", "parent.bottom - 300");
  current->setTextSize(14);
  pgui->add(current, "progress_label");

  current = tgui::Label::create();
  current->setPosition("parent.left", "parent.bottom - 300 + 20");
  current->setTextSize(14);
  pgui->add(current, "secs_label");

  current = tgui::Label::create();
  current->setPosition("parent.left + 150", "parent.bottom - 300 + 20");
  current->setTextSize(14);
  pgui->add(current, "fps_label");

  current = tgui::Label::create();
  current->setPosition("parent.left + 280", "parent.bottom - 300 + 20");
  current->setTextSize(14);
  pgui->add(current, "sps_label");

  current = tgui::Label::create();
  current->setPosition("parent.left + 400", "parent.bottom - 300 + 20");
  current->setTextSize(14);
  pgui->add(current, "threads_label");

  current = tgui::Label::create();
  current->setPosition("parent.left + 500", "parent.bottom - 300 + 20");
  current->setTextSize(14);
  pgui->add(current, "cuda_label");

  current = tgui::Label::create();
  current->setPosition("parent.left", "parent.bottom - 300 + 40");
  current->setTextSize(14);
  pgui->add(current, "boundary_label");

  current = tgui::Label::create();
  current->setPosition("parent.left", "parent.bottom - 300 + 60");
  current->setTextSize(14);
  pgui->add(current, "stats_label");

  auto menu = tgui::MenuBar::create();
  menu->setPosition("parent.left", "parent.bottom - 300 + 80");
  menu->setSize(200.f, 22.f);
  menu->addMenu("Fractal");
  for (auto frac : FRAC) {
    menu->addMenuItem(frac.name);
  }
  menu->addMenu("Help");
  menu->addMenuItem("Use mouse wheel to zoom (some fractals)");
  menu->addMenuItem("Use right mouse button to recenter (some fractals)");
  menu->addMenuItem("Type h to hide/show gui");
  menu->addMenuItem("Type s to take a screenshot");
  menu->addMenuItem("Type c to turn cuda on and off");
  menu->addMenuItem("Type f for fullscreen toggle (buggy)");
  menu->addMenuItem("Type e to exit");

  pgui->add(menu);
  menu->connect("MenuItemClicked", signalFractalMenu, p_model, pgui);

  //Params column
  current = tgui::Label::create();
  current->setPosition("parent.left + 300", "parent.bottom - 150 - 20");
  current->setTextSize(14);
  pgui->add(current, "power_label");
  
  auto editBox = tgui::EditBox::create();
  editBox->setSize(100, 20);
  editBox->setTextSize(14);
  editBox->setPosition("parent.left + 300", "parent.bottom - 150");
  editBox->setDefaultText("2");
  pgui->add(editBox, "power_box");
  editBox->connect("TextChanged", signalPower, p_model, pgui);


  current = tgui::Label::create();
  current->setPosition("parent.left + 300", "parent.bottom - 100 - 20");
  current->setTextSize(14);
  pgui->add(current, "zconst_label");
  
  editBox = tgui::EditBox::create();
  editBox->setSize(100, 20);
  editBox->setTextSize(14);
  editBox->setPosition("parent.left + 300", "parent.bottom - 100");
  editBox->setDefaultText("0");
  pgui->add(editBox, "zconst_real_box");
  editBox->connect("TextChanged", signalZconstr, p_model, pgui);

  editBox = tgui::EditBox::create();
  editBox->setSize(100, 20);
  editBox->setTextSize(14);
  editBox->setPosition("parent.left + 300 + 120", "parent.bottom - 100");
  editBox->setDefaultText("0");
  pgui->add(editBox, "zconst_imag_box");
  editBox->connect("TextChanged", signalZconsti, p_model, pgui);



  
}

// When model changes resulting in gui changes
void updateGuiElements(shared_ptr<tgui::Gui> &pgui,
                       shared_ptr<FractalModel> &p_model) {
  // hide all widgets
  // show and update the ones that are needed
  // labels
  // default values for gui elements
}

// When some per model control changes: f.e. theta
void setGuiElementsFromModel(shared_ptr<tgui::Gui> &pgui,
                             shared_ptr<FractalModel> &p_model) {}

// update current things every draw even if no change
void updateCurrentGuiElements(shared_ptr<tgui::Gui> &pgui,
                              shared_ptr<FractalModel> &p_model, float secs,
                              float fps) {
  auto current = pgui->get<tgui::Label>("fractal_label");
  current->setText("Fractal: " + FRAC[p_model->current_fractal].name);

  current = pgui->get<tgui::Label>("progress_label");
  current->setText("Progress: " + to_string(p_model->maxred) + "/" +
                   to_string(p_model->maxgreen) + "/" +
                   to_string(p_model->maxblue) + "  " +
                   to_string(p_model->hitsums));

  current = pgui->get<tgui::Label>("secs_label");
  current->setText("Secs: " + to_string(secs));

  current = pgui->get<tgui::Label>("fps_label");
  current->setText("fps: " + to_string(fps));

  current = pgui->get<tgui::Label>("sps_label");
  current->setText(
      "sps: " +
      to_string(p_model->stats[p_model->current_fractal].samples_per_second));

  current = pgui->get<tgui::Label>("threads_label");
  current->setText("Threads: " + to_string(p_model->num_threads));

  current = pgui->get<tgui::Label>("cuda_label");
  if ((FRAC[p_model->current_fractal].cuda_mode == true) && (p_model->cuda_detected == true))
    current->setText("Cuda Running");
  else
    current->setText("Cuda Off");

  current = pgui->get<tgui::Label>("boundary_label");
  current->setText(
      "Boundary: [" +
      to_string(R.xstart) + "->" +
      to_string(R.xstart + (R.original_width) * R.xdelta) +
      "]/[" + to_string(R.ystart) +
      "->" +
      to_string(R.ystart + (R.original_height) * R.ydelta) +
      "]");

  current = pgui->get<tgui::Label>("stats_label");
  current->setText(
      "Stats: rejected/total escaped/in " +
      to_string(p_model->stats[p_model->current_fractal].rejected) + "/" +
      to_string(p_model->stats[p_model->current_fractal].total) + " " +
      to_string(p_model->stats[p_model->current_fractal].escaped_set) + "/" +
      to_string(p_model->stats[p_model->current_fractal].in_set) + " " +
      to_string(100.0 * p_model->stats[p_model->current_fractal].rejected /
                (p_model->stats[p_model->current_fractal].total)) +
      "\%" + " " +
      to_string(p_model->stats[p_model->current_fractal].escaped_set /
                (static_cast<double>(
                    p_model->stats[p_model->current_fractal].in_set +
                    p_model->stats[p_model->current_fractal].escaped_set))) +
      "\% total");

  //Params column
  current = pgui->get<tgui::Label>("power_label");
  current->setText("Power: " + to_string(FRAC[p_model->current_fractal].power));

  current = pgui->get<tgui::Label>("zconst_label");
  current->setText("zconst: " + to_string(FRAC[p_model->current_fractal].zconst.real()) + " " +
                   to_string(FRAC[p_model->current_fractal].zconst.imag()) + "*i");
  
}

void display_all_widgets(shared_ptr<tgui::Gui> &pgui, bool maybe) {
  std::vector<tgui::Widget::Ptr> widgets;
  widgets = pgui->getWidgets();

  for (auto &w : widgets) {
    w->setVisible(maybe);
  }
}

// respond to mouse wheel zoom
double get_new_zoom(sf::View &view, int delta) {
  if (delta < 0) {
    // zoom in
    R.requested_zoom = R.requested_zoom * 0.90;
    // cout << "zoom: " << current_zoom << endl;
    if (R.requested_zoom < 0.000000001) {
      R.requested_zoom = 1.0;
    }
  } else {
    // zoom out
    R.requested_zoom = R.requested_zoom * 1.1;
    // cout << "zoom: " << current_zoom << endl;

    if (R.requested_zoom > 5.0) {
      R.requested_zoom = 1.0;
    }
  }

  return R.requested_zoom;
}

// save a screenshot
void save_screenshot(sf::RenderWindow &window, string name) {
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[80];

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, sizeof(buffer), "%d-%m-%Y_%H_%M_%S", timeinfo);
  std::string timestring(buffer);

  sf::Vector2u windowSize = window.getSize();
  sf::Texture texture;
  texture.create(windowSize.x, windowSize.y);
  texture.update(window);
  sf::Image screenshot = texture.copyToImage();
  screenshot.saveToFile(name + timestring + ".png");
};

int main(int argc, char **argv) {
  // Register signal and signal handler
  signal(SIGINT, signal_callback_handler);

  sf::Vector2u screenDimensions(IMAGE_WIDTH, IMAGE_HEIGHT);
  sf::RenderWindow window(sf::VideoMode(screenDimensions.x, screenDimensions.y),
                          "Fractals!", sf::Style::Fullscreen);
  window.setKeyRepeatEnabled(false);

  // // Display the list of all the video modes available for fullscreen
  // std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();
  // for (std::size_t i = 0; i < modes.size(); ++i)
  // {
  //   sf::VideoMode mode = modes[i];
  //   std::cout << "Mode #" << i << ": "
  //             << mode.width << "x" << mode.height << " - "
  //             << mode.bitsPerPixel << " bpp" << std::endl;
  // }
  // // Create a window with the same pixel depth as the desktop
  // sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
  // window.create(sf::VideoMode(1024, 768, desktop.bitsPerPixel), "SFML
  // window");

  // Main window
  sf::View modelview;
  sf::Vector2u viewD(screenDimensions.x, screenDimensions.y);
  modelview.setSize(viewD.x, viewD.y);
  R.displayed_zoom = 1.0;
  R.requested_zoom = 1.0;
  R.xstart = 0.0;
  R.ystart = 0.0;
  R.current_height = screenDimensions.y;
  R.current_width = screenDimensions.x;
  R.original_height = screenDimensions.y;
  R.original_width = screenDimensions.x;
  modelview.setCenter(screenDimensions.x / 2.0, screenDimensions.y / 2.0);
  window.setView(modelview);

  // create the fractal model (i.e. Model)
  //    note: we are passing this shared ptr to signals and threads so they can
  //    change the model (MVC)
  auto p_model =
      make_shared<FractalModel>(screenDimensions.x, screenDimensions.y);

  p_model->cudaTest();

  // Create the worker threads:
  cout << "Machine supports " << thread::hardware_concurrency()
       << " simultaneous threads" << endl;

  unsigned int cmd_line_threads;
  if (argc > 1)
    cmd_line_threads = atoi(argv[1]);
  else
    cmd_line_threads = thread::hardware_concurrency() - 1;
  unsigned int num_threads = 1;
  if ((cmd_line_threads <= 0) or
      (cmd_line_threads >= thread::hardware_concurrency() - 1))
    num_threads = thread::hardware_concurrency() - 1;
  else
    num_threads = cmd_line_threads;
  p_model->num_threads = num_threads;

  cout << "Using " << num_threads << " threads to speed up fractal rendering" << endl;
  thread threads[num_threads];
  std::promise<void> terminateThreadSignal[num_threads];

  //funilly enough we dont need the complex C++ thread sync primitives
  //this is to prevent unnecessary calculation when we request 2 zooms in a row quickly
  bool thread_asked_to_reset[num_threads]; 
  

  // start up thread pool
  // thread:
  // input: thread id, p_model, mutex to report results, future object for
  // thread termination output: merges hits into model under a mutex
  for (unsigned int tix = 0; tix < num_threads; ++tix) {
    std::future<void> futureObj = terminateThreadSignal[tix].get_future();

    threads[tix] = thread(&FractalModel::fractal_thread, p_model, tix,
                          std::move(futureObj), &thread_asked_to_reset[0]);
  }

  // Create the gui and attach it to the window
  bool display_gui = true;
  auto pgui = make_shared<tgui::Gui>(window);
  tgui::Theme theme{"themes/BabyBlue.txt"};
  tgui::Theme::setDefault(&theme);
  createGuiElements(pgui, p_model);
  updateGuiElements(pgui, p_model);

  // create a clock to track the elapsed time
  sf::Clock clock_s;  // start
  sf::Clock clock_e;  // elapsed between each draw cycle

  sf::Time start = clock_s.restart();
  int frames = 0;

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) window.close();  // breaks out above

      // Handle keyboard control commands

      if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::H) {
          if (display_gui == false) {
            display_gui = true;
            display_all_widgets(pgui, true);
          } else {
            display_gui = false;
            display_all_widgets(pgui, false);
          }
        }
      }

      if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::S) {
          save_screenshot(window, FRAC[p_model->current_fractal].name);
        }
      }

      if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::E) {
          window.close();
	  break;
        }
      }

      if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::C) {
          if (FRAC[p_model->current_fractal].cuda_mode == true)
            FRAC[p_model->current_fractal].cuda_mode = false;
          else
            FRAC[p_model->current_fractal].cuda_mode = true;
        }
      }

      if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::F) {
          // sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
          // window.setSize(sf::Vector2u{screenDimensions.x,
          // screenDimensions.y});
          window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y),
                        "Fractals!", sf::Style::Fullscreen);
          window.setSize(sf::Vector2u{screenDimensions.x, screenDimensions.y});
        }
      }

      // Handle Mouse
      // Zoom the whole sim if mouse wheel moved
      if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
          double newzoom =
              get_new_zoom(modelview, event.mouseWheelScroll.delta);
          cout << "New zoom: " << newzoom << endl;
          p_model->zoomFractal(newzoom);
          for (unsigned int tix = 0; tix < num_threads; ++tix) {thread_asked_to_reset[tix] = true;}
        }
      }

      // record center
      if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Right) {
          cout << "New center: ";
          cout << "x: " << event.mouseButton.x;
          cout << " y: " << event.mouseButton.y << endl;
          p_model->panFractal(event.mouseButton.x, event.mouseButton.y);
          for (unsigned int tix = 0; tix < num_threads; ++tix) {thread_asked_to_reset[tix] = true;}
        }
      }

      // Handle the control coming from the gui
      pgui->handleEvent(event);
    }
    ++frames;

    // Evolve the model independantly
    sf::Time elapsed = clock_e.restart();
    p_model->update(elapsed);  // rebuild the pixels from what threads did in background

    // Draw the GUI and the MODEL both of which are controlled by
    // Keyboard, mouse, and gui elements
    updateCurrentGuiElements(pgui, p_model, start.asSeconds(),
                             frames / start.asSeconds());

    start = clock_s.getElapsedTime();

    window.clear();
    window.setView(modelview);
    window.draw(*p_model);  // draw fractals
    pgui->draw();           // Draw all GUI widgets

    window.display();
  }

  // terminate threads in thread pool
  for (unsigned int tix = 0; tix < num_threads; ++tix) {
    terminateThreadSignal[tix].set_value();
    //to make it check for terminate
    for (unsigned int tix = 0; tix < num_threads; ++tix) {thread_asked_to_reset[tix] = true;} 
    threads[tix].join();
  }

  return 0;
}
