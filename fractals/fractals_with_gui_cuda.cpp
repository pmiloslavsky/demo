#include <math.h>
#include <signal.h>

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <chrono>
#include <cmath>
#include <complex>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>

#include "buddha_cuda_kernel.h"
#include "fractals.h"
#include "tinycolormap.hpp"

// Fractals:
//  Mandlebrot
//  Nebulabrot (3 colors)
//  Mandelbrot with Power parameter
//  Julia with power parameter and start parameter
//  Buddhabrot (one color)
//  Buddha with power (non cuda)
//  Nebulabrot cuda
//  Spiral Septagon
// TODO
// Allow fractal panning by arrow keys
// Mandelbrot coloring improvements
//  Work on smooth coloring and colormaps more
// Orbit Traps: Point, Lines, Shapes, pickover stalk
// Color based on pixel in input image
// Interior coloring
// reset all variables on fractal switch
// "Reflect colormap toggle" - is it actually reflected on gui
// save position in file name
// make font bold black
// memory:
// save position and scale (just x?) and miters and zconst and power to db file
// display saved position database
// apply specified position from database
// apply next position from database (button?)
// display if position in database on gui

using namespace std;
namespace fs = std::filesystem;

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
const int IMAGE_HEIGHT = 1440;

// const int IMAGE_WIDTH = 1600;
// const int IMAGE_HEIGHT = 1200;

// The zoom and rotation and panning and color of the fractal

enum class ColoringAlgo { MULTICYCLE, SMOOTH, USE_IMAGE, SHADOW_MAP };

enum class ColorCycle { CC8, CC16, CC32, CC64, CC128, CC256 };

enum class InteriorColoringAlgo {
  SOLID,
  MULTICYCLE,
  USE_IMAGE,
  TRIG,
  TRIG2,
  DIST,
  DIST2,
  TEMP
};
#define LAST_INTERIOR_COLOR_ALGO ((unsigned int)InteriorColoringAlgo::DISTANCE)
unsigned int interior_color_adjust = 0;

// std::is_trivially_copyable
class ReferenceFrame {
 public:
  // x-y rotation angle [future]
  float theta;

  // View
  //
  double xstart;          // in fractal coordinates
  double ystart;          // in fractal coordinates
  double xdelta;          // delta per pixel in fractal coordinates
  double ydelta;          // delta per pixel in fractal coordinates
  double current_width;   // in pixels
  double current_height;  // in pixels

  // Zoom Level
  double displayed_zoom;
  double requested_zoom;

  // zoom crop area
  bool show_selection;

  // Color Palletes
  ColoringAlgo color_algo;
  int color_cycle_size;

  tinycolormap::ColormapType palette;
  bool reflect_palette;

  // Escape Image coloring
  unsigned int escape_image_w;
  unsigned int escape_image_h;
  bool image_loaded;

  // normal maps
  double light_pos_r;
  double light_pos_i;
  double light_angle;
  double light_height;

  // Random Sample faster but less exact rendering
  bool random_sample = false;

  // Original total Pixels
  double original_width;
  double original_height;

  ReferenceFrame(float _thetaxy, double _zoom)
      : theta{_thetaxy}, displayed_zoom{_zoom} {};

};  // ReferenceFrame

// Non Serializable part of Fractal description thats too big to store in a key
class NSReferenceFrame {
 public:
  vector<string> color_cycle_size_names{string("8"),   string("16"),
                                        string("32"),  string("64"),
                                        string("128"), string("256")};
  vector<string> color_names{string("Parula"),
                             string("Heat"),
                             string("Jet"),
                             string("Hot"),
                             string("Turbo"),
                             string("Gray"),
                             string("Magma"),
                             string("Inferno"),
                             string("Plasma"),
                             string("Viridis"),
                             string("Cividis"),
                             string("Github"),
                             string("Cubehelix"),
                             string("UF16")};
  sf::Texture escape_texture;
  sf::Image escape_image;
};  // NSReferenceFrame

ReferenceFrame R(0, 1.0);
NSReferenceFrame NSR;  // non serializable


class ReferenceFrameInt {
 public:
  // Color Palletes
  InteriorColoringAlgo color_algo;
  int color_cycle_size;

  tinycolormap::ColormapType palette;
  bool reflect_palette;
  ReferenceFrameInt(InteriorColoringAlgo _ca, int _ccs, tinycolormap::ColormapType _p, bool _rp)
      : color_algo{_ca},
        color_cycle_size{_ccs},
        palette{_p},
        reflect_palette{_rp} {};
};  // ReferenceFrameInt

ReferenceFrameInt RI(InteriorColoringAlgo::SOLID, 256,
                     tinycolormap::ColormapType::UF16, false);


// Submodel for different fractals
// #include "fractals.h" SupportedFractal (shared with cuda)

// struct SupportedFractal {
//   std::string name;
//   bool cuda_mode;
//   bool probabalistic; //i.e. like buddha - affects thread model
//   bool julia;
//   bool anti;
//   std::vector<double> xMinMax;  // default x min max
//   std::vector<double> yMinMax;  // default min max
//   std::vector<unsigned int> current_max_iters;
//   std::vector<unsigned int> default_max_iters;
//   double current_power;
//   double default_power;
//   std::complex<double> current_zconst;
//   std::complex<double> default_zconst;
// };

vector<SupportedFractal> FRAC = {
    {string("Mandelbrot_300"),
     false,
     false,
     false,  // julia
     false,
     {-2.50, 1.5},
     {-1.4, 1.4},
     {300, 0, 0},
     {300, 0, 0},
     2,
     2,
     complex<double>{0, 0},
     complex<double>{0, 0},
     2,
     2},
    {string("Mandelbrot_1000"),
     false,
     false,
     false,  // julia
     false,
     {-2.5, 1.5},
     {-1.4, 1.4},
     {1000, 0, 0},
     {1000, 0, 0},
     2,
     2,
     complex<double>{0, 0},
     complex<double>{0, 0},
     2,
     2},
    {string("Julia"),
     false,
     false,
     true,  // julia
     false,
     {-2.2, 2.2},
     {-1.2, 1.2},
     {300, 0, 0},
     {300, 0, 0},
     2,
     2,
     complex<double>{-0.79, 0.15},
     complex<double>{-0.79, 0.15},
     2,
     2},
    {string("Spiral_Septagon"),
     false,
     false,
     false,
     false,
     {-2.2, 2.2},
     {-1.4, 1.4},
     {300, 0, 0},
     {300, 0, 0},
     7,
     7,
     complex<double>{0, 0},
     complex<double>{0, 0},
     2,
     2},
    {string("Buddhabrot"),  // not going to be zoomable and pannable
     true,
     true,
     false,
     false,
     {-2.2, 1.0},
     {-1.2, 1.2},
     {10000, 1000, 100},
     {10000, 1000, 100},
     2,
     2,
     complex<double>{0, 0},
     complex<double>{0, 0},
     2,
     2},
    {string("Buddhabrot_BW"),  // not going to be zoomable and pannable
     true,
     true,
     false,
     false,
     {-2.2, 1.0},
     {-1.2, 1.2},
     {10000, 10000, 10000},
     {10000, 10000, 10000},
     2,
     2,
     complex<double>{0, 0},
     complex<double>{0, 0},
     2,
     2},
    {string("Buddhabrot_General"),  // not going to be zoomable and pannable
     false,
     true,
     false,
     false,
     {-2.2, 1.0},
     {-1.2, 1.2},
     {10000, 1000, 100},
     {10000, 1000, 100},
     2,
     2,
     complex<double>{0, 0},
     complex<double>{0, 0},
     2,
     2},
    {string(
         "Buddhabrot_General_Julia"),  // not going to be zoomable and pannable
     false,
     true,
     true,  // julia
     false,
     {-2.2, 1.0},
     {-1.2, 1.2},
     {10000, 1000, 100},
     {10000, 1000, 100},
     2,
     2,
     complex<double>{0, 0},
     complex<double>{0, 0},
     2,
     2},
    {string(
         "Anti_Buddhabrot_General"),  // not going to be zoomable and pannable
     false,
     true,
     false,
     true,  // anti
     {-2.2, 1.0},
     {-1.2, 1.2},
     {10000, 10000, 10000},
     {10000, 10000, 10000},
     2,
     2,
     complex<double>{0, 0},
     complex<double>{0, 0},
     2,
     2},
    {string("Anti_Buddhabrot_Small"),  // not going to be zoomable and pannable
     false,
     true,
     false,
     true,  // anti
     {-1.5, .25},
     {-1.0, .25},
     {10000, 10000, 10000},
     {10000, 10000, 10000},
     2,
     2,
     complex<double>{0, 0},
     complex<double>{0, 0},
     2,
     2},
    {string("Nova_z6+z3-1"),
     false,
     false,
     false,
     false,
     {-2.5, 1.5},
     {-1.4, 1.4},
     {300, 0, 0},
     {300, 0, 0},
     6,
     6,
     complex<double>{0, 0},
     complex<double>{0, 0},
     2,
     2},
    {string("Newton_z6+z3-1"),
     false,
     false,
     false,
     false,
     {-2.5, 1.5},
     {-1.4, 1.4},
     {300, 0, 0},
     {300, 0, 0},
     6,
     6,
     complex<double>{0, 0},
     complex<double>{0, 0},
     2,
     2},
};

bool update_and_draw = true;  // stop using cpu for a bit

bool save_and_exit = true;

bool hide = false;

const int FRACTAL_VERSION{1};

std::string key_version =
    std::string{"fractal_key_version_"} + to_string(FRACTAL_VERSION);
// Should be trivially_copyable/serializable
class SavedFractal {
 public:
  int version = FRACTAL_VERSION;
  // p_model->current_fractal
  // FRAC[p_model->current_fractal].current_power
  // FRAC[p_model->current_fractal].current_max_iters[0]
  // FRAC[p_model->current_fractal].current_zconst
  int valid;
  unsigned int current_fractal;
  unsigned int current_max_iters[3];
  double current_power;
  std::complex<double> current_zconst;
  double current_escape_r;

  ReferenceFrame RF;
  // but not NSR

  SavedFractal(float _thetaxy, double _zoom) : valid{0}, RF{_thetaxy, _zoom} {};
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

inline void get_iteration_color(const int iter_ix, const int iters_max,
                                const complex<double> &zfinal,
                                complex<double> &derivative, int *p_rcolor,
                                int *p_gcolor, int *p_bcolor) {
  // palette: UF16, Viridis, Plasma, Jet, Hot, Heat, Parula, Gray, Cividis,
  // Github, UF16(added) cycle_size: 8,16,32,64,128,256 color_algo: Smooth,
  // MultiCycle

  if (R.color_algo == ColoringAlgo::USE_IMAGE) {
    double rd, ri;
    double xi, yi;
    xi = abs(modf(zfinal.real() * 2, &rd));
    yi = abs(modf(zfinal.imag() * 2, &ri));
    // xi = abs(zfinal.real() - (long long)zfinal.real());
    // yi = abs(zfinal.imag() - (long long)zfinal.imag());
    sf::Color color = NSR.escape_image.getPixel(xi * (R.escape_image_w - 1),
                                                yi * (R.escape_image_h - 1));
    *p_rcolor = color.r;
    *p_gcolor = color.g;
    *p_bcolor = color.b;
    return;
  } else if (R.color_algo == ColoringAlgo::SHADOW_MAP) {
    const double h2 = R.light_height;  // height factor of the incoming light
    const double angle = R.light_angle / 360;  // incoming direction of light
    const complex<double> I(0.0, 1.0);
    complex<double> u;
    double t;
    const double pi = 3.14159265358979323846;
    complex<double> v =
        std::exp(I * complex<double>((angle * 2 * pi),
                                     0));  // unit 2D vector in this direction
    // incoming light 3D vector = (v.re, v.im, h2)

    u = zfinal / derivative;
    u = u / abs(u);  // normal vector : (u.re, u.im, 1)
    t = u.real() * v.real() + u.imag() * v.imag() +
        h2;            // dot product with the incoming light
    t = t / (1 + h2);  // rescale so that t does not get bigger than 1
    if (t < 0) t = 0;
      // a+t(b-a) black -> white
#if 0
		if (p_rcolor != 0) *p_rcolor = t * 255;
		if (p_gcolor != 0) *p_gcolor = t * 255;
		if (p_bcolor != 0) *p_bcolor = t * 255;
#else
    // colormap
    int i = (int)(t * 256) % R.color_cycle_size;
    tinycolormap::Color color(0.0, 0.0, 0.0);
    if (R.reflect_palette)
      color = tinycolormap::GetColorR(
          i / static_cast<double>(R.color_cycle_size), R.palette);
    else
      color = tinycolormap::GetColor(
          i / static_cast<double>(R.color_cycle_size), R.palette);

    *p_rcolor = 255 * color.r();
    *p_gcolor = 255 * color.g();
    *p_bcolor = 255 * color.b();
#endif
    return;
  }

  if (R.palette == tinycolormap::ColormapType::UF16) {
    // Ultra Fractal Default non smooth
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

    int i = iter_ix % 16;
    if (R.reflect_palette) {
      i = iter_ix % 32;
      if (i >= 16) i = 31 - i;
    }

    *p_rcolor = mapping[i][0];
    *p_gcolor = mapping[i][1];
    *p_bcolor = mapping[i][2];
    return;
  }

  // Using tinycolormap

  if (R.color_algo == ColoringAlgo::MULTICYCLE) {
    // colormap non smooth
    int i = iter_ix % R.color_cycle_size;
    tinycolormap::Color color(0.0, 0.0, 0.0);
    if (R.reflect_palette)
      color = tinycolormap::GetColorR(
          i / static_cast<double>(R.color_cycle_size), R.palette);
    else
      color = tinycolormap::GetColor(
          i / static_cast<double>(R.color_cycle_size), R.palette);

    *p_rcolor = 255 * color.r();
    *p_gcolor = 255 * color.g();
    *p_bcolor = 255 * color.b();
  } else if (R.color_algo == ColoringAlgo::SMOOTH) {
    double smooth = ((iter_ix + 1 - log(log2(abs(zfinal)))));  // 0 -> iters_max
    tinycolormap::Color color(0.0, 0.0, 0.0);
    if (R.reflect_palette)
      color = tinycolormap::GetColorR(smooth / iters_max, R.palette);
    else
      color = tinycolormap::GetColor(smooth / iters_max, R.palette);

    *p_rcolor = 255 * color.r();
    *p_gcolor = 255 * color.g();
    *p_bcolor = 255 * color.b();
  }
  return;

  // smooth but use chunked colormap
  //  double smooth = ((iter_ix + 1 - log(log2(abs(z))))); //0 -> iters_max

  // //colormap portions - chunk colormap into 32
  // double chunked_smooth = std::round(smooth * 8.0)/8.0;

  // const tinycolormap::Color color =
  // tinycolormap::GetColor(chunked_smooth/static_cast<double>(iters_max),
  // tinycolormap::ColormapType::Viridis);

  // *p_rcolor = 255*color.r();
  // *p_gcolor = 255*color.g();
  // *p_bcolor = 255*color.b();

  // Basic coloring:
  //     return (255 * iter_ix) / (iters_max - 1);
  //   smooth coloring ????    mu = N + 1 - log (log  |Z(N)|) / log 2
  // double smooth = ((iter_ix + 1 - log(log2(abs(z))))/iters_max; //0 -> 1

  // *p_rcolor = (255*iter_ix) / (iters_max);
  // *p_gcolor = 255 - (255*iter_ix) / (iters_max);
  // *p_bcolor = clamp(128 + (255*iter_ix) / (iters_max),(unsigned
  // int)0,(unsigned int)255);
}

inline void get_iteration_interior_color(const complex<double> &zstart,
                                         const complex<double> &zfinal,
                                         unsigned int iters_max,
                                         double distancei, double distancer,
                                         int *p_rcolor, int *p_gcolor,
                                         int *p_bcolor) {
  const double pi = 3.14159265358979323846;

  if ((interior_color_adjust == 0) &&
      (RI.color_algo != InteriorColoringAlgo::SOLID))
    interior_color_adjust = 1;

  switch (RI.color_algo) {
    case InteriorColoringAlgo::SOLID: {
      *p_rcolor = interior_color_adjust & 0xff;
      *p_gcolor = (interior_color_adjust & 0xff00) >> 8;
      *p_bcolor = (interior_color_adjust & 0xff0000) >> 16;
      return;
    } break;
    case InteriorColoringAlgo::MULTICYCLE: {
      if (RI.palette == tinycolormap::ColormapType::UF16) {
        // Ultra Fractal Default non smooth
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

        int i = (interior_color_adjust *10 * (int)(distancer + distancei)) % 16;
        if (R.reflect_palette) {
          i = (interior_color_adjust * 10 * (int)(distancer + distancei)) % 32;
          if (i >= 16) i = 31 - i;
        }

        *p_rcolor = mapping[i][0];
        *p_gcolor = mapping[i][1];
        *p_bcolor = mapping[i][2];
        return;
      }

      // colormap non smooth
      int i = (interior_color_adjust * (int)(distancer + distancei)) %
              RI.color_cycle_size;
      tinycolormap::Color color(0.0, 0.0, 0.0);
      if (RI.reflect_palette)
        color = tinycolormap::GetColorR(
            i / static_cast<double>(RI.color_cycle_size), RI.palette);
      else
        color = tinycolormap::GetColor(
            i / static_cast<double>(RI.color_cycle_size), RI.palette);

      *p_rcolor = 255 * color.r();
      *p_gcolor = 255 * color.g();
      *p_bcolor = 255 * color.b();
      return;
    } break;
    case InteriorColoringAlgo::USE_IMAGE: {
      double rd, ri;
      double xi, yi;
      xi = abs(
          modf(zstart.real() * (2 / (interior_color_adjust*R.displayed_zoom)),
               &rd));  // -1 -> 1
      yi = abs(modf(
          zstart.imag() * (2 / (interior_color_adjust * R.displayed_zoom)), &ri));
      // xi = abs(zstart.real() - (long long)zstart.real());
      // yi = abs(zstart.imag() - (long long)zstart.imag());
      double xp = (xi) * (R.escape_image_w - 1);
      double yp = (yi) * (R.escape_image_h - 1);
      sf::Color color = NSR.escape_image.getPixel(xp, yp);
      *p_rcolor = color.r;
      *p_gcolor = color.g;
      *p_bcolor = color.b;
      return;
    } break;
    case InteriorColoringAlgo::TRIG: {
      *p_rcolor = 255 * (cos(zfinal.imag() + zfinal.real())) * 0.1 *
                  ((interior_color_adjust / R.displayed_zoom) *
                   (distancer + distancei))  / (iters_max);
      *p_gcolor = 255 * (sin(zfinal.real() + zfinal.real())) * 0.1 *
                  ((interior_color_adjust / R.displayed_zoom) *
                   (distancer + distancei))  / (iters_max);
      *p_bcolor = 255 * (atan(zfinal.imag() + zfinal.real())) * 0.1 *
                  ((interior_color_adjust / R.displayed_zoom) *
                   (distancer + distancei)) / (iters_max);
      return;
    } break;
    case InteriorColoringAlgo::TRIG2: {
      return;
    } break;
    case InteriorColoringAlgo::DIST: {
      *p_rcolor = interior_color_adjust * 255 * (distancer) / (iters_max);
      *p_gcolor = interior_color_adjust * 255 * (distancei) / (iters_max);
      *p_bcolor = interior_color_adjust * 255 * (distancer + distancei) / (iters_max);
      return;
    } break;
    case InteriorColoringAlgo::DIST2: {
      *p_rcolor = 255.0 * (cos(distancer + distancei)) *
                  ((1.0/interior_color_adjust) *
                   (distancer + distancei)) /
                  (iters_max);
      *p_gcolor = 255.0 * (sin(distancer + distancei)) *
                  ((1.0 / interior_color_adjust) *
                   (distancer + distancei)) /
                  (iters_max);
      *p_bcolor = 255.0 * (atan(distancer + distancei)) *
                  ((1.0 / interior_color_adjust) *
                   (distancer + distancei)) /
                  (iters_max);
      return;
    } break;
    case InteriorColoringAlgo::TEMP: {
      *p_rcolor = 255.0 * (cos(distancer) + sin(distancei)) *
                  ((1.0/interior_color_adjust) * (distancer + distancei)) /
                  (iters_max);
      *p_gcolor = 255.0 * (sin(distancer) + cos(distancei)) *
                  ((1.0/interior_color_adjust) * (distancer + distancei)) /
                  (iters_max);
      *p_bcolor = 255.0 * ((distancer + distancei)) *
                  ((1.0/interior_color_adjust) * (distancer + distancei)) /
                  (iters_max);
      return;
      return;
    } break;
  }
}

void mandelbrot_iterations_to_escape(double x, double y, unsigned int iters_max,
                                     int *p_rcolor, int *p_gcolor,
                                     int *p_bcolor, double power,
                                     complex<double> zconst, double escape_r,
                                     bool julia, unsigned long long &in,
                                     unsigned long long &out) {
  complex<double> point(x, y);
  complex<double> z(0, 0);
  complex<double> zn(0, 0);
  complex<double> dc(R.light_pos_r, R.light_pos_i);
  complex<double> derivative = dc;
  unsigned int iter_ix = 0;
  double distancei = 0;
  double distancer = 0;

  if (julia) z = point;

  while (abs(z) < (escape_r * escape_r) && iter_ix <= iters_max) {
    if (julia)
      zn = pow(z, power) + zconst;  // With Julia you dont add Point
    else {
      if (R.color_algo == ColoringAlgo::SHADOW_MAP)
        derivative =
            derivative * complex<double>(2, 0) * z + dc;  // shadow map only
      zn = pow(z, power) + point;
    }
    // how far did we travel during orbit
    distancei += (z.imag() - zn.imag()) * (z.imag() - zn.imag());
    distancer += (z.real() - zn.real()) * (z.real() - zn.real());
    z = zn;
    // z = z*z + point;
    iter_ix++;
  }

  if (iter_ix < iters_max)
    ++out;
  else
    ++in;

  if (iter_ix < iters_max) {
    get_iteration_color(iter_ix, iters_max, z, derivative, p_rcolor, p_gcolor,
                        p_bcolor);
  } else {  // set interior set color
    get_iteration_interior_color(point, z, iters_max, distancei, distancer,
                                 p_rcolor, p_gcolor, p_bcolor);
  }
}

void spiral_septagon_iterations_to_escape(
    double x, double y, unsigned int iters_max, int *p_rcolor, int *p_gcolor,
    int *p_bcolor, double power, complex<double> zconst, double escape_r,
    bool julia, unsigned long long &in, unsigned long long &out) {
  complex<double> point(x, y);
  complex<double> z(x, y);
  complex<double> derivative(1, 0);
  unsigned int iter_ix = 0;

  while (abs(z) < (escape_r * escape_r) && iter_ix <= iters_max) {
    z = (pow(z, power) - (0.7 / 5)) / z;
    iter_ix++;
  }

  if (iter_ix < iters_max)
    ++out;
  else
    ++in;

  if (iter_ix < iters_max) {
    get_iteration_color(iter_ix, iters_max, z, derivative, p_rcolor, p_gcolor,
                        p_bcolor);
  } else  // set interior set color
  {
    if (p_rcolor != 0) *p_rcolor = 0;
    if (p_gcolor != 0) *p_gcolor = 0;
    if (p_bcolor != 0) *p_bcolor = 0;
  }
}

complex<double> Fz6(complex<double> z) {
  return pow(z, 6) + pow(z, 3) - complex<double>(1, 0);
}

complex<double> dFz6(complex<double> z) {
  return complex<double>(6, 0) * pow(z, 5) + complex<double>(3, 0) * pow(z, 2);
}

vector<complex<double>> Fz6_roots{
    complex<double>(0.586992498352664, 1.016700830808605),
    complex<double>(-1.17398499670533, 0),
    complex<double>(0.586992498352664, -1.016700830808605),
    complex<double>(-0.4258998211039621, -0.737680128975117),
    complex<double>(0.851799642079243, 0),
    complex<double>(-0.4258998211039621, 0.737680128975117)};

void nova_z6_iterations_to_escape(double x, double y, unsigned int iters_max,
                                  int *p_rcolor, int *p_gcolor, int *p_bcolor,
                                  double power, complex<double> zconst,
                                  double escape_r, bool julia,
                                  unsigned long long &in,
                                  unsigned long long &out) {
  complex<double> point(x, y);
  complex<double> z(x, y);
  complex<double> zprev(x, y);
  complex<double> derivative(1, 0);
  unsigned int iter_ix = 0;
  double tolerance = 0.000001;

  // Mandelbrot nova
  //  zconst = z;
  //  z = Fz6_roots[0];

  while (iter_ix <= iters_max) {
    zprev = z;
    z = z - Fz6(z) / dFz6(z) + zconst;

    complex<double> diff = z - zprev;
    if ((abs(diff.real()) < tolerance) && (abs(diff.imag()) < tolerance)) {
      break;
    }
    iter_ix++;
  }

  if (iter_ix < iters_max)
    ++out;
  else
    ++in;

  if (iter_ix < iters_max) {
    get_iteration_color(iter_ix, iters_max, z, derivative, p_rcolor, p_gcolor,
                        p_bcolor);
  } else  // set interior set color
  {
    if (p_rcolor != 0) *p_rcolor = 0;
    if (p_gcolor != 0) *p_gcolor = 0;
    if (p_bcolor != 0) *p_bcolor = 0;
  }
}

void newton_z6_iterations_to_escape(double x, double y, unsigned int iters_max,
                                    int *p_rcolor, int *p_gcolor, int *p_bcolor,
                                    double power, complex<double> zconst,
                                    double escape_r, bool julia,
                                    unsigned long long &in,
                                    unsigned long long &out) {
  complex<double> point(x, y);
  complex<double> z(x, y);
  complex<double> derivative(1, 0);
  unsigned int iter_ix = 0;
  double tolerance = 0.000001;
  unsigned int which_root = 0;

  while (iter_ix <= iters_max) {
    z = z - Fz6(z) / dFz6(z);
    bool root_found = false;

    for (unsigned int i = 0; i < Fz6_roots.size(); ++i) {
      complex<double> diff = z - Fz6_roots[i];

      if ((abs(diff.real()) < tolerance) && (abs(diff.imag()) < tolerance)) {
        root_found = true;
        which_root = i;
        break;
      }
    }
    if (root_found == true) break;
    iter_ix++;
  }

  if (iter_ix < iters_max)
    ++out;
  else
    ++in;

  if (iter_ix < iters_max) {
    // found which_root
    // color the root
    //  *p_rcolor = 64+32*which_root;
    //  *p_gcolor = 255 - 32*which_root;
    //  *p_bcolor = 128 + 16*which_root;
    int color_ix = 0;
    if (R.palette == tinycolormap::ColormapType::UF16)
      color_ix = 2 + 2 * which_root;
    else
      color_ix = 1 + (iters_max / 7) * which_root;

    get_iteration_color(color_ix, iters_max, z, derivative, p_rcolor, p_gcolor,
                        p_bcolor);

    // color any root
    // get_iteration_color(iter_ix, iters_max, z, p_rcolor, p_gcolor, p_bcolor);

  } else  // set interior set color
  {
    if (p_rcolor != 0) *p_rcolor = 0;
    if (p_gcolor != 0) *p_gcolor = 0;
    if (p_bcolor != 0) *p_bcolor = 0;
  }
}

void generate_buddhabrot_trail(const complex<double> &c, unsigned int iters_max,
                               vector<complex<double>> &trail, int power,
                               complex<double> zconst, double escape_r,
                               bool julia, bool anti, unsigned long long &in,
                               unsigned long long &out) {
  unsigned int iter_ix = 0;
  complex<double> z(0, 0);
  // unsigned int max_iters_in_cycle = iters_max; //for long_orbit

  // bool long_orbit = true;

  // Modified Julia to make buddhabrot do something
  if (julia) {
    z = c;
    zconst = zconst + c;
  }

  bool cycles = true;
  if (cycles) {
    struct complex_double_hash {
      std::size_t operator()(const complex<double> &c) const {
        return std::hash<double>()(c.real()) ^ std::hash<double>()(c.imag());
      }
    };

    unordered_map<complex<double>, long, complex_double_hash> point_trail;
    // unordered_map<long, std::complex<double>> cycle_present;

    trail.clear();
    trail.reserve(iters_max + 1);

    while (iter_ix < iters_max && abs(z) < (escape_r * escape_r)) {
      if (julia)
        z = pow(z, power) + zconst;  // With Julia you dont add Point usually
      else
        z = pow(z, power) + c;
      // z = z*z + c;

      auto search = point_trail.find(z);
      if (search != point_trail.end()) {
        // max_iters_in_cycle = iter_ix;
        iter_ix = iters_max;
        break;
      }
      point_trail[z] = iter_ix;

      ++iter_ix;
      trail.push_back(z);
    }

  } else {
    trail.clear();
    trail.reserve(iters_max + 1);

    while (iter_ix < iters_max && abs(z) < 2.0) {
      if (julia)
        z = pow(z, power) + zconst;  // With Julia you dont add Point usually
      else
        z = pow(z, power) + c;
      // z = z*z + c;
      ++iter_ix;
      trail.push_back(z);
    }
  }

  //
  if (iter_ix == iters_max) {
    // ANTI
    ++in;
    if (!anti) trail.clear();  // reject in set points for regular buddhabrot

    // if ((cycles) && (max_iters_in_cycle < ((31/32) * iters_max)))
    // trail.clear(); if ((cycles) && (max_iters_in_cycle == iters_max))
    // trail.clear();

    // if (long_orbit) trail.clear();
  } else {
    // REGULAR
    ++out;

    // if (long_orbit) {
    //   if (iter_ix < (15/16)*iters_max) {trail.clear(); return;}
    //   else return;
    // }

    if (anti) trail.clear();  // reject escaped points for anti-buddhabrot
  }

  // return trail
}

// fun-illy enough we dont need the complex C++ thread sync primitives
// this is to prevent unnecessary calculation when we request 2 zooms in a row
// quickly
#define MAX_THREADS 32
bool thread_asked_to_reset[MAX_THREADS];
unsigned int thread_iteration[MAX_THREADS];

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
    R.reflect_palette = false;
    R.light_pos_r = 1;
    R.light_pos_i = 0;
    R.light_angle = 45;
    R.light_height = 1.5;
    R.xdelta =
        (FRAC[current_fractal].xMinMax[1] - FRAC[current_fractal].xMinMax[0]) /
        R.original_width;
    R.ydelta =
        (FRAC[current_fractal].yMinMax[1] - FRAC[current_fractal].yMinMax[0]) /
        R.original_height;

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

  // switching fractals
  void reset_fractal_params() {
    FRAC[current_fractal].current_max_iters =
        FRAC[current_fractal].default_max_iters;
    FRAC[current_fractal].current_power = FRAC[current_fractal].default_power;
    FRAC[current_fractal].current_zconst = FRAC[current_fractal].default_zconst;
    FRAC[current_fractal].current_escape_r =
        FRAC[current_fractal].default_escape_r;
  }

  // could just be tuning fractal
  void reset_fractal_and_reference_frame() {
    R.displayed_zoom = 1.0;
    R.requested_zoom = 1.0;
    R.xstart = FRAC[current_fractal].xMinMax[0];
    R.ystart = FRAC[current_fractal].yMinMax[0];
    R.xdelta =
        (FRAC[current_fractal].xMinMax[1] - FRAC[current_fractal].xMinMax[0]) /
        R.original_width;
    R.ydelta =
        (FRAC[current_fractal].yMinMax[1] - FRAC[current_fractal].yMinMax[0]) /
        R.original_height;
    R.current_height = R.original_height;
    R.current_width = R.original_width;
    R.show_selection = false;  // mouse click on menu is not a selection
    R.light_pos_r = 1;
    R.light_pos_i = 0;
    R.light_angle = 45;
    R.light_height = 1.5;
    hitsums = 0;
    maxred = 0;
    maxgreen = 0;
    maxblue = 0;

    memset(&stats, 0, sizeof(stats));
    if (FRAC[current_fractal].probabalistic != true) {
      zoomFractal(1.0);
    }

    for (unsigned int tix = 0; tix < this->num_threads; ++tix) {
      thread_asked_to_reset[tix] = true;
      thread_iteration[tix] = 0;
      image_wraps[tix] = 0;
      current_x[tix] = FRAC[current_fractal].xMinMax[0] + deltax * tix;
      current_y[tix] = FRAC[current_fractal].yMinMax[0] + deltay * tix;
    }

    // TODO zero the per thread hits as well
    std::lock_guard<std::mutex> guard(
        thread_result_report_mutex);  // keep out other threads
    redTrailHits.resize(0);
    greenTrailHits.resize(0);
    blueTrailHits.resize(0);
    createBuddhabrot();
  }

  // thread pool is currently started outside the model
  void fractal_thread(int tix, std::future<void> terminate, bool *p_reset,
                      unsigned int *p_iteration) {
    // cout << "fractal thread " << tix << " running with oversampling: " << 4.0
    // << endl;

    deltax = 1.0 / (4.0 * IMAGE_WIDTH);
    deltay = 1.0 / (4.0 * IMAGE_HEIGHT);
    current_x[tix] = FRAC[current_fractal].xMinMax[0] + deltax * tix;
    current_y[tix] = FRAC[current_fractal].yMinMax[0] + deltay * tix;

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
      // see if we should terminate - since we're exiting anyway dont bother to
      // pass it into the threads
      if (terminate.wait_for(std::chrono::nanoseconds(0)) !=
          std::future_status::timeout) {
        // std::cout << "Terminate thread requested: " << tix << std::endl;
        break;
      }

      // Don't update if we want to draw just one
      if ((save_and_exit == true) && (p_iteration[tix] == 2)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }

      // Don't update if we want to pause cpu usage
      if (update_and_draw == false) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        continue;
      }

      // std::this_thread::sleep_for(std::chrono::milliseconds(10));

      // do work if we didnt terminate

      // Non Probabalistic fractals
      if (FRAC[current_fractal].probabalistic != true) {
        reset_detected = getImagePixels(R.xstart, R.ystart, R.xdelta, R.ydelta,
                                        tix, p_reset);
        if (reset_detected == true) {
          reset_detected = false;
          // Clear any data generated so far
          for (auto &v : color) v.clear();
        }

        p_iteration[tix]++;
        // cout << "tix iteration: " << p_iteration[tix] << endl;

        continue;
      }

      // Probabalistic fractals

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
        reset_detected = generateMoreTrailHits(redHits, greenHits, blueHits,
                                               &p_reset[tix], tix);
      }

      if (reset_detected == true) {
        reset_detected = false;
        // Clear any data generated so far
        for (auto &v : redHits) v.clear();
        for (auto &v : greenHits) v.clear();
        for (auto &v : blueHits) v.clear();
        continue;  // dont merge fractal per thread trails
      }

      // auto end = chrono::high_resolution_clock::now();
      // cout << "sample time " << tix << " " <<
      // chrono::duration_cast<chrono::milliseconds>(end - start).count() << "
      // ms" << endl;

      // merge trail hits into the instance of the class (but dont make image)
      mergeHits(redHits, greenHits, blueHits);  // mutex inside
    }

    // cout << "fractal thread exiting: " << tix << endl;
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

  void cudaPresent() {
    int count = cuda_info();

    if (count == 0) {
      cuda_detected = false;
      return;
    }
    cuda_detected = true;
  }

  void cudaTest() {
    int count = cuda_info();

    if (count == 0) {
      cuda_detected = false;
      return;
    }
    cuda_detected = true;

    // test some cuda vector addition and return of sum vector to host
    // just to see cuda is working
    cuda_vec_add(IMAGE_WIDTH, IMAGE_HEIGHT);

    // test a prototype API thats very much like the final one we have to write
    // but doesnt have all the details
    cuda_generate_hits_prototype(IMAGE_WIDTH, IMAGE_HEIGHT);

    // test some cuda hit generation and return of hits to the host - this is
    // the real API used by threads
    SampleStats fakestat;
    cuda_generate_buddhabrot_hits(IMAGE_WIDTH, IMAGE_HEIGHT,
                                  FRAC[current_fractal], fakestat, redTrailHits,
                                  greenTrailHits, blueTrailHits);
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
                             bool *p_reset, int tix) {
    bool reset_detected = false;

    if (image_wraps[tix] > 8) {
      // cout << "thread " << tix << " paused" << endl;
      return reset_detected;
    }

    // looks like we need to do this even if not random
    // if (R.random_sample) {
    //  Randomly sampled pixels
    std::random_device rd;
    static std::mt19937_64 re(rd());
    // uniform_real_distribution<double> xDistribution(
    //     FRAC[current_fractal].xMinMax[0], FRAC[current_fractal].xMinMax[1]);
    // uniform_real_distribution<double> yDistribution(
    //     FRAC[current_fractal].yMinMax[0], FRAC[current_fractal].yMinMax[1]);
    uniform_real_distribution<double> xDistribution(-2, 2);
    uniform_real_distribution<double> yDistribution(-2, 2);
    re.seed(chrono::high_resolution_clock::now().time_since_epoch().count());
    //}

    unsigned long long max_samples =
        100000;  // large enought to overcome thread sleep time

    for (unsigned long long s_ix = 0; s_ix < max_samples; ++s_ix) {
      // see if we should reset
      if (*p_reset == true) {
        *p_reset = false;
        // std::cout << "Reset hits thread requested: " << std::endl;
        reset_detected = true;
        break;
      }

      complex<double> sample;
      if (R.random_sample) {
        // Randomly sampled pixels
        sample = {xDistribution(re), yDistribution(re)};
      } else {
        // Linearly sampled pixels
        sample = {current_x[tix], current_y[tix]};
        double xmin, xmax, ymin, ymax;

        if (FRAC[current_fractal].name == string("Anti_Buddhabrot_Small")) {
          // to draw all orbits
          xmin = -2.2;
          xmax = 1.0;
          ymin = -1.2;
          ymax = 1.2;
        } else {
          xmin = FRAC[current_fractal].xMinMax[0];
          xmax = FRAC[current_fractal].xMinMax[1];
          ymin = FRAC[current_fractal].yMinMax[0];
          ymax = FRAC[current_fractal].yMinMax[1];
        }

        double x = current_x[tix] + num_threads * deltax;
        if (x > xmax) {
          current_x[tix] = xmin + tix * deltax;
          current_y[tix] = current_y[tix] + deltay;
          if (current_y[tix] > ymax) {
            current_y[tix] = ymin;
            image_wraps[tix]++;
            if (image_wraps[tix] > 8) break;
          }
        } else
          current_x[tix] = x;
      }

      stats[current_fractal].total++;  // not atomic....

      if ((FRAC[current_fractal].current_power == 2) &&
          (FRAC[current_fractal].anti == false) &&
          (true == skipInSet(sample))) {
        stats[current_fractal].rejected++;  // not atomic....
        continue;
      }

      vector<complex<double>> trail;

      unsigned int red_max_iters = FRAC[current_fractal].current_max_iters[0];
      unsigned int green_max_iters = FRAC[current_fractal].current_max_iters[1];
      unsigned int blue_max_iters = FRAC[current_fractal].current_max_iters[2];

      generate_buddhabrot_trail(
          sample, red_max_iters, trail, FRAC[current_fractal].current_power,
          FRAC[current_fractal].current_zconst,
          FRAC[current_fractal].current_escape_r, FRAC[current_fractal].julia,
          FRAC[current_fractal].anti, stats[current_fractal].in_set,
          stats[current_fractal].escaped_set);
      saveBuddhabrotTrailToColor(trail, redHits);
      if (0 != trail.size()) {
        sample = complex<double>(sample.real(), -sample.imag());
        generate_buddhabrot_trail(
            sample, red_max_iters, trail, FRAC[current_fractal].current_power,
            FRAC[current_fractal].current_zconst,
            FRAC[current_fractal].current_escape_r, FRAC[current_fractal].julia,
            FRAC[current_fractal].anti, stats[current_fractal].in_set,
            stats[current_fractal].escaped_set);
        saveBuddhabrotTrailToColor(trail, redHits);
      }

      generate_buddhabrot_trail(
          sample, green_max_iters, trail, FRAC[current_fractal].current_power,
          FRAC[current_fractal].current_zconst,
          FRAC[current_fractal].current_escape_r, FRAC[current_fractal].julia,
          FRAC[current_fractal].anti, stats[current_fractal].in_set,
          stats[current_fractal].escaped_set);
      saveBuddhabrotTrailToColor(trail, greenHits);
      if (0 != trail.size()) {
        sample = complex<double>(sample.real(), -sample.imag());
        generate_buddhabrot_trail(
            sample, green_max_iters, trail, FRAC[current_fractal].current_power,
            FRAC[current_fractal].current_zconst,
            FRAC[current_fractal].current_escape_r, FRAC[current_fractal].julia,
            FRAC[current_fractal].anti, stats[current_fractal].in_set,
            stats[current_fractal].escaped_set);
        saveBuddhabrotTrailToColor(trail, greenHits);
      }

      generate_buddhabrot_trail(
          sample, blue_max_iters, trail, FRAC[current_fractal].current_power,
          FRAC[current_fractal].current_zconst,
          FRAC[current_fractal].current_escape_r, FRAC[current_fractal].julia,
          FRAC[current_fractal].anti, stats[current_fractal].in_set,
          stats[current_fractal].escaped_set);
      saveBuddhabrotTrailToColor(trail, blueHits);
      if (0 != trail.size()) {
        sample = complex<double>(sample.real(), -sample.imag());
        generate_buddhabrot_trail(
            sample, blue_max_iters, trail, FRAC[current_fractal].current_power,
            FRAC[current_fractal].current_zconst,
            FRAC[current_fractal].current_escape_r, FRAC[current_fractal].julia,
            FRAC[current_fractal].anti, stats[current_fractal].in_set,
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
                      double ydelta, unsigned int tix, bool *p_reset) {
    bool reset_detected = false;

    // Subdivide x range by tix and num_threads
    unsigned int xrange = R.original_width / num_threads;
    unsigned int xs = tix * xrange;
    unsigned int xe = (tix + 1) * xrange;
    if (tix == num_threads - 1) xe = R.original_width;

    for (unsigned int i = xs; i < xe; i++) {
      for (unsigned int j = 0; j < R.original_height; j++) {
        // see if we should reset
        if (p_reset[tix] == true) {
          p_reset[tix] = false;
          // std::cout << "Reset thread requested: " << tix << std::endl;
          reset_detected = true;
          break;
        }

        double xi = xstart + i * xdelta;
        double yj = ystart + j * ydelta;
        stats[current_fractal].total++;

        int rcolor = 0;
        int gcolor = 0;
        int bcolor = 0;

        if (FRAC[current_fractal].name == string("Spiral_Septagon"))
          spiral_septagon_iterations_to_escape(
              xi, yj, FRAC[current_fractal].current_max_iters[0], &rcolor,
              &gcolor, &bcolor, FRAC[current_fractal].current_power,
              FRAC[current_fractal].current_zconst,
              FRAC[current_fractal].current_escape_r,
              FRAC[current_fractal].julia, stats[current_fractal].in_set,
              stats[current_fractal].escaped_set);
        else if (FRAC[current_fractal].name == string("Nova_z6+z3-1")) {
          nova_z6_iterations_to_escape(
              xi, yj, FRAC[current_fractal].current_max_iters[0], &rcolor,
              &gcolor, &bcolor, FRAC[current_fractal].current_power,
              FRAC[current_fractal].current_zconst,
              FRAC[current_fractal].current_escape_r,
              FRAC[current_fractal].julia, stats[current_fractal].in_set,
              stats[current_fractal].escaped_set);
        } else if (FRAC[current_fractal].name == string("Newton_z6+z3-1")) {
          newton_z6_iterations_to_escape(
              xi, yj, FRAC[current_fractal].current_max_iters[0], &rcolor,
              &gcolor, &bcolor, FRAC[current_fractal].current_power,
              FRAC[current_fractal].current_zconst,
              FRAC[current_fractal].current_escape_r,
              FRAC[current_fractal].julia, stats[current_fractal].in_set,
              stats[current_fractal].escaped_set);
        } else
          mandelbrot_iterations_to_escape(
              xi, yj, FRAC[current_fractal].current_max_iters[0], &rcolor,
              &gcolor, &bcolor, FRAC[current_fractal].current_power,
              FRAC[current_fractal].current_zconst,
              FRAC[current_fractal].current_escape_r,
              FRAC[current_fractal].julia, stats[current_fractal].in_set,
              stats[current_fractal].escaped_set);

        color[i][j] = sf::Color(rcolor, gcolor, bcolor);
      }

      if (reset_detected == true) break;
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
    double xstart = R.xstart + (maxx - minx) * (R.displayed_zoom - newzoom) /
                                   2.0;  // pixels

    R.xstart = xstart;

    double ystart = R.ystart + (maxy - miny) * (R.displayed_zoom - newzoom) /
                                   2.0;  // pixels

    R.ystart = ystart;

    R.current_width = newzoom * R.original_width;
    R.current_height = newzoom * R.original_height;

    R.displayed_zoom = newzoom;

    cout << "zoom: " << R.displayed_zoom;
    cout << "  cdims: " << R.current_width << " " << R.current_height << " ";
    cout.precision(10);
    cout << scientific << " starts: " << R.xstart << " " << R.ystart << " ";
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
    double xstart = R.xstart - ((maxx - minx) / R.original_width) *
                                   (R.original_width / 2.0 - xcenter) *
                                   R.displayed_zoom;  // pixels

    R.xstart = xstart;

    double ystart = R.ystart - ((maxy - miny) / R.original_height) *
                                   (R.original_height / 2.0 - ycenter) *
                                   R.displayed_zoom;  // pixels

    R.ystart = ystart;

    cout << "pan: " << xcenter << " " << ycenter << " ";
    cout << "  cdims: " << R.current_width << " " << R.current_height;
    cout.precision(10);
    cout << scientific << " starts: " << R.xstart << " " << R.ystart << " ";
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
    // draw image from latest data
    if (FRAC[current_fractal].probabalistic == true) {
      {
        std::lock_guard<std::mutex> guard(thread_result_report_mutex);
        rebuildImageFromHits();  // SetImagePixels
      }
    } else {
      setImagePixels(R.xstart, R.ystart, R.xdelta, R.ydelta);
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

  SampleStats stats[16];  // indexed by fractal

  unsigned int num_threads;

  // point to try next if not using random sampling
  double current_x[MAX_THREADS];
  double current_y[MAX_THREADS];
  int image_wraps[MAX_THREADS];
  double deltax;
  double deltay;

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

  // Non buddha fractals
  vector<vector<sf::Color>> color;
};  // FractalModel

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
  p_model->reset_fractal_and_reference_frame();
  p_model->reset_fractal_params();
  setGuiElementsFromModel(pgui, p_model);
}

void signalPower(shared_ptr<FractalModel> p_model, shared_ptr<tgui::Gui> pgui,
                 const sf::String &value) {
  updateGuiElements(pgui, p_model);

  double input = 2.0;
  try {
    input = std::stod(value.toAnsiString());
  } catch (const std::invalid_argument &ia) {
    cout << "Invalid " << ia.what() << endl;
    input = 2.0;
  }
  FRAC[p_model->current_fractal].current_power = input;
  p_model->reset_fractal_and_reference_frame();
  setGuiElementsFromModel(pgui, p_model);
}

void signalMIters(shared_ptr<FractalModel> p_model, shared_ptr<tgui::Gui> pgui,
                  int iter_ix, const sf::String &value) {
  unsigned int input = 2.0;
  try {
    input = std::stoi(value.toAnsiString());
  } catch (const std::invalid_argument &ia) {
    cout << "Invalid " << ia.what() << endl;
    input = 300;
  }
  FRAC[p_model->current_fractal].current_max_iters[iter_ix] = input;
}

void signalZconstr(shared_ptr<FractalModel> p_model, shared_ptr<tgui::Gui> pgui,
                   const sf::String &value) {
  updateGuiElements(pgui, p_model);

  double input = 0.0;
  try {
    input = std::stod(value.toAnsiString());
  } catch (const std::invalid_argument &ia) {
    cout << "Invalid " << ia.what() << endl;
    input = 0.0;
  }
  FRAC[p_model->current_fractal].current_zconst = complex<double>(
      input, FRAC[p_model->current_fractal].current_zconst.imag());
  p_model->reset_fractal_and_reference_frame();
  setGuiElementsFromModel(pgui, p_model);
}

void signalZconsti(shared_ptr<FractalModel> p_model, shared_ptr<tgui::Gui> pgui,
                   const sf::String &value) {
  updateGuiElements(pgui, p_model);

  double input = 0.0;
  try {
    input = std::stod(value.toAnsiString());
  } catch (const std::invalid_argument &ia) {
    cout << "Invalid " << ia.what() << endl;
    input = 0.0;
  }
  FRAC[p_model->current_fractal].current_zconst = complex<double>(
      FRAC[p_model->current_fractal].current_zconst.real(), input);
  p_model->reset_fractal_and_reference_frame();
  setGuiElementsFromModel(pgui, p_model);
}

void signal_escape_r(shared_ptr<FractalModel> p_model,
                     shared_ptr<tgui::Gui> pgui, const sf::String &value) {
  updateGuiElements(pgui, p_model);

  double input = 0.0;
  try {
    input = std::stod(value.toAnsiString());
  } catch (const std::invalid_argument &ia) {
    cout << "Invalid " << ia.what() << endl;
    input = 0.0;
  }
  FRAC[p_model->current_fractal].current_escape_r = input;
  // p_model->reset_fractal_and_reference_frame();
  setGuiElementsFromModel(pgui, p_model);
}

void signalSamplingButton(shared_ptr<FractalModel> p_model) {
  if (R.random_sample == true)
    R.random_sample = false;
  else
    R.random_sample = true;

  for (unsigned int tix = 0; tix < p_model->num_threads; ++tix) {
    thread_asked_to_reset[tix] = true;
    thread_iteration[tix] = 0;
    p_model->image_wraps[tix] = 0;
    p_model->current_x[tix] =
        FRAC[p_model->current_fractal].xMinMax[0] + p_model->deltax * tix;
    p_model->current_y[tix] =
        FRAC[p_model->current_fractal].yMinMax[0] + p_model->deltay * tix;
  }
}

void signalColorBox(const int selected) {
  R.palette = static_cast<tinycolormap::ColormapType>(selected);
}

void signalColorCycleBox(const int selected) {
  R.color_cycle_size = 8 * pow(2, selected);
}

void signalCAlgoBox(const int selected) {
  if (selected == 0)
    R.color_algo = ColoringAlgo::MULTICYCLE;
  else if (selected == 1)
    R.color_algo = ColoringAlgo::SMOOTH;
  else if ((selected == 2) && (true == R.image_loaded))
    R.color_algo = ColoringAlgo::USE_IMAGE;
  else if (selected == 3)
    R.color_algo = ColoringAlgo::SHADOW_MAP;
}

void signalButton() {
  if (R.reflect_palette == true)
    R.reflect_palette = false;
  else
    R.reflect_palette = true;
}

// Interior Color
void signalIntColorAdj(shared_ptr<FractalModel> p_model,
                       shared_ptr<tgui::Gui> pgui, const sf::String &value) {
  unsigned int input = 0;
  try {
    input = (unsigned int)stoul(value.toAnsiString(),nullptr,0);
    interior_color_adjust = input;
  } catch (const std::invalid_argument &ia) {
    cout << "Invalid " << ia.what() << endl;
  } catch (...) {
    // input = 0;
  }
}

void signalIntColorBox(const int selected) {
  RI.palette = static_cast<tinycolormap::ColormapType>(selected);
}

void signalIntColorCycleBox(const int selected) {
  RI.color_cycle_size = 8 * pow(2, selected);
}

void signalIntCAlgoBox(const int selected) {
  if (selected == 0)
    RI.color_algo = InteriorColoringAlgo::SOLID;
  else if (selected == 1)
    RI.color_algo = InteriorColoringAlgo::MULTICYCLE;
  else if ((selected == 2) && (true == R.image_loaded))
    RI.color_algo = InteriorColoringAlgo::USE_IMAGE;
  else if (selected == 3)
    RI.color_algo = InteriorColoringAlgo::TRIG;
  else if (selected == 4)
    RI.color_algo = InteriorColoringAlgo::TRIG2;
  else if (selected == 5)
    RI.color_algo = InteriorColoringAlgo::DIST;
  else if (selected == 6)
    RI.color_algo = InteriorColoringAlgo::DIST2;
  else if (selected == 7)
    RI.color_algo = InteriorColoringAlgo::TEMP;
}

void signalIntButton() {
  if (RI.reflect_palette == true)
    RI.reflect_palette = false;
  else
    RI.reflect_palette = true;
}

const int max_saved = 30;
SavedFractal no_fractal{0, 1.0};
int last_loaded_key_ix = -1;
int frac_ix = 0;
int displayed_frac_ix = -1;
vector<SavedFractal> savf(max_saved,
                          no_fractal);  // for saving good looking ones
SavedFractal Last(no_fractal);          // for undo

void signalSaveFractal(shared_ptr<FractalModel> p_model,
                       shared_ptr<tgui::Gui> pgui) {
  updateGuiElements(pgui, p_model);

  // Mainly reference frame stuff
  // p_model->current_fractal
  // FRAC[p_model->current_fractal].current_power
  // FRAC[p_model->current_fractal].current_max_iters[0]
  // FRAC[p_model->current_fractal].current_zconst
  // TBD colors

  savf[frac_ix] = no_fractal;
  savf[frac_ix].version = FRACTAL_VERSION;
  savf[frac_ix].valid = 1;
  savf[frac_ix].current_fractal = p_model->current_fractal;
  savf[frac_ix].current_power = FRAC[p_model->current_fractal].current_power;
  savf[frac_ix].current_max_iters[0] =
      FRAC[p_model->current_fractal].current_max_iters[0];
  savf[frac_ix].current_max_iters[1] =
      FRAC[p_model->current_fractal].current_max_iters[1];
  savf[frac_ix].current_max_iters[2] =
      FRAC[p_model->current_fractal].current_max_iters[2];
  savf[frac_ix].current_zconst = FRAC[p_model->current_fractal].current_zconst;
  savf[frac_ix].current_escape_r =
      FRAC[p_model->current_fractal].current_escape_r;
  savf[frac_ix].RF = R;

  frac_ix++;
  if (frac_ix > max_saved) frac_ix = 0;

  setGuiElementsFromModel(pgui, p_model);
}

void SaveLast(shared_ptr<FractalModel> p_model) {
  Last = no_fractal;
  Last.version = FRACTAL_VERSION;
  Last.valid = 1;
  Last.current_fractal = p_model->current_fractal;
  Last.current_power = FRAC[p_model->current_fractal].current_power;
  Last.current_max_iters[0] =
      FRAC[p_model->current_fractal].current_max_iters[0];
  Last.current_max_iters[1] =
      FRAC[p_model->current_fractal].current_max_iters[1];
  Last.current_max_iters[2] =
      FRAC[p_model->current_fractal].current_max_iters[2];
  Last.current_zconst = FRAC[p_model->current_fractal].current_zconst;
  Last.current_escape_r = FRAC[p_model->current_fractal].current_escape_r;
  Last.RF = R;
}

/* CRC-32C (iSCSI) polynomial in reversed bit order. */
//#define POLY 0x82f63b78

/* CRC-32 (Ethernet, ZIP, etc.) polynomial in reversed bit order. */
#define POLY 0xedb88320

uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len) {
  int k;

  crc = ~crc;
  while (len--) {
    crc ^= *buf++;
    for (k = 0; k < 8; k++) crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
  }
  return ~crc;
}

// std::filesystem::path::preferred_separator
//  / works on windows also
std::string separator{"/"};

void signalImportKeys(shared_ptr<FractalModel> p_model,
                      shared_ptr<tgui::Gui> pgui) {
  updateGuiElements(pgui, p_model);

  std::string filename;

  if (!fs::is_directory(key_version) || !fs::exists(key_version)) {
    fs::create_directory(key_version);
  }

  // The keys are saved either in the linux or the windows place
  // but they are imported from both places when you push import

  // copy the keys from the linux location if it exists
  // ../../../fractal_key_version_1
  filename = std::string{".."} + separator + std::string{".."} + separator +
             std::string{".."} + separator + key_version;
  if (fs::is_directory(filename)) {
    for (auto &p : fs::directory_iterator(filename)) {
      if (!fs::exists(key_version + separator + p.path().filename().string())) {
        fs::copy_file(p.path(),
                      key_version + separator + p.path().filename().string());
      }
    }
  }

  // copy the keys for the windows location if it exists
  // fractals_cuda/x64/Release/fractal_key_version_1
  filename = std::string{"fractals_cuda"} + separator + std::string{"x64"} +
             separator + std::string{"Release"} + separator + key_version;
  if (fs::is_directory(filename)) {
    for (auto &p : fs::directory_iterator(filename)) {
      filename = p.path().string();
      if (!fs::exists(key_version + separator + p.path().filename().string())) {
        fs::copy_file(p.path(),
                      key_version + separator + p.path().filename().string());
      }
    }
  }

  setGuiElementsFromModel(pgui, p_model);
}

void signalSaveKey(shared_ptr<FractalModel> p_model, shared_ptr<tgui::Gui> pgui,
                   std::string infname = "") {
  updateGuiElements(pgui, p_model);

  savf[frac_ix] = no_fractal;
  savf[frac_ix].version = FRACTAL_VERSION;
  savf[frac_ix].valid = 1;
  savf[frac_ix].current_fractal = p_model->current_fractal;
  savf[frac_ix].current_power = FRAC[p_model->current_fractal].current_power;
  savf[frac_ix].current_max_iters[0] =
      FRAC[p_model->current_fractal].current_max_iters[0];
  savf[frac_ix].current_max_iters[1] =
      FRAC[p_model->current_fractal].current_max_iters[1];
  savf[frac_ix].current_max_iters[2] =
      FRAC[p_model->current_fractal].current_max_iters[2];
  savf[frac_ix].current_zconst = FRAC[p_model->current_fractal].current_zconst;
  savf[frac_ix].current_escape_r =
      FRAC[p_model->current_fractal].current_escape_r;
  savf[frac_ix].RF = R;

  SavedFractal *p_savf = &savf[frac_ix];

  frac_ix++;
  if (frac_ix > max_saved) frac_ix = 0;

  std::string filename;
  std::ofstream key;

  // save the keys into either the windows location or the linux location
  // import will pick it up from both
  if (!fs::is_directory(key_version) || !fs::exists(key_version)) {
    fs::create_directory(key_version);
  }

  uint32_t crc =
      crc32c(0, reinterpret_cast<unsigned char *>(p_savf), sizeof(*p_savf));

  if (infname != "") {
    filename = infname + "." + key_version;
  } else {
    filename = key_version + separator + FRAC[p_model->current_fractal].name +
               "_" + to_string(crc) + "." + key_version;
  }
  key.open(filename.c_str(), ios::out | ios::binary);
  key.write(reinterpret_cast<char *>(p_savf), sizeof(*p_savf));
  key.close();

  cout << "saved: " << filename << " " << p_savf->current_fractal << " "
       << p_savf->current_power << endl;

  setGuiElementsFromModel(pgui, p_model);
}

void signalLoadNextSaved(shared_ptr<FractalModel> p_model,
                         shared_ptr<tgui::Gui> pgui) {
  updateGuiElements(pgui, p_model);
  p_model->reset_fractal_and_reference_frame();

  SavedFractal *p_savf;
  int tried = 0;
  int try_frac_ix = ++displayed_frac_ix;

  while (1) {
    p_savf = &savf[try_frac_ix];
    if (p_savf->valid != 0) break;
    if (tried > max_saved) return;
    try_frac_ix++;
    if (try_frac_ix > max_saved) try_frac_ix = 0;
    tried++;
  }
  displayed_frac_ix = try_frac_ix;

  p_model->current_fractal = p_savf->current_fractal;

  FRAC[p_model->current_fractal].current_power = p_savf->current_power;
  FRAC[p_model->current_fractal].current_max_iters[0] =
      p_savf->current_max_iters[0];
  FRAC[p_model->current_fractal].current_max_iters[1] =
      p_savf->current_max_iters[1];
  FRAC[p_model->current_fractal].current_max_iters[2] =
      p_savf->current_max_iters[2];
  FRAC[p_model->current_fractal].current_zconst = p_savf->current_zconst;
  FRAC[p_model->current_fractal].current_escape_r = p_savf->current_escape_r;
  R = p_savf->RF;

  setGuiElementsFromModel(pgui, p_model);
}

void LoadLast(shared_ptr<FractalModel> p_model, shared_ptr<tgui::Gui> pgui) {
  updateGuiElements(pgui, p_model);
  p_model->reset_fractal_and_reference_frame();

  SavedFractal *p_savf = &Last;

  p_model->current_fractal = p_savf->current_fractal;

  FRAC[p_model->current_fractal].current_power = p_savf->current_power;
  FRAC[p_model->current_fractal].current_max_iters[0] =
      p_savf->current_max_iters[0];
  FRAC[p_model->current_fractal].current_max_iters[1] =
      p_savf->current_max_iters[1];
  FRAC[p_model->current_fractal].current_max_iters[2] =
      p_savf->current_max_iters[2];
  FRAC[p_model->current_fractal].current_zconst = p_savf->current_zconst;
  FRAC[p_model->current_fractal].current_escape_r = p_savf->current_escape_r;
  R = p_savf->RF;

  setGuiElementsFromModel(pgui, p_model);
}

int LoadProvidedKey(shared_ptr<FractalModel> p_model,
                    shared_ptr<tgui::Gui> pgui, std::string keyname) {
  updateGuiElements(pgui, p_model);
  p_model->reset_fractal_and_reference_frame();
  SavedFractal savef = no_fractal;
  SavedFractal *p_savf = &savef;
  std::string filename;

  // Check if key contains key_version
  std::size_t found = keyname.find(key_version);
  if ((keyname == "no key") || (found == std::string::npos)) {
    cout << "wrong fractal version: " << keyname << std::flush << endl;
    return 1;
  }

  filename = keyname;

  std::ifstream key;
  key.open(filename.c_str(), ios::in | ios::binary);
  key.read(reinterpret_cast<char *>(p_savf), sizeof(*p_savf));
  key.close();

  cout << "LOADED PASSED IN KEY: " << keyname << " " << p_savf->current_fractal
       << " *****" << endl;

  p_model->current_fractal = p_savf->current_fractal;

  FRAC[p_model->current_fractal].current_power = p_savf->current_power;
  FRAC[p_model->current_fractal].current_max_iters[0] =
      p_savf->current_max_iters[0];
  FRAC[p_model->current_fractal].current_max_iters[1] =
      p_savf->current_max_iters[1];
  FRAC[p_model->current_fractal].current_max_iters[2] =
      p_savf->current_max_iters[2];
  FRAC[p_model->current_fractal].current_zconst = p_savf->current_zconst;
  FRAC[p_model->current_fractal].current_escape_r = p_savf->current_escape_r;
  R = p_savf->RF;

  // R.original_width/2 R.original_height/2 is a click on the center
  // Assume the user changed xstart and ystart
  cout << "Changes: " << R.original_width << " " << R.original_height << " "
       << R.requested_zoom << " " << R.xstart << " " << R.ystart << " " << endl;
  p_model->panFractal(R.original_width / 2.0, R.original_height / 2.0);

  p_model->zoomFractal(R.requested_zoom);

  setGuiElementsFromModel(pgui, p_model);
  return 0;
}

int key_count = 0;
void signalLoadNextKey(shared_ptr<FractalModel> p_model,
                       shared_ptr<tgui::Gui> pgui) {
  updateGuiElements(pgui, p_model);
  p_model->reset_fractal_and_reference_frame();

  SavedFractal savef = no_fractal;
  SavedFractal *p_savf = &savef;
  int ix = 0;

  key_count = 0;
  for (auto &p : fs::directory_iterator(key_version)) {
    std::cout << p.path() << '\n';
    key_count++;
  }
  if (key_count == 0) return;

  int try_frac_ix = ++last_loaded_key_ix;

  while (1) {
    if (try_frac_ix >= key_count) try_frac_ix = 0;
    break;
  }
  last_loaded_key_ix = try_frac_ix;

  std::string filename;
  for (auto &p : fs::directory_iterator(key_version)) {
    if (ix == try_frac_ix) {
      filename = p.path().string();
      break;
    }
    ix++;
  }

  std::ifstream key;
  key.open(filename.c_str(), ios::in | ios::binary);
  key.read(reinterpret_cast<char *>(p_savf), sizeof(*p_savf));
  key.close();

  cout << "loaded: " << filename << " " << p_savf->current_fractal << " "
       << p_savf->current_power << endl;

  p_model->current_fractal = p_savf->current_fractal;

  FRAC[p_model->current_fractal].current_power = p_savf->current_power;
  FRAC[p_model->current_fractal].current_max_iters[0] =
      p_savf->current_max_iters[0];
  FRAC[p_model->current_fractal].current_max_iters[1] =
      p_savf->current_max_iters[1];
  FRAC[p_model->current_fractal].current_max_iters[2] =
      p_savf->current_max_iters[2];
  FRAC[p_model->current_fractal].current_zconst = p_savf->current_zconst;
  FRAC[p_model->current_fractal].current_escape_r = p_savf->current_escape_r;
  R = p_savf->RF;

  setGuiElementsFromModel(pgui, p_model);
}

#ifdef _WINDOWS
std::string escape_dir = std::string{"..\\..\\..\\escape_image"};
#else
std::string escape_dir = std::string{"escape_image"};
#endif

int escape_count = 0;
int escape_ix = -1;
void signalLoadNextEscape(shared_ptr<FractalModel> p_model,
                          shared_ptr<tgui::Gui> pgui) {
  updateGuiElements(pgui, p_model);

  int ix = 0;

  escape_count = 0;
  for (auto &p : fs::directory_iterator(escape_dir)) {
    //std::cout << p.path() << '\n';
    escape_count++;
  }
  if (escape_count == 0) return;

  if (escape_ix == escape_count - 1) escape_ix = -1;

  std::string filename{""};
  for (auto &p : fs::directory_iterator(escape_dir)) {
    if ((escape_count == 1) || (ix > escape_ix)) {
      filename = p.path().string();
      escape_ix = ix;
      break;
    }
    ix++;
  }

  if (NSR.escape_texture.loadFromFile(filename.c_str())) {
    NSR.escape_image = NSR.escape_texture.copyToImage();
    sf::Vector2u escape_image_dims = NSR.escape_image.getSize();
    R.escape_image_w = escape_image_dims.x;
    R.escape_image_h = escape_image_dims.y;
    cout << "Loaded escape_image " << filename << " Dims: " << R.escape_image_w
         << " " << R.escape_image_h << endl;
    R.image_loaded = true;
  }

  setGuiElementsFromModel(pgui, p_model);
}

// The main GUI elements inside the view
void createGuiElements(shared_ptr<tgui::Gui> pgui,
                       shared_ptr<FractalModel> &p_model) {
  // Current and Menu Group
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
  menu->setPosition("parent.left", "parent.bottom - 300 - 30");
  menu->setSize(200.f, 22.f);
  menu->addMenu("Fractal");
  for (auto frac : FRAC) {
    menu->addMenuItem(frac.name);
  }
  menu->addMenu("Help");
  menu->addMenuItem("Use mouse wheel to zoom (some fractals)");
  menu->addMenuItem("Use right mouse button to recenter (some fractals)");
  menu->addMenuItem(
      "Hold and drag left mouse button to zoom to cropped area (some "
      "fractals)");
  menu->addMenuItem("Type h to hide/show gui");
  menu->addMenuItem("Type p to pause/resume fractal generation");
  menu->addMenuItem("Type c to turn cuda on/off");
  menu->addMenuItem("Type f for fullscreen on/off (buggy)");
  menu->addMenuItem("Type s to take a screenshot");
  menu->addMenuItem("Type z to undo last zoom/pan");
  menu->addMenuItem("Type n to load next coloring escape image");
  menu->addMenuItem("Type e to exit");

  // pgui->add(menu); //added at end so its always on top
  menu->connect("MenuItemClicked", signalFractalMenu, p_model, pgui);

  // Params Group
  current = tgui::Label::create();
  current->setPosition("parent.left + 50", "parent.bottom - 150 - 20");
  current->setTextSize(14);
  pgui->add(current, "power_label");

  auto editBox = tgui::EditBox::create();
  editBox->setSize(100, 20);
  editBox->setTextSize(14);
  editBox->setPosition("parent.left + 50", "parent.bottom - 150");
  editBox->setDefaultText("2");
  pgui->add(editBox, "power_box");
  editBox->connect("TextChanged", signalPower, p_model, pgui);

  current = tgui::Label::create();
  current->setPosition("parent.left + 50", "parent.bottom - 210");
  current->setTextSize(14);
  pgui->add(current, "miters_label");

  editBox = tgui::EditBox::create();
  editBox->setSize(100, 20);
  editBox->setTextSize(14);
  editBox->setPosition("parent.left + 50 + 120", "parent.bottom - 150");
  editBox->setDefaultText("");
  pgui->add(editBox, "max_iters_box2");
  editBox->connect("TextChanged", signalMIters, p_model, pgui, 2);

  editBox = tgui::EditBox::create();
  editBox->setSize(100, 20);
  editBox->setTextSize(14);
  editBox->setPosition("parent.left + 50 + 120", "parent.bottom - 180");
  editBox->setDefaultText("");
  pgui->add(editBox, "max_iters_box1");
  editBox->connect("TextChanged", signalMIters, p_model, pgui, 1);

  editBox = tgui::EditBox::create();
  editBox->setSize(100, 20);
  editBox->setTextSize(14);
  editBox->setPosition("parent.left + 50 + 120", "parent.bottom - 210");
  editBox->setDefaultText("");
  pgui->add(editBox, "max_iters_box0");
  editBox->connect("TextChanged", signalMIters, p_model, pgui, 0);

  auto cbox = tgui::CheckBox::create();
  cbox->setPosition("parent.left + 50 + 250", "parent.bottom - -210");
  cbox->setText("Random\nSample");
  cbox->setSize(30, 30);
  pgui->add(cbox, "RandomSample");
  cbox->connect("Checked", signalSamplingButton, p_model);
  cbox->connect("Unchecked", signalSamplingButton, p_model);
  cbox->setChecked(true);

  current = tgui::Label::create();
  current->setPosition("parent.left + 50", "parent.bottom - 100 - 20");
  current->setTextSize(14);
  pgui->add(current, "zconst_label");

  editBox = tgui::EditBox::create();
  editBox->setSize(100, 20);
  editBox->setTextSize(14);
  editBox->setPosition("parent.left + 50", "parent.bottom - 100");
  editBox->setDefaultText("0");
  pgui->add(editBox, "zconst_real_box");
  editBox->connect("TextChanged", signalZconstr, p_model, pgui);

  editBox = tgui::EditBox::create();
  editBox->setSize(100, 20);
  editBox->setTextSize(14);
  editBox->setPosition("parent.left + 50 + 120", "parent.bottom - 100");
  editBox->setDefaultText("0");
  pgui->add(editBox, "zconst_imag_box");
  editBox->connect("TextChanged", signalZconsti, p_model, pgui);

  current = tgui::Label::create();
  current->setPosition("parent.left + 50", "parent.bottom - 80");
  current->setTextSize(14);
  pgui->add(current, "escape_r_label");

  editBox = tgui::EditBox::create();
  editBox->setSize(100, 20);
  editBox->setTextSize(14);
  editBox->setPosition("parent.left + 50 + 120", "parent.bottom - 60");
  editBox->setDefaultText("0");
  pgui->add(editBox, "escape_r_box");
  editBox->connect("TextChanged", signal_escape_r, p_model, pgui);

  // Save Fractal Group

  auto button = tgui::Button::create();
  button->setPosition("parent.left + 400", "parent.bottom - 150");
  button->setText("Save Fractal");
  button->setSize(120, 30);
  pgui->add(button, "SaveFractal");
  button->connect("Pressed", signalSaveFractal, p_model, pgui);

  button = tgui::Button::create();
  button->setPosition("parent.left + 400", "parent.bottom - 100");
  button->setText("Load Next Saved");
  button->setSize(120, 30);
  pgui->add(button, "LoadNextSaved");
  button->connect("Pressed", signalLoadNextSaved, p_model, pgui);

  current = tgui::Label::create();
  current->setPosition("parent.left + 400", "parent.bottom - 50");
  current->setTextSize(14);
  pgui->add(current, "saved_fractal_label");

  button = tgui::Button::create();
  button->setPosition("parent.left + 600", "parent.bottom - 200");
  button->setText("Import Keys");
  button->setSize(120, 30);
  pgui->add(button, "ImportKeys");
  button->connect("Pressed", signalImportKeys, p_model, pgui);

  button = tgui::Button::create();
  button->setPosition("parent.left + 600", "parent.bottom - 150");
  button->setText("Save Key");
  button->setSize(120, 30);
  pgui->add(button, "SaveKey");
  button->connect("Pressed", signalSaveKey, p_model, pgui);

  button = tgui::Button::create();
  button->setPosition("parent.left + 600", "parent.bottom - 100");
  button->setText("Load Next Key");
  button->setSize(120, 30);
  pgui->add(button, "LoadNextKey");
  button->connect("Pressed", signalLoadNextKey, p_model, pgui);

  current = tgui::Label::create();
  current->setPosition("parent.left + 600", "parent.bottom - 50");
  current->setTextSize(14);
  pgui->add(current, "keys_label");

  // Palette Column
  auto lbox = tgui::ListBox::create();
  lbox->setPosition("parent.left + 800", "parent.bottom - 300");
  lbox->setSize(100.f, 290.f);
  for (auto e : NSR.color_names) {
    lbox->addItem(e);
  }

  pgui->add(lbox, "ColorBox");
  lbox->connect("ItemSelected", signalColorBox);

  lbox = tgui::ListBox::create();
  lbox->setPosition("parent.left + 900", "parent.bottom - 300");
  lbox->setSize(60.f, 130.f);
  for (auto e : NSR.color_cycle_size_names) {
    lbox->addItem(e);
  }

  pgui->add(lbox, "CycleBox");
  lbox->connect("ItemSelected", signalColorCycleBox);

  cbox = tgui::CheckBox::create();
  cbox->setPosition("parent.left + 900", "parent.bottom - 150");
  cbox->setText("Reflect");
  cbox->setSize(30, 30);
  pgui->add(cbox, "Reflect");
  cbox->connect("Checked", signalButton);
  cbox->connect("Unchecked", signalButton);

  lbox = tgui::ListBox::create();
  lbox->setPosition("parent.left + 900", "parent.bottom - 100");
  lbox->setSize(100.f, 100.f);
  lbox->addItem("MULTICYCLE");
  lbox->addItem("SMOOTH");
  lbox->addItem("USE_IMAGE");
  lbox->addItem("SHADOW_MAP");

  pgui->add(lbox, "CAlgoBox");
  lbox->connect("ItemSelected", signalCAlgoBox);

  // Interior Coloring Column
  lbox = tgui::ListBox::create();
  lbox->setPosition("parent.left + 1000", "parent.bottom - 300");
  lbox->setSize(100.f, 290.f);
  for (auto e : NSR.color_names) {
    lbox->addItem(e);
  }

  pgui->add(lbox, "IntColorBox");
  lbox->connect("ItemSelected", signalIntColorBox);

  lbox = tgui::ListBox::create();
  lbox->setPosition("parent.left + 1100", "parent.bottom - 300");
  lbox->setSize(60.f, 130.f);
  for (auto e : NSR.color_cycle_size_names) {
    lbox->addItem(e);
  }

  pgui->add(lbox, "IntCycleBox");
  lbox->connect("ItemSelected", signalIntColorCycleBox);

  cbox = tgui::CheckBox::create();
  cbox->setPosition("parent.left + 1100", "parent.bottom - 150");
  cbox->setText("Reflect");
  cbox->setSize(30, 30);
  pgui->add(cbox, "IntReflect");
  cbox->connect("Checked", signalIntButton);
  cbox->connect("Unchecked", signalIntButton);

  lbox = tgui::ListBox::create();
  lbox->setPosition("parent.left + 1100", "parent.bottom - 100");
  lbox->setSize(100.f, 100.f);
  lbox->addItem("SOLID");
  lbox->addItem("MULTICYCLE");
  lbox->addItem("USE_IMAGE");
  lbox->addItem("TRIG");
  lbox->addItem("TRIG2");
  lbox->addItem("DIST");
  lbox->addItem("DIST2");
  lbox->addItem("TEMP");

  pgui->add(lbox, "IntCAlgoBox");
  lbox->connect("ItemSelected", signalIntCAlgoBox);


  editBox = tgui::EditBox::create();
  editBox->setSize(100, 20);
  editBox->setTextSize(14);
  editBox->setPosition("parent.left + 1200", "parent.bottom - 300");
  editBox->setDefaultText("dec triple");
  pgui->add(editBox, "interior_color_adjust");
  editBox->connect("TextChanged", signalIntColorAdj, p_model, pgui);

  pgui->add(menu);  // to be on top
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
                             shared_ptr<FractalModel> &p_model) {
  // R.palette
  auto current = pgui->get<tgui::ListBox>("ColorBox");
  current->setSelectedItemByIndex(static_cast<int>(R.palette));
}

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
  if ((FRAC[p_model->current_fractal].cuda_mode == true) &&
      (p_model->cuda_detected == true))
    current->setText("Cuda Running");
  else
    current->setText("Cuda Off");

  std::string zoom_string;
  std::ostringstream out;
  out.precision(16);
  out << std::fixed << R.displayed_zoom;
  zoom_string = out.str();

  current = pgui->get<tgui::Label>("boundary_label");
  current->setText("Boundary: [" + to_string(R.xstart) + "->" +
                   to_string(R.xstart + (R.original_width) * R.xdelta) + "]/[" +
                   to_string(R.ystart) + "->" +
                   to_string(R.ystart + (R.original_height) * R.ydelta) + "]" +
                   " Zoom: " + zoom_string);

  current = pgui->get<tgui::Label>("stats_label");
  current->setText(
      "Stats: rejected/total : escaped/in -> " +
      to_string(p_model->stats[p_model->current_fractal].rejected) + "/" +
      to_string(p_model->stats[p_model->current_fractal].total) + " : " +
      to_string(p_model->stats[p_model->current_fractal].escaped_set) + "/" +
      to_string(p_model->stats[p_model->current_fractal].in_set) + " : " +
      to_string(100.0 * p_model->stats[p_model->current_fractal].rejected /
                (p_model->stats[p_model->current_fractal].total)) +
      "\%" + " : " +
      to_string(p_model->stats[p_model->current_fractal].escaped_set /
                (static_cast<double>(
                    p_model->stats[p_model->current_fractal].in_set +
                    p_model->stats[p_model->current_fractal].escaped_set))) +
      "\% total");

  // Params column
  current = pgui->get<tgui::Label>("power_label");
  current->setText(
      "Power: " +
      to_string(FRAC[p_model->current_fractal].current_power).substr(0, 4));

  current = pgui->get<tgui::Label>("miters_label");
  current->setText(
      "Max iterations: \n" +
      to_string(FRAC[p_model->current_fractal].current_max_iters[0]) + " " +
      to_string(FRAC[p_model->current_fractal].current_max_iters[1]) + " " +
      to_string(FRAC[p_model->current_fractal].current_max_iters[2]));

  current = pgui->get<tgui::Label>("zconst_label");
  current->setText(
      "z_const: " +
      to_string(FRAC[p_model->current_fractal].current_zconst.real()) + " + " +
      to_string(FRAC[p_model->current_fractal].current_zconst.imag()) + "*i");

  current = pgui->get<tgui::Label>("escape_r_label");
  current->setText("Escape R: " +
                   to_string(FRAC[p_model->current_fractal].current_escape_r));

  current = pgui->get<tgui::Label>("saved_fractal_label");
  current->setText("Fractal ix: " + to_string(displayed_frac_ix));

  current = pgui->get<tgui::Label>("keys_label");
  current->setText("Key ix: " + to_string(last_loaded_key_ix) + "/" +
                   to_string(key_count));
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

    if (R.requested_zoom > 25.0) {
      R.requested_zoom = 1.0;
    }
  }

  return R.requested_zoom;
}

// save a screenshot
void save_screenshot(sf::RenderWindow &window, string name, sf::View &modelview,
                     shared_ptr<FractalModel> p_model,
                     shared_ptr<tgui::Gui> pgui, bool display_gui,
                     std::string savename) {
  char buffer[80] = "no date";
  time_t rawtime;
  struct tm *timeinfop = nullptr;

  time(&rawtime);
#ifdef _WINDOWS
  struct tm timeinfo;
  localtime_s(&timeinfo, &rawtime);
  timeinfop = &timeinfo;
#else
  timeinfop = localtime(&rawtime);
#endif

  strftime(buffer, sizeof(buffer), "%d-%m-%Y_%H_%M_%S", timeinfop);
  std::string timestring(buffer);

  // window.clear();
  // window.setView(modelview);
  window.draw(*p_model);  // draw fractals in case the model is being hidden
  if (display_gui) pgui->draw();

  sf::Vector2u windowSize = window.getSize();
  sf::Texture texture;
  texture.create(windowSize.x, windowSize.y);
  // texture.create(IMAGE_WIDTH, IMAGE_HEIGHT);
  texture.update(window);
  sf::Image screenshot = texture.copyToImage();
  if (savename != "none")
    screenshot.saveToFile(savename);
  else {
#ifdef _WINDOWS
    screenshot.saveToFile(string{".."} + separator + string{".."} + separator +
                          string{".."} + separator + string{"screenshots"} + separator + name +
                          timestring +
                          ".png");
#else
    screenshot.saveToFile(string{"screenshots"} + separator + name + timestring +
                          ".png");
#endif
  }
};

int main(int argc, char **argv) {
  std::vector<std::string> argList;
  std::string savename{"no key"};
  std::string keyname{"no key"};
  update_and_draw = false;
  save_and_exit = false;
  hide = false;

  if (std::is_trivially_copyable<SavedFractal>::value == false) {
    cout << "SavedFractal not serializable\n";
    return -1;
  }

  if (argc > 3) {
    for (auto val : argList) {
      cout << val << " ";
    }
    cout << endl;

    argList = std::vector<std::string>(argv, argv + argc);
    // run one iteration using the supplied fractal_key and save the image to
    // savename and exit in python you can edit the fractal key parameters and
    // increment the savename and stitch together the images into a movie
    if (argList[1] == "save_and_exit") {
      keyname = argList[2];
      savename = argList[3];
      save_and_exit = true;

      if (argList[4] == "hide") hide = true;
    }
  }

  // Register signal and signal handler
  signal(SIGINT, signal_callback_handler);

  sf::Vector2u screenDimensions(IMAGE_WIDTH, IMAGE_HEIGHT);
  sf::RenderWindow window;
  window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y),
                "Fractals!", sf::Style::None);  // sf::Style::Fullscreen
  if (hide) window.setVisible(false);

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

  sf::View modelview;
  sf::Vector2u viewD(screenDimensions.x, screenDimensions.y);
  modelview.setSize(viewD.x, viewD.y);
  R.displayed_zoom = 1.0;
  R.requested_zoom = 1.0;
  R.current_height = screenDimensions.y;
  R.current_width = screenDimensions.x;
  R.original_height = screenDimensions.y;
  R.original_width = screenDimensions.x;

  R.color_cycle_size = 32;
  R.palette =
      tinycolormap::ColormapType::UF16;  // tinycolormap::ColormapType::Viridis;
                                         // tinycolormap::ColormapType::UF16
  R.color_algo = ColoringAlgo::MULTICYCLE;

  std::string escape_file1 =
      escape_dir + separator + std::string("escape_image.jpg");
  std::string escape_file2 =
      escape_dir + separator + std::string("escape_image.png");
  // R.color_algo = ColoringAlgo::USE_IMAGE;
  if ((NSR.escape_texture.loadFromFile(escape_file1.c_str())) ||
      (NSR.escape_texture.loadFromFile(escape_file1.c_str()))) {
    NSR.escape_image = NSR.escape_texture.copyToImage();
    sf::Vector2u escape_image_dims = NSR.escape_image.getSize();
    R.escape_image_w = escape_image_dims.x;
    R.escape_image_h = escape_image_dims.y;
    cout << "Loaded escape_image "
         << escape_file1.c_str() << " Dims: " << R.escape_image_w << " "
         << R.escape_image_h << endl;
    R.image_loaded = true;
  } else {
    cout << "missing escape_image.jpg[png] for fractal escape coloring" << endl;
    R.color_algo = ColoringAlgo::MULTICYCLE;
    R.image_loaded = false;
  }
  R.light_pos_r = 1;
  R.light_pos_i = 0;
  R.light_angle = 45;
  R.light_height = 1.5;

  modelview.setCenter(screenDimensions.x / 2.0, screenDimensions.y / 2.0);
  window.setView(modelview);

  // create the fractal model (i.e. Model)
  //    note: we are passing this shared ptr to signals and threads so they can
  //    change the model (MVC)
  auto p_model =
      make_shared<FractalModel>(screenDimensions.x, screenDimensions.y);

  // p_model->cudaTest();
  if (!save_and_exit) p_model->cudaPresent();

  // Create the worker threads:
  cout << "Machine supports " << thread::hardware_concurrency()
       << " simultaneous threads" << endl;

  unsigned int cmd_line_threads;
  if (argc > 1)
    cmd_line_threads = atoi(argv[1]);
  else
    cmd_line_threads = thread::hardware_concurrency() - 1;
  unsigned int num_threads = 1;
  if ((cmd_line_threads <= 0) ||
      (cmd_line_threads >= thread::hardware_concurrency() - 1))
    num_threads = thread::hardware_concurrency() - 1;
  else
    num_threads = cmd_line_threads;
  p_model->num_threads = num_threads;

  cout << "Using " << num_threads << " threads to speed up fractal rendering"
       << endl;
  thread threads[MAX_THREADS];
  std::promise<void> terminateThreadSignal[MAX_THREADS];

  // start up thread pool
  // thread:
  // input: thread id, p_model, mutex to report results, future object for
  // thread termination output: merges hits into model under a mutex
  for (unsigned int tix = 0; tix < num_threads; ++tix) {
    std::future<void> futureObj = terminateThreadSignal[tix].get_future();

    threads[tix] = thread(&FractalModel::fractal_thread, p_model, tix,
                          std::move(futureObj), &thread_asked_to_reset[0],
                          &thread_iteration[0]);
  }

  bool display_gui = true;
  // Create the gui and attach it to the window
  auto pgui = make_shared<tgui::Gui>(window);
#ifdef _WINDOWS
  tgui::Theme theme{"../../../themes/BabyBlue.txt"};
#else
  tgui::Theme theme{"themes/BabyBlue.txt"};
#endif
  tgui::Theme::setDefault(&theme);
  createGuiElements(pgui, p_model);
  updateGuiElements(pgui, p_model);

  // load the fractal key on startup
  if (save_and_exit) {
    if (LoadProvidedKey(p_model, pgui, keyname)) {
      // terminate threads in thread pool
      for (unsigned int tix = 0; tix < num_threads; ++tix) {
        terminateThreadSignal[tix].set_value();
        // to make it check for terminate
        for (unsigned int tix = 0; tix < num_threads; ++tix) {
          thread_asked_to_reset[tix] = true;
        }
        threads[tix].join();
      }
      exit(-1);
    }
  }
  update_and_draw = true;

  // Track attempted crops with mouse
  int crop_start_x = 0;
  int crop_start_y = 0;
  int crop_end_x = 0;
  int crop_end_y = 0;
  sf::RectangleShape selection(sf::Vector2f(0, 0));
  R.show_selection = false;

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
        } else if (event.key.code == sf::Keyboard::Z) {
          update_and_draw = false;
          for (unsigned int tix = 0; tix < num_threads; ++tix) {
            thread_asked_to_reset[tix] = true;
          }
          LoadLast(p_model, pgui);
          update_and_draw = true;
        } else if (event.key.code == sf::Keyboard::P) {
          if (update_and_draw == false) {
            update_and_draw = true;
          } else {
            update_and_draw = false;
          }
        } else if (event.key.code == sf::Keyboard::S) {
          save_screenshot(window, FRAC[p_model->current_fractal].name,
                          modelview, p_model, pgui, display_gui, "none");
        } else if (event.key.code == sf::Keyboard::E) {
          window.close();
          break;
        } else if (event.key.code == sf::Keyboard::N) {
          update_and_draw = false;
          for (unsigned int tix = 0; tix < num_threads; ++tix) {
            thread_asked_to_reset[tix] = true;
          }
          signalLoadNextEscape(p_model, pgui);
          update_and_draw = true;
        } else if (event.key.code == sf::Keyboard::C) {
          if (FRAC[p_model->current_fractal].cuda_mode == true)
            FRAC[p_model->current_fractal].cuda_mode = false;
          else
            FRAC[p_model->current_fractal].cuda_mode = true;
        } else if (event.key.code == sf::Keyboard::F) {
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
          SaveLast(p_model);
          double newzoom =
              get_new_zoom(modelview, event.mouseWheelScroll.delta);
          cout << "New zoom: " << newzoom << endl;
          p_model->zoomFractal(newzoom);
          for (unsigned int tix = 0; tix < num_threads; ++tix) {
            thread_asked_to_reset[tix] = true;
          }
        }
      }

      // record center for right mouse button and crop for left
      if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Right) {
          // Pan
          SaveLast(p_model);
          cout << "New center: ";
          cout << "x: " << event.mouseButton.x;
          cout << " y: " << event.mouseButton.y << endl;
          p_model->panFractal(event.mouseButton.x, event.mouseButton.y);
          for (unsigned int tix = 0; tix < num_threads; ++tix) {
            thread_asked_to_reset[tix] = true;
          }
        }

        if (event.mouseButton.button == sf::Mouse::Left) {
          // Crop start
          SaveLast(p_model);
          crop_start_x = event.mouseButton.x;
          crop_start_y = event.mouseButton.y;
        }
      }

      // Crop finish
      if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
          crop_end_x = event.mouseButton.x;
          crop_end_y = event.mouseButton.y;

          // Hopefully this filters out menu clicks
          if (abs(crop_end_x - crop_start_x) > 10) {
            cout << "maintaining aspect ratio" << endl;
            // We cant do this directly - we have to combine pan and zoom since
            // they preserve aspect ratio
            p_model->panFractal((crop_start_x + crop_end_x) / 2,
                                (crop_start_y + crop_end_y) / 2);
            // Now fake a new zoom -> update R.requested_zoom
            R.requested_zoom =
                R.requested_zoom *
                (abs(crop_end_x - crop_start_x) / R.original_width);
            p_model->zoomFractal(R.requested_zoom);
            R.show_selection = false;
            // tell threads to start drawing new stuff
            for (unsigned int tix = 0; tix < num_threads; ++tix) {
              thread_asked_to_reset[tix] = true;
            }
          }
          R.show_selection = false;
        }
      }

      // Draw selection while button not released
      if ((event.type == sf::Event::MouseMoved) &&
          true == sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        // Hopefully this filters out menu clicks
        if (abs(crop_end_x - crop_start_x) > 10) {
          selection.setSize(
              sf::Vector2f(abs(crop_start_x - event.mouseMove.x),
                           abs(crop_start_y - event.mouseMove.y)));
          selection.setFillColor(sf::Color::Transparent);
          selection.setPosition(crop_start_x, crop_start_y);

          // set a 5-pixel wide orange outline
          selection.setOutlineThickness(5);
          selection.setOutlineColor(sf::Color(250, 150, 100));
          R.show_selection = true;
        }
      }

      // Handle the control coming from the gui
      pgui->handleEvent(event);
    }
    ++frames;

    if (update_and_draw) {
      // Evolve the model independantly
      sf::Time elapsed = clock_e.restart();
      p_model->update(
          elapsed);  // rebuild the pixels from what threads did in background

      // Draw the GUI and the MODEL both of which are controlled by
      // Keyboard, mouse, and gui elements
      updateCurrentGuiElements(pgui, p_model, start.asSeconds(),
                               frames / start.asSeconds());

      start = clock_s.getElapsedTime();

      window.clear();
      window.setView(modelview);
      window.draw(*p_model);  // draw fractals in gui off mode to save cpu
      if (R.show_selection) window.draw(selection);  // draw mouse selection
      pgui->draw();                                  // Draw all GUI widgets
      window.display();  // if you always do this it will cause screen jitter,
                         // but if you alt-tabe you will get white screen
    }

    bool done = true;
    for (unsigned int tix = 0; tix < num_threads; ++tix) {
      if (thread_iteration[tix] < 2) {
        done = false;
        break;
      }
    }

    if ((save_and_exit) && (done)) {
      // Evolve the model independantly
      sf::Time elapsed = clock_e.restart();
      p_model->update(
          elapsed);  // rebuild the pixels from what threads did in background

      cout << "saving " << savename << endl;
      // save screenshot
      save_screenshot(window, FRAC[p_model->current_fractal].name, modelview,
                      p_model, pgui, false, savename);
      signalSaveKey(p_model, pgui, "changed_key");
      window.close();
      break;
    }
  }

  // terminate threads in thread pool
  for (unsigned int tix = 0; tix < num_threads; ++tix) {
    terminateThreadSignal[tix].set_value();
    // to make it check for terminate
    for (unsigned int tix = 0; tix < num_threads; ++tix) {
      thread_asked_to_reset[tix] = true;
    }
    threads[tix].join();
  }
  // cout << "joined threads" << endl;

  return 0;
}
