#include <math.h>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <cmath>
#include <ctime>
#include <iostream>
#include <random>
#include <string>
#include <utility>

// TODO
// Add more ODEs
// Add particle size based on distance from observer
// Freeze ODE update and induce slow rotation of all points (not just latest)
// Offset one attractor, merge it with another, in the plane boundary give
// random
// probability of following one ODE for next cycles
// OpenGL 3D rotating colored cube
// Compile on Windows C++
// Enhance to use Runge-Kutta4

using namespace std;

// The overall goal is to create a generic visualizable model of 10 or so
// different 3-d and 2-d differential equations producing
// strange attractor behavior.
// We want to make it fun and easy to see the chaotic behavior

// Model: The model will store time sequences of data points
//  in the perspective reference frame.
//  At each dT we will convert back to the equation reference frame,
//  apply the DE, and then convert back to the user's desired
//  reference frame and store the point
//  Which DE we use will be a user settable strategy
// The Model will depend on the reference frame and the DE classes
//  (i.e. it will contain references to those)
// In addition the model will support visualization options like colors
//  and particle counts and trail length and perspective size scaling (distance
//  from viewpoint) which will be valid not matter which DE we are using

// Visual Elements (View):
//   Model Core Elements
//   Reference frame axes
//   Spinning Logo (textured 3D cube)
//   Current Model Parametarization Display
//   Gui Graphical Controls (Model Params, Reference frame angle and zoom etc)
//   Zoom via mouse
//

// Controls:
//  Gui Graphical Controls:
//  Zoom via mouse
//  Hide controls via keyboard shortcut
//  Keyboard shortcut to take screenshots

class ReferenceFrame {
 public:
  // Euler angles
  float theta[3];

  // View
  //
  float xcenter;
  float ycenter;

  ReferenceFrame(float _thetax, float _thetay, float _thetaz)
      : theta{_thetax, _thetay, _thetaz} {};

  // Cant use vectors in this API because sfml lib does 2-D drawing
  void rotatePoint(float x, float y, float z, float &xnew, float &ynew,
                   float &znew, bool rotate_back) {
    // Now make xnew,ynew,znew rotate for the given euler angles
    float xin = x;
    float yin = y;
    float zin = z;

    float xout = x;
    float yout = y;
    float zout = z;

    if (rotate_back == false)  // Rotate to the View Reference Frame
    {
      if (theta[0] != 0) {
        float xangle = (M_PI * theta[0]) / 180.0;
        float cosx = cos(xangle);
        float sinx = sin(xangle);

        xout = xin;
        yout = yin * cosx - zin * sinx;
        zout = yin * sinx + zin * cosx;
        xin = xout;
        yin = yout;
        zin = zout;
      }

      if (theta[1] != 0) {
        float yangle = (M_PI * theta[1]) / 180.0;
        float cosy = cos(yangle);
        float siny = sin(yangle);

        xout = xin * cosy + zin * siny;
        yout = yin;
        zout = zin * cosy - xin * siny;
        xin = xout;
        yin = yout;
        zin = zout;
      }

      if (theta[2] != 0) {
        float zangle = (M_PI * theta[2]) / 180.0;
        float cosz = cos(zangle);
        float sinz = sin(zangle);

        xout = xin * cosz - yin * sinz;
        yout = xin * sinz + yin * cosz;
        zout = zin;
        xin = xout;
        yin = yout;
        zin = zout;
      }
    } else  // rotate back to Model Reference Frame
    {
      if (theta[2] != 0) {
        float zangle = -(M_PI * theta[2]) / 180.0;
        float cosz = cos(zangle);
        float sinz = sin(zangle);

        xout = xin * cosz - yin * sinz;
        yout = xin * sinz + yin * cosz;
        zout = zin;
        xin = xout;
        yin = yout;
        zin = zout;
      }

      if (theta[1] != 0) {
        float yangle = -(M_PI * theta[1]) / 180.0;
        float cosy = cos(yangle);
        float siny = sin(yangle);

        xout = xin * cosy + zin * siny;
        yout = yin;
        zout = zin * cosy - xin * siny;
        xin = xout;
        yin = yout;
        zin = zout;
      }

      if (theta[0] != 0) {
        float xangle = -(M_PI * theta[0]) / 180.0;
        float cosx = cos(xangle);
        float sinx = sin(xangle);

        xout = xin;
        yout = yin * cosx - zin * sinx;
        zout = yin * sinx + zin * cosx;
        xin = xout;
        yin = yout;
        zin = zout;
      }
    }

    // Doesnt work:
    //
    // float xangle = (M_PI*thetax)/180.0;
    // if (back == true) xangle = -xangle;
    // float cosx = cos(xangle);
    // float sinx = sin(xangle);

    // float yangle = (M_PI*thetay)/180.0;
    // if (back == true) yangle = -yangle;
    // float cosy = cos(yangle);
    // float siny = sin(yangle);

    // float zangle = (M_PI*thetaz)/180.0;
    // if (back == true) zangle = -zangle;
    // float cosz = cos(zangle);
    // float sinz = sin(zangle);

    // xout = xin*(cosy*cosz) + yin*(sinx*siny*cosz - cosx*sinz)
    // +zin*(cosx*siny*sinz - sinx*cosz); yout = xin*(cosy*sinz) +
    // yin*(sinx*siny*sinz + cosx*cosz) + zin*(cosx*siny*sinz - sinx*cosz); zout
    // = xin*(-siny) + yin*(sinx*cosy) + zin*(cosx*cosy);

    xnew = xout;
    ynew = yout;
    znew = zout;
  }

  void testRotation() {
    float xnew, ynew, znew;
    cout << "ThetaXYZ: " << theta[0] << "," << theta[1] << "," << theta[2]
         << endl;
    cout << "1,0,0:" << endl;
    rotatePoint(1.0, 0.0, 0.0, xnew, ynew, znew, true);
    cout << xnew << " " << ynew << " " << znew << " " << endl;
    rotatePoint(xnew, ynew, znew, xnew, ynew, znew, false);
    cout << xnew << " " << ynew << " " << znew << " " << endl;

    cout << "0,1,0:" << endl;
    rotatePoint(0.0, 1.0, 0.0, xnew, ynew, znew, true);
    cout << xnew << " " << ynew << " " << znew << " " << endl;
    rotatePoint(xnew, ynew, znew, xnew, ynew, znew, false);
    cout << xnew << " " << ynew << " " << znew << " " << endl;

    cout << "0,0,1:" << endl;
    rotatePoint(0.0, 0.0, 1.0, xnew, ynew, znew, true);
    cout << xnew << " " << ynew << " " << znew << " " << endl;
    rotatePoint(xnew, ynew, znew, xnew, ynew, znew, false);
    cout << xnew << " " << ynew << " " << znew << " " << endl;
  }

};  // ReferenceFrame

ReferenceFrame R(0, 0, 0);

struct RGBColorMap {
 public:
  string name;
  sf::Color start;
  sf::Color decrement1;
  sf::Color decrement2;
};

// i 0->255
sf::Color get_color_for_map(RGBColorMap map, int i) {
  i = i % 256;

  int third = 256 / 3;
  sf::Color answer(0, 0, 0);

  // color multiplication is actually (.r * .r )/256  not % 256

  if (i < (third)) {
    sf::Color mult =
        sf::Color(map.decrement1.r * 3 * i, map.decrement1.g * 3 * i,
                  map.decrement1.b * 3 * i, 0);
    // cout << "mult:"<< " " << static_cast<int>(mult.r) << " " <<
    // static_cast<int>(mult.g) << " " << static_cast<int>(mult.b) << endl;
    answer = map.start - mult;
  } else if (i < (2 * third)) {
    sf::Color mult1 =
        sf::Color(map.decrement1.r * 3 * third, map.decrement1.g * 3 * third,
                  map.decrement1.b * 3 * third, 0);
    sf::Color mult2 = sf::Color(map.decrement2.r * 3 * (i - third),
                                map.decrement2.g * 3 * (i - third),
                                map.decrement2.b * 3 * (i - third), 0);
    answer = map.start - mult1 - mult2;
  } else {
    sf::Color mult1 =
        sf::Color(map.decrement1.r * 3 * third, map.decrement1.g * 3 * third,
                  map.decrement1.b * 3 * third, 0);
    sf::Color mult2 = sf::Color(map.decrement2.r * 3 * (third),
                                map.decrement2.g * 3 * (third),
                                map.decrement2.b * 3 * (third), 0);
    sf::Color mult3 = sf::Color(map.decrement1.r * 3 * (i - 2 * third),
                                map.decrement1.g * 3 * (i - 2 * third),
                                map.decrement1.b * 3 * (i - 2 * third), 0);
    answer = map.start - mult1 - mult2 + mult3;
  }

  // cout << "i: " << i << endl;
  // cout << "answer:"<< " " << static_cast<int>(answer.r) << " " <<
  // static_cast<int>(answer.g) << " " << static_cast<int>(answer.b) << endl;
  answer = sf::Color(answer.r % 160, answer.g % 160, answer.b % 160);  // dull
  return answer;
}

vector<RGBColorMap> RGBMap = {{
                                  string("GreenBlue"),
                                  sf::Color(0, 255, 255),
                                  sf::Color(0, 0, 1),
                                  sf::Color(0, 1, 0),
                              },
                              {
                                  string("RedGreen"),
                                  sf::Color(255, 255, 0),
                                  sf::Color(0, 1, 0),
                                  sf::Color(1, 0, 0),
                              },
                              {
                                  string("RedBlue"),
                                  sf::Color(255, 0, 255),
                                  sf::Color(0, 0, 1),
                                  sf::Color(1, 0, 0),
                              }};
int current_color_map_ix = 0;

//currently not used - a way to cycle through all the combinations
void get_next_hue(sf::Uint8 &r, sf::Uint8 &g, sf::Uint8 &b) {
  static int state = 0;
  static int max_delay = 64;
  static int delay_transition = max_delay;

  // play some games with color space
  if (delay_transition >= 0) {
    delay_transition--;
    if (delay_transition < 0) {
      delay_transition = max_delay;
    } else
      return;
  }

  if (state == 0) {
    g++;
    if (g == 255) state = 1;
  }
  if (state == 1) {
    r--;
    if (r == 0) state = 2;
  }
  if (state == 2) {
    b++;
    if (b == 255) state = 3;
  }
  if (state == 3) {
    g--;
    if (g == 0) state = 4;
  }
  if (state == 4) {
    r++;
    if (r == 255) state = 5;
  }
  if (state == 5) {
    b--;
    if (b == 0) state = 0;
  }
}


//Differential equation sub-model
struct StrangeAttractorDE {
  float current_dt;
  float dt;  // dT for PDE
  // Data Structures that hold PDE specific things
  string name;
  vector<string> PNames;
  vector<float> p;                    // slider values
  vector<pair<float, float>> MinMax;  // slider min max
  vector<vector<float>> Examples;  // Known Interesting sample parameter values
  pair<float, float> RandMinMax;   // min max starting values for randomized
                                  // particles for x,y,z Model Reference Frame
  float particle_size;
  float current_particle_size;
  pair<float, float> ViewCenter;  // xy
  pair<float, float> ViewSize;    // xy
};

typedef void (*applyDE_type)(float x, float y, float z, float &xnew,
                             float &ynew, float &znew, ReferenceFrame &R,
                             StrangeAttractorDE &DE);

vector<StrangeAttractorDE> DE = {
    {
        0.003,
        0.003,                                             // dT
        string("Lorenz"),                                  // name
        {string("sigma"), string("beta"), string("rho")},  // parameter names
        {10.0, 8.0 / 3.0, 28.0},                           // parameter values
        {{1, 100}, {0, 10}, {1, 100}},                     // Min/Max
        {{10.0, 8.0 / 3.0, 28.0},
         {12.69, 0.13, 53.03},
         {95.03, 0.19, 82.7}},  // Examples
        {-1, 1},                // particle starts Model View
        0.33,                   // particle_size
        0.33,
        {0, 0},     // View Center
        {140, 200}  // View Pixel Size
    },
    {
        0.001,
        0.001,                                    // dT
        string("Chen-Lee"),                       // name
        {string("a"), string("b"), string("c")},  // parameter names
        {5.0, -10.0, -0.38},                      // parameter values
        {{-100, 100}, {-100, 100}, {-100, 100}},  // Min/Max
        {{5.0, -10.0, -0.38}},                    // Examples
        {-1, 1},                                  // particle starts Model View
        0.33,                                     // particle_size
        0.33,
        {0, 0},     // View Center
        {140, 200}  // View Pixel Size
    },
    {
        0.01,
        0.01,                                     // dT
        string("Rossler"),                        // name
        {string("a"), string("b"), string("c")},  // parameter names
        {0.2, 0.2, 5.7},                          // parameter values
        {{-100, 100}, {-100, 100}, {-100, 100}},  // Min/Max
        {{0.2, 0.2, 5.7}, {0.1, 0.1, 14.0}},      // Examples
        {-1, 1},                                  // particle starts Model View
        0.33,                                     // particle_size
        0.33,
        {0, 0},     // View Center
        {140, 200}  // View Pixel Size
    },
    {
        0.01,
        0.01,              // dT
        string("Aizawa"),  // name
        {string("a"), string("b"), string("c"), string("d"), string("e"),
         string("f")},                     // parameter names
        {0.95, 0.7, 0.6, 3.5, 0.25, 0.1},  // parameter values
        {{-100, 100},
         {-100, 100},
         {-100, 100},
         {-100, 100},
         {-100, 100},
         {-100, 100}},                       // Min/Max
        {{0.95, 0.7, 0.6, 3.5, 0.25, 0.1}},  // Examples
        {-1, 1},                             // particle starts Model View
        0.03,                                // particle_size
        0.03,
        {0, 0},     // View Center
        {140, 200}  // View Pixel Size
    },
    {
        0.0001,
        0.0001,                          // dT
        string("Three-Scroll-Unified"),  // name
        {string("a"), string("b"), string("c"), string("d"), string("e"),
         string("f")},                               // parameter names
        {40.0, 55.0, 11.0 / 6.0, 0.16, 0.65, 20.0},  // parameter values
        {{-100, 100},
         {-100, 100},
         {-100, 100},
         {-100, 100},
         {-100, 100},
         {-100, 100}},                                 // Min/Max
        {{40.0, 55.0, 11.0 / 6.0, 0.16, 0.65, 20.0}},  // Examples
        {-1, 1},  // particle starts Model View
        2.03,     // particle_size
        2.03,
        {0, 0},     // View Center
        {140, 200}  // View Pixel Size
    },
    {
        0.01,
        0.01,                               // dT
        string("Thomas"),                   // name
        {string("b")},                      // parameter names
        {0.208186},                         // parameter values
        {{-2, 2}},                          // Min/Max
        {{0.208186}, {0.1998}, {0.32899}},  // Examples
        {-1, 1},                            // particle starts Model View
        .03,                                // particle_size
        .03,
        {0, 0},     // View Center
        {140, 200}  // View Pixel Size
    },
    {
        0.001,
        0.001,             // dT
        string("Dadras"),  // name
        {
            string("a"),
            string("b"),
            string("c"),
            string("d"),
            string("e"),
        },                          // parameter names
        {3.0, 2.7, 1.7, 2.0, 9.0},  // parameter values
        {
            {-50, 50},
        },                            // Min/Max
        {{3.0, 2.7, 1.7, 2.0, 9.0}},  // Examples
        {-1, 1},                      // particle starts Model View
        .03,                          // particle_size
        .03,
        {0, 0},     // View Center
        {140, 200}  // View Pixel Size
    }};

void apply_lorenz_de(float x, float y, float z, float &xnew, float &ynew,
                     float &znew, ReferenceFrame &R, StrangeAttractorDE &de) {
  // rotate to Model reference froma from View reference frame
  float xin, yin, zin;
  R.rotatePoint(x, y, z, xin, yin, zin, true);

  float dt = de.current_dt;
  // cout << de.p[0] << " "  <<de.p[1] << " " <<de.p[2] << " " << R.theta[0] <<
  // R.theta[1] << R.theta[2] << endl;
  float sigma{de.p[0]}, beta{de.p[1]}, rho{de.p[2]};
  float xout = xin + dt * sigma * (yin - xin);
  float yout = yin + dt * (xin * (rho - zin) - yin);
  float zout = zin + dt * (xin * yin - beta * zin);

  // rotate back to View Reference frame
  R.rotatePoint(xout, yout, zout, xnew, ynew, znew, false);
};

void apply_chen_lee_de(float x, float y, float z, float &xnew, float &ynew,
                       float &znew, ReferenceFrame &R, StrangeAttractorDE &de) {
  // rotate to Model reference froma from View reference frame
  float xin, yin, zin;
  R.rotatePoint(x, y, z, xin, yin, zin, true);

  float dt = de.current_dt;
  float a{de.p[0]}, b{de.p[1]}, c{de.p[2]};
  float xout = xin + dt * (a * xin - yin * zin);
  float yout = yin + dt * (b * yin + xin * zin);
  float zout = zin + dt * (c * zin + (xin * yin / 3.0));

  // rotate back to View Reference frame
  R.rotatePoint(xout, yout, zout, xnew, ynew, znew, false);
};

void apply_rossler_de(float x, float y, float z, float &xnew, float &ynew,
                      float &znew, ReferenceFrame &R, StrangeAttractorDE &de) {
  // rotate to Model reference froma from View reference frame
  float xin, yin, zin;
  R.rotatePoint(x, y, z, xin, yin, zin, true);

  float dt = de.current_dt;
  float a{de.p[0]}, b{de.p[1]}, c{de.p[2]};
  float xout = xin - dt * (yin + zin);
  float yout = yin + dt * (a * yin + xin);
  float zout = zin + dt * (b + zin * (xin - c));

  // rotate back to View Reference frame
  R.rotatePoint(xout, yout, zout, xnew, ynew, znew, false);
};

void apply_aizawa_de(float x, float y, float z, float &xnew, float &ynew,
                     float &znew, ReferenceFrame &R, StrangeAttractorDE &de) {
  // rotate to Model reference froma from View reference frame
  float xin, yin, zin;
  R.rotatePoint(x, y, z, xin, yin, zin, true);

  float dt = de.current_dt;
  float a{de.p[0]}, b{de.p[1]}, c{de.p[2]}, d{de.p[3]}, e{de.p[4]}, f{de.p[5]};
  float xout = xin + dt * ((zin - b) * xin - d * yin);
  float yout = yin + dt * (d * xin + (zin - b) * yin);
  float zout = zin + dt * (c + a * zin - (zin * zin * zin / 3) -
                           (xin * xin + yin * yin) * (1 + e * zin) +
                           f * (zin * xin * xin * xin));

  // rotate back to View Reference frame
  R.rotatePoint(xout, yout, zout, xnew, ynew, znew, false);
};

void apply_three_scroll_de(float x, float y, float z, float &xnew, float &ynew,
                           float &znew, ReferenceFrame &R,
                           StrangeAttractorDE &de) {
  // rotate to Model reference froma from View reference frame
  float xin, yin, zin;
  R.rotatePoint(x, y, z, xin, yin, zin, true);

  float dt = de.current_dt;
  float a{de.p[0]}, b{de.p[1]}, c{de.p[2]}, d{de.p[3]}, e{de.p[4]}, f{de.p[5]};
  float xout = xin + dt * (a * (yin - xin) + d * xin * zin);
  float yout = yin + dt * (b * xin - xin * zin + f * yin);
  float zout = zin + dt * (c * zin + xin * yin - e * xin * xin);

  // rotate back to View Reference frame
  R.rotatePoint(xout, yout, zout, xnew, ynew, znew, false);
};

void apply_thomas_de(float x, float y, float z, float &xnew, float &ynew,
                     float &znew, ReferenceFrame &R, StrangeAttractorDE &de) {
  // rotate to Model reference froma from View reference frame
  float xin, yin, zin;
  R.rotatePoint(x, y, z, xin, yin, zin, true);

  float dt = de.current_dt;
  float b{de.p[0]};
  float xout = xin + dt * (sin(yin) - b * xin);
  float yout = yin + dt * (sin(zin) - b * yin);
  float zout = zin + dt * (sin(xin) - b * zin);

  // rotate back to View Reference frame
  R.rotatePoint(xout, yout, zout, xnew, ynew, znew, false);
};

void apply_dadras_de(float x, float y, float z, float &xnew, float &ynew,
                     float &znew, ReferenceFrame &R, StrangeAttractorDE &de) {
  // rotate to Model reference froma from View reference frame
  float xin, yin, zin;
  R.rotatePoint(x, y, z, xin, yin, zin, true);

  float dt = de.current_dt;
  float a{de.p[0]}, b{de.p[1]}, c{de.p[2]}, d{de.p[3]}, e{de.p[4]};
  float xout = xin + dt * (yin - a * xin + b * yin * zin);
  float yout = yin + dt * (c * yin - xin * zin + zin);
  float zout = zin + dt * (d * xin * yin - e * zin);

  // rotate back to View Reference frame
  R.rotatePoint(xout, yout, zout, xnew, ynew, znew, false);
};

vector<applyDE_type> applyDe = {
    apply_lorenz_de,       apply_chen_lee_de, apply_rossler_de, apply_aizawa_de,
    apply_three_scroll_de, apply_thomas_de,   apply_dadras_de};


// Strange Attractor System of ordinary differential equations
//   This is the overall model that uses the differential equation submodel plug in
//   and also relies on reference frame
class DeStrangeSystem : public sf::Drawable, public sf::Transformable {
 public:
  DeStrangeSystem(unsigned int _particle_count, unsigned int _trail_length,
                  int current_de = 0)
      : particle_count(_particle_count),
        trail_length(_trail_length)

  {
    current_de = 0;
    cur_trail_ix = 0;
    setParticles(particle_count);
    setTrail(trail_length);

    // Create textured moving spheres for fun (move these out later to a different model)
    m_family.resize(3, sf::CircleShape(1.5 * DE[current_de].particle_size));
    m_texture.resize(3);
    for (size_t i = 0; i < m_family.size(); ++i)
      m_family[i].setFillColor(sf::Color::White);

    if (!m_texture[0].loadFromFile("moorcat_texture.jpg")) {
      cout << "missing cat" << endl;
    }

    if (!m_texture[1].loadFromFile("stella_texture.jpg")) {
      cout << "missing dog" << endl;
    }

    if (!m_texture[2].loadFromFile("nico_texture.jpg")) {
      cout << "missing son" << endl;
    }

    // p_texture->setSmooth(true);
    for (size_t i = 0; i < m_family.size(); ++i)
      m_family[i].setTexture(&m_texture[i]);

    // Create the particles that represent the 3 axes
    uaxes.resize(3);
    axes.resize(3);

    for (std::size_t a_ix = 0; a_ix < uaxes.size(); ++a_ix) {
      uaxes[a_ix].resize(100);
      axes[a_ix].resize(100);
    }

    for (size_t p = 0; p < uaxes[0].size(); ++p) {
      uaxes[0][p] = sf::Vector3f(p, 0, 0);
    }

    for (size_t p = 0; p < uaxes[0].size(); ++p) {
      uaxes[1][p] = sf::Vector3f(0, p, 0);
    }

    for (size_t p = 0; p < uaxes[0].size(); ++p) {
      uaxes[2][p] = sf::Vector3f(0, 0, p);
    }

    // rotate the axes to match the user requested view, like Zul'jin would do
    rotateAxes();
  }

  void rotateAxes() {
    // create axes from uaxes
    for (std::size_t a_ix = 0; a_ix < uaxes.size(); ++a_ix) {
      sf::Color color = sf::Color::Red;
      if (a_ix == 1) color = sf::Color::Green;
      if (a_ix == 2) color = sf::Color::Blue;

      for (size_t p = 0; p < uaxes[a_ix].size(); ++p) {
        float xnew, ynew, znew;
        R.rotatePoint(uaxes[a_ix][p].x, uaxes[a_ix][p].y, uaxes[a_ix][p].z,
                      xnew, ynew, znew, false);
        axes[a_ix][p].position = sf::Vector2f(xnew, ynew);
        axes[a_ix][p].color = color;
      }
    }
  }

  void resetDefaults() {
    // resize particles
    // set pde params
    // 10.0,8.0/3.0,28.0
    disable_update.lock();

    DE[current_de].p = DE[current_de].Examples[0];
    DE[current_de].current_dt = DE[current_de].dt;
    DE[current_de].current_particle_size = DE[current_de].particle_size;

    cur_trail_ix = 0;

    // need to wipe out all particles and trails first
    m_circles.resize(0);
    m_circles_z.resize(0);
    m_particle_trail.resize(0);
    m_particle_trail_z.resize(0);

    setParticles(100);
    setTrail(5000);

    disable_update.unlock();
  }

  std::size_t getParticles(void) { return m_circles.size(); }

  void setParticles(int count) {
    std::size_t start_p_ix = 0;
    bool new_particles = false;
    if (count - m_circles.size() > 0) {
      new_particles = true;
      start_p_ix = m_circles.size();
    }

    particle_count = count;
    m_circles.resize(count,
                     sf::CircleShape(DE[current_de].current_particle_size));
    m_circles_z.resize(count, 0.0f);

    m_particle_trail.resize(particle_count);
    m_particle_trail_z.resize(particle_count);

    if (false == new_particles) return;

    std::random_device rd;
    std::default_random_engine e2(rd());
    std::uniform_real_distribution<> dist(DE[current_de].RandMinMax.first,
                                          DE[current_de].RandMinMax.second);

    for (std::size_t i = start_p_ix; i < m_circles.size(); ++i) {
      float xnew, ynew, znew;
      float xu = dist(e2);
      float yu = dist(e2);
      float zu = dist(e2);
      // cout << xu << yu << zu << endl;
      R.rotatePoint(xu, yu, zu, xnew, ynew, znew, false);
      m_circles[i].setPosition(xnew, ynew);
      // m_circles[i].setFillColor(sf::Color(0, 255 - i * 20, 120 + i * 20));
      m_circles[i].setFillColor(
          get_color_for_map(RGBMap[current_color_map_ix], i));
      m_circles_z[i] = znew;
    }  // new particles

    for (std::size_t p_ix = start_p_ix; p_ix < m_circles.size(); ++p_ix) {
      m_particle_trail[p_ix].resize(trail_length);
      m_particle_trail_z[p_ix].resize(trail_length);

      sf::Vector2f cur_pos = m_circles[p_ix].getPosition();
      sf::Color cur_color = m_circles[p_ix].getFillColor();
      for (std::size_t t_ix = 0; t_ix < getTrail(); ++t_ix) {
        m_particle_trail[p_ix][t_ix].position = cur_pos;
        m_particle_trail[p_ix][t_ix].color = cur_color;
        m_particle_trail_z[p_ix][t_ix] = m_circles_z[p_ix];
      }  // all trails of new particles
    }    // new particles
  }

  // How many particles are out of the simulation
  int getEscaped(void) {
    int escaped = 0;
    for (std::size_t p_ix = 0; p_ix < m_circles.size(); ++p_ix) {
      if (isnan(m_particle_trail[p_ix][0].position.x)) escaped++;
    }
    return escaped;
  }

  std::size_t getTrail(void) { return m_particle_trail[0].getVertexCount(); }

  void setTrail(int count) {
    std::size_t start_t_ix = 0;
    bool new_trail = false;
    cur_trail_ix = 0;  // maybe some atomicity issue if we add threads
    if (count - m_particle_trail[0].getVertexCount() > 0) {
      new_trail = true;
      start_t_ix = m_particle_trail[0].getVertexCount();
    }
    trail_length = count;

    for (std::size_t p_ix = 0; p_ix < m_circles.size(); ++p_ix) {
      m_particle_trail[p_ix].resize(count);
      m_particle_trail_z[p_ix].resize(count);
    }

    if (false == new_trail) return;

    for (std::size_t p_ix = 0; p_ix < m_circles.size(); ++p_ix) {
      sf::Vector2f cur_pos = m_circles[p_ix].getPosition();
      sf::Color cur_color = m_circles[p_ix].getFillColor();
      for (int t_ix = start_t_ix; t_ix < count; ++t_ix) {
        m_particle_trail[p_ix][t_ix].position = cur_pos;
        m_particle_trail[p_ix][t_ix].color = cur_color;
        m_particle_trail_z[p_ix][t_ix] = m_circles_z[p_ix];
      }
    }
    cur_trail_ix = 0;
  }

  void update(sf::Time elapsed) {
    // a little bit of protection since we are changing array size in callback
    // signal context
    disable_update.lock();
    // change the position of the circles and add a new element to the trail
    ++cur_trail_ix;

    if (cur_trail_ix >= static_cast<int>(getTrail())) {
      cur_trail_ix = 0;
    }

    for (std::size_t p_ix = 0; p_ix < m_circles.size(); ++p_ix) {
      sf::Vector2f cur_pos = m_circles[p_ix].getPosition();
      float xnew;
      float ynew;
      float znew;
      applyDe[current_de](cur_pos.x, cur_pos.y, m_circles_z[p_ix], xnew, ynew,
                          znew, R, DE[current_de]);
      m_circles[p_ix].setPosition(xnew, ynew);
      m_circles_z[p_ix] = znew;

      cur_pos = m_circles[p_ix].getPosition();
      sf::Color cur_color = m_circles[p_ix].getFillColor();
      float cur_z = m_circles_z[p_ix];

      m_particle_trail[p_ix][cur_trail_ix].position = cur_pos;
      m_particle_trail[p_ix][cur_trail_ix].color = cur_color;
      m_particle_trail_z[p_ix][cur_trail_ix] = cur_z;

      // if (!isnan(xnew))
      // {
      //   cout << p_ix << ":" <<cur_trail_ix << " " << xnew << " " << ynew <<
      //   " " << znew << " " << endl;
      // }
    }

    for (size_t i = 0; i < m_family.size(); ++i) {
      sf::Vector2f cur_pos = m_circles[i].getPosition();
      m_family[i].setTexture(&m_texture[i]);
      m_family[i].setPosition(cur_pos.x, cur_pos.y);
    }

    disable_update.unlock();
  }

 private:
  virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
    // apply the transform
    states.transform *= getTransform();

    // our particles don't use a texture
    states.texture = NULL;

    // draw the particle trail VertexArray for each particle
    for (std::size_t p_ix = 0; p_ix < m_circles.size(); ++p_ix) {
      target.draw(m_particle_trail[p_ix], states);
    }

    // draw each particle as a circle
    for (auto c : m_circles) {
      target.draw(c, states);
    }

    if (display_family == true) {
      for (auto c : m_family) {
        target.draw(c, states);
      }
    }

    // draw rotated x,y,z axis
    for (std::size_t a_ix = 0; a_ix < axes.size(); ++a_ix) {
      target.draw(axes[a_ix], states);
    }
  }

 public:
  int current_de;  // which ODE we are doing
  bool display_family = true;

 private:
  int particle_count;
  int trail_length;
  int cur_trail_ix;
  vector<sf::VertexArray> m_particle_trail;
  vector<std::vector<float>> m_particle_trail_z;

  std::vector<sf::CircleShape> m_circles;
  std::vector<float> m_circles_z;

  std::vector<sf::CircleShape> m_family;
  std::vector<sf::Texture> m_texture;

  vector<sf::VertexArray> axes;        // x,y,z drawable
  vector<vector<sf::Vector3f>> uaxes;  // unrotated

  mutex disable_update;
};

//Now we try to do the control elements displayed inside the view GUI that control
//both the model and the view (not just the view)

void setGuiElementsFromModel(shared_ptr<tgui::Gui> &pgui,
                             shared_ptr<DeStrangeSystem> &p_model);

void signalButton(int random, shared_ptr<DeStrangeSystem> p_model,
                  shared_ptr<tgui::Gui> pgui) {
  // std::cout << "Button pressed" << std::endl;
  if (random == 0) {
    p_model->resetDefaults();
    setGuiElementsFromModel(pgui, p_model);
  } else {
    for (size_t i = 0; i < DE[p_model->current_de].p.size(); ++i) {
      std::random_device rd;
      std::default_random_engine e2(rd());
      std::uniform_real_distribution<> dist(
          DE[p_model->current_de].MinMax[i].first,
          DE[p_model->current_de].MinMax[i].second);
      float pr = dist(e2);
      DE[p_model->current_de].p[i] = pr;
    }
    setGuiElementsFromModel(pgui, p_model);
  }
}
void updateGuiElements(shared_ptr<tgui::Gui> &pgui,
                       shared_ptr<DeStrangeSystem> &p_model);

void signalDeMenu(shared_ptr<DeStrangeSystem> p_model,
                  shared_ptr<tgui::Gui> pgui, const sf::String &selected) {
  for (size_t i = 0; i < DE.size(); ++i) {
    if (selected == DE[i].name) {
      p_model->current_de = i;
    }
  }
  updateGuiElements(pgui, p_model);
  p_model->resetDefaults();
  setGuiElementsFromModel(pgui, p_model);
}

void signalPde(shared_ptr<DeStrangeSystem> p_model, shared_ptr<tgui::Gui> pgui,
               tgui::ListBox::Ptr box) {
  // std::cout << "List changed to " <<  box->getSelectedItemIndex() <<
  // std::endl;
  p_model->setParticles(p_model->getParticles());

  size_t selection_ix = box->getSelectedItemIndex();

  if (selection_ix < DE[p_model->current_de].Examples[0].size()) {
    DE[p_model->current_de].p = DE[p_model->current_de].Examples[selection_ix];
  }
  setGuiElementsFromModel(pgui, p_model);
}

void signalParticles(shared_ptr<DeStrangeSystem> p_model,
                     tgui::Slider::Ptr slider) {
  // std::cout << "Particles changed to " << slider->getValue() << std::endl;
  p_model->setParticles(slider->getValue());
}

void signalTrail(shared_ptr<DeStrangeSystem> p_model,
                 tgui::Slider::Ptr slider) {
  // std::cout << "Particle Trail Length changed to " << slider->getValue() <<
  // std::endl;
  p_model->setTrail(slider->getValue());
}

void signalParticleSize(shared_ptr<DeStrangeSystem> p_model,
                        tgui::Slider::Ptr slider) {
  DE[p_model->current_de].current_particle_size = slider->getValue();
}

void signalDT(shared_ptr<DeStrangeSystem> p_model, tgui::Slider::Ptr slider) {
  DE[p_model->current_de].current_dt = slider->getValue();
}

void signalCenter(bool y, shared_ptr<sf::View> p_view,
                  tgui::Slider::Ptr slider) {
  if (y)
    R.ycenter = slider->getValue();
  else
    R.xcenter = slider->getValue();

  p_view->setCenter(R.xcenter, R.ycenter);
}

void signalColorMap(const sf::String &selected) {
  for (size_t i = 0; i < RGBMap.size(); ++i) {
    if (selected == RGBMap[i].name) {
      current_color_map_ix = i;
    }
  }
}

void signalParam(shared_ptr<DeStrangeSystem> p_model, size_t p_ix,
                 tgui::Slider::Ptr slider) {
  // std::cout << "De param changed to " << slider->getValue() << std::endl;
  DE[p_model->current_de].p[p_ix] = slider->getValue();
  // setGuiElementsFromModel(pgui, p_model);
}

void signalTheta(shared_ptr<DeStrangeSystem> p_model, size_t theta_ix,
                 tgui::Slider::Ptr slider) {
  R.theta[theta_ix] = slider->getValue();
  p_model->rotateAxes();
  // setGuiElementsFromModel(pgui, p_model);
}


//The main GUI elements inside the view
void createGuiElements(shared_ptr<tgui::Gui> pgui,
                       shared_ptr<DeStrangeSystem> &p_model,
                       shared_ptr<sf::View> &p_view) {
  int ystart = 950;
  int xstart = 0;

  // Current and Menu Column
  auto menu = tgui::MenuBar::create();
  menu->setPosition(xstart, ystart + 80);
  // static_cast<float>(window.getSize().x)
  menu->setSize(200.f, 22.f);
  menu->addMenu("ODE Strange Attractors");
  for (auto de : DE) {
    menu->addMenuItem(de.name);
  }
  menu->addMenu("Help");
  menu->addMenuItem("Use mouse wheel to zoom");
  menu->addMenuItem("Type h to hide/show gui");
  menu->addMenuItem("Type l to hide logo");
  menu->addMenuItem("Type f to hide/show family");
  menu->addMenuItem("Type s to take a screenshot");

  pgui->add(menu);

  auto current = tgui::Label::create();
  current->setPosition(xstart, ystart);
  current->setTextSize(14);
  pgui->add(current, "secs_label");

  current = tgui::Label::create();
  current->setPosition(xstart + 150, ystart);
  current->setTextSize(14);
  pgui->add(current, "fps_label");

  current = tgui::Label::create();
  current->setPosition(xstart, ystart + 20);
  current->setTextSize(14);
  pgui->add(current, "particles_label");

  current = tgui::Label::create();
  current->setPosition(xstart + 150, ystart + 20);
  current->setTextSize(14);
  pgui->add(current, "trail_label");

  current = tgui::Label::create();
  current->setPosition(xstart, ystart + 40);
  current->setTextSize(14);
  pgui->add(current, "vertices_label");

  current = tgui::Label::create();
  current->setPosition(xstart, ystart + 60);
  current->setTextSize(14);
  pgui->add(current, "attractor_label");

  int xcolstart = xstart + 300;
  // Params column
  auto buttond = tgui::Button::create();
  buttond->setPosition(xcolstart, ystart);
  buttond->setText("Default Params");
  buttond->setSize(150, 30);
  pgui->add(buttond, "Default");

  auto buttonr = tgui::Button::create();
  buttonr->setPosition(xcolstart, ystart + 35);
  buttonr->setText("Random Params");
  buttonr->setSize(150, 30);
  pgui->add(buttonr, "Random");

  auto pde_box = tgui::ListBox::create();
  pde_box->setSize(xstart + 200, 85);
  pde_box->setItemHeight(20);
  pde_box->setPosition(xcolstart, ystart + 70);
  for (auto ex : DE[p_model->current_de].Examples) {
    string exs("");
    for (auto exp : ex) {
      exs += to_string(exp).substr(0, to_string(exp).find(".") + 3) + ", ";
    }
    pde_box->addItem(exs);
  }
  string exs("");
  for (auto pn : DE[p_model->current_de].PNames) {
    exs += pn + ", ";
  }
  pde_box->addItem(exs);

  pgui->add(pde_box, "pde_box");

  current = tgui::Label::create();  // text set later
  current->setPosition(xcolstart, ystart + 175);
  current->setTextSize(14);
  pgui->add(current, "params_label");

  sf::Vector2u slider_size(200, 18);
  xcolstart = xstart + 650;

  // Particle Column
  auto trail_slider = tgui::Slider::create(1.0, 30000.0);
  trail_slider->setValue(5000);
  trail_slider->setStep(1);
  auto particles_slider = tgui::Slider::create(1.0, 256.0);
  particles_slider->setValue(100);
  particles_slider->setStep(1);

  particles_slider->setPosition(xcolstart, ystart);
  particles_slider->setSize(slider_size.x, slider_size.y);
  trail_slider->setPosition(xcolstart, ystart + 25);
  trail_slider->setSize(slider_size.x, slider_size.y);

  pgui->add(particles_slider, "particle_slider");
  pgui->add(trail_slider, "trail_slider");

  auto cmenu = tgui::MenuBar::create();
  cmenu->setPosition(xcolstart + 50, ystart + 150);
  // static_cast<float>(window.getSize().x)
  cmenu->setSize(150.f, 22.f);
  cmenu->addMenu("Trail Color Map");
  for (auto color : RGBMap) {
    cmenu->addMenuItem(color.name);
  }
  pgui->add(cmenu);

  auto particle_size_slider = tgui::Slider::create(0.01, 4.0);
  particle_size_slider->setValue(.03);
  particle_size_slider->setStep(.01);
  particle_size_slider->setPosition(xcolstart, ystart + 75);
  particle_size_slider->setSize(slider_size.x, slider_size.y);
  pgui->add(particle_size_slider, "particle_size_slider");

  auto dt_slider = tgui::Slider::create(0.0001, 0.05);
  dt_slider->setValue(.01);
  dt_slider->setStep(.0001);
  dt_slider->setPosition(xcolstart, ystart + 100);
  dt_slider->setSize(slider_size.x, slider_size.y);
  pgui->add(dt_slider, "dt_slider");

  auto xcenter_slider = tgui::Slider::create(-10, 10);
  xcenter_slider->setValue(0);
  xcenter_slider->setStep(0.1);
  xcenter_slider->setPosition(xcolstart, ystart + 125);
  xcenter_slider->setSize(slider_size.x, slider_size.y);
  pgui->add(xcenter_slider, "xcenter_slider");
  auto ycenter_slider = tgui::Slider::create(-10, 10.0);
  ycenter_slider->setValue(0);
  ycenter_slider->setStep(0.1);
  ycenter_slider->setPosition(xcolstart, ystart + 150);
  ycenter_slider->setSize(slider_size.x - 100, slider_size.y);
  ycenter_slider->setVerticalScroll(true);
  pgui->add(ycenter_slider, "ycenter_slider");

  auto label = tgui::Label::create("Particles");
  label->setPosition("particle_slider.left - width - 10",
                     "particle_slider.top");
  label->setTextSize(14);
  pgui->add(label, "particles_label");

  label = tgui::Label::create("Trail");
  label->setPosition("trail_slider.left - width - 10", "trail_slider.top");
  label->setTextSize(14);
  pgui->add(label, "trail_label");

  label = tgui::Label::create("Particle Size");
  label->setPosition("particle_size_slider.left - width - 10",
                     "particle_size_slider.top");
  label->setTextSize(14);
  pgui->add(label, "particle_size_label");

  label = tgui::Label::create();
  label->setText("dT");
  label->setPosition("dt_slider.left - width - 10", "dt_slider.top");
  label->setTextSize(14);
  pgui->add(label, "dt_label");

  label = tgui::Label::create();
  label->setText("xcenter");
  label->setPosition("xcenter_slider.left - width - 10", "xcenter_slider.top");
  label->setTextSize(14);
  pgui->add(label, "xcenter_label");

  label = tgui::Label::create();
  label->setText("ycenter");
  label->setPosition("ycenter_slider.left - width - 10", "ycenter_slider.top");
  label->setTextSize(14);
  pgui->add(label, "ycenter_label");

  xcolstart = xstart + 650 + 320;
  // Params and Theta Column
  auto p0_slider = tgui::Slider::create(1.0, 100.0);
  p0_slider->setStep(0.1);
  auto p1_slider = tgui::Slider::create(1.0, 100.0);
  p1_slider->setStep(0.1);
  auto p2_slider = tgui::Slider::create(1.0, 100.0);
  p2_slider->setStep(0.1);
  auto p3_slider = tgui::Slider::create(1.0, 100.0);
  p3_slider->setStep(0.1);
  auto p4_slider = tgui::Slider::create(1.0, 100.0);
  p4_slider->setStep(0.1);
  auto p5_slider = tgui::Slider::create(1.0, 100.0);
  p5_slider->setStep(0.1);

  p0_slider->setPosition(xcolstart, ystart);
  p0_slider->setSize(slider_size.x, slider_size.y);
  p1_slider->setPosition(xcolstart, ystart + 25);
  p1_slider->setSize(slider_size.x, slider_size.y);
  p2_slider->setPosition(xcolstart, ystart + 50);
  p2_slider->setSize(slider_size.x, slider_size.y);
  p3_slider->setPosition(xcolstart, ystart + 75);
  p3_slider->setSize(slider_size.x, slider_size.y);
  p4_slider->setPosition(xcolstart, ystart + 100);
  p4_slider->setSize(slider_size.x, slider_size.y);
  p5_slider->setPosition(xcolstart, ystart + 125);
  p5_slider->setSize(slider_size.x, slider_size.y);

  auto Thetax_slider = tgui::Slider::create(0.0, 180.0);
  auto Thetay_slider = tgui::Slider::create(0.0, 180.0);
  auto Thetaz_slider = tgui::Slider::create(0.0, 180.0);

  Thetax_slider->setPosition(xcolstart, ystart + 175);
  Thetax_slider->setSize(slider_size.x, slider_size.y);
  Thetay_slider->setPosition(xcolstart, ystart + 200);
  Thetay_slider->setSize(slider_size.x, slider_size.y);
  Thetaz_slider->setPosition(xcolstart, ystart + 225);
  Thetaz_slider->setSize(slider_size.x, slider_size.y);

  pgui->add(p0_slider, "p0_slider");
  pgui->add(p1_slider, "p1_slider");
  pgui->add(p2_slider, "p2_slider");
  pgui->add(p3_slider, "p3_slider");
  pgui->add(p4_slider, "p4_slider");
  pgui->add(p5_slider, "p5_slider");

  pgui->add(Thetax_slider, "Thetax_slider");
  pgui->add(Thetay_slider, "Thetay_slider");
  pgui->add(Thetaz_slider, "Thetaz_slider");

  current = tgui::Label::create();
  current->setPosition(xcolstart, ystart + 150);
  current->setTextSize(14);
  pgui->add(current, "theta_label");

  // "σ" "β" "ρ" "θ"  Dont display in GUI

  label = tgui::Label::create();
  // label->setText(DE[p_model->current_de].PNames[0]);
  label->setPosition("p0_slider.left - width - 10", "p0_slider.top");
  label->setTextSize(14);
  pgui->add(label, "p0_label");

  label = tgui::Label::create();
  // label->setText(DE[p_model->current_de].PNames[1]);
  label->setPosition("p1_slider.left - width - 10", "p1_slider.top");
  label->setTextSize(14);
  pgui->add(label, "p1_label");

  label = tgui::Label::create();
  // label->setText(DE[p_model->current_de].PNames[2]);
  label->setPosition("p2_slider.left - width - 10", "p2_slider.top");
  label->setTextSize(14);
  pgui->add(label, "p2_label");

  label = tgui::Label::create();
  // label->setText(DE[p_model->current_de].PNames[3]);
  label->setPosition("p3_slider.left - width - 10", "p3_slider.top");
  label->setTextSize(14);
  pgui->add(label, "p3_label");

  label = tgui::Label::create();
  // label->setText(DE[p_model->current_de].PNames[4]);
  label->setPosition("p4_slider.left - width - 10", "p4_slider.top");
  label->setTextSize(14);
  pgui->add(label, "p4_label");

  label = tgui::Label::create();
  // label->setText(DE[p_model->current_de].PNames[5]);
  label->setPosition("p5_slider.left - width - 10", "p5_slider.top");
  label->setTextSize(14);
  pgui->add(label, "p5_label");

  label = tgui::Label::create();
  label->setText("ThetaX");
  label->setPosition("Thetax_slider.left - width - 10", "Thetax_slider.top");
  label->setTextSize(14);
  pgui->add(label, "Thetax_label");

  label = tgui::Label::create();
  label->setText("ThetaY");
  label->setPosition("Thetay_slider.left - width - 10", "Thetay_slider.top");
  label->setTextSize(14);
  pgui->add(label, "Thetay_label");

  label = tgui::Label::create();
  label->setText("ThetaZ");
  label->setPosition("Thetaz_slider.left - width - 10", "Thetaz_slider.top");
  label->setTextSize(14);
  pgui->add(label, "Thetaz_label");

  // Connects
  menu->connect("MenuItemClicked", signalDeMenu, p_model, pgui);
  cmenu->connect("MenuItemClicked", signalColorMap);

  buttond->connect("Pressed", signalButton, 0, p_model, pgui);
  buttonr->connect("Pressed", signalButton, 1, p_model, pgui);
  particles_slider->connect("ValueChanged", signalParticles, p_model,
                            particles_slider);
  trail_slider->connect("ValueChanged", signalTrail, p_model, trail_slider);
  dt_slider->connect("ValueChanged", signalDT, p_model, dt_slider);
  particle_size_slider->connect("ValueChanged", signalParticleSize, p_model,
                                particle_size_slider);
  xcenter_slider->connect("ValueChanged", signalCenter, 0, p_view,
                          xcenter_slider);
  ycenter_slider->connect("ValueChanged", signalCenter, 1, p_view,
                          ycenter_slider);

  p0_slider->connect("ValueChanged", signalParam, p_model, 0, p0_slider);
  p1_slider->connect("ValueChanged", signalParam, p_model, 1, p1_slider);
  p2_slider->connect("ValueChanged", signalParam, p_model, 2, p2_slider);
  p3_slider->connect("ValueChanged", signalParam, p_model, 3, p3_slider);
  p4_slider->connect("ValueChanged", signalParam, p_model, 4, p4_slider);
  p5_slider->connect("ValueChanged", signalParam, p_model, 5, p5_slider);

  Thetax_slider->connect("ValueChanged", signalTheta, p_model, 0,
                         Thetax_slider);
  Thetay_slider->connect("ValueChanged", signalTheta, p_model, 1,
                         Thetay_slider);
  Thetaz_slider->connect("ValueChanged", signalTheta, p_model, 2,
                         Thetaz_slider);

  pde_box->connect("ItemSelected", signalPde, p_model, pgui, pde_box);
  pde_box->connect("MousePressed", signalPde, p_model, pgui, pde_box);
}

// When strange attractor changes
void updateGuiElements(shared_ptr<tgui::Gui> &pgui,
                       shared_ptr<DeStrangeSystem> &p_model) {
  // hide all px sliders and their labels
  // show the ones that are needed
  // show and update the labels with the right text
  // set the sliders to the default values and min maxes

  tgui::Slider::Ptr widgets[6] = {pgui->get<tgui::Slider>("p0_slider"),
                                  pgui->get<tgui::Slider>("p1_slider"),
                                  pgui->get<tgui::Slider>("p2_slider"),
                                  pgui->get<tgui::Slider>("p3_slider"),
                                  pgui->get<tgui::Slider>("p4_slider"),
                                  pgui->get<tgui::Slider>("p5_slider")};

  tgui::Label::Ptr widgetl[6] = {
      pgui->get<tgui::Label>("p0_label"), pgui->get<tgui::Label>("p1_label"),
      pgui->get<tgui::Label>("p2_label"), pgui->get<tgui::Label>("p3_label"),
      pgui->get<tgui::Label>("p4_label"), pgui->get<tgui::Label>("p5_label")};

  for (size_t i = 0; i < 6; ++i) {
    widgets[i]->setVisible(false);
    widgetl[i]->setVisible(false);
  }

  for (size_t i = 0; i < DE[p_model->current_de].p.size(); ++i) {
    widgets[i]->setMinimum(DE[p_model->current_de].MinMax[i].first);
    widgets[i]->setMaximum(DE[p_model->current_de].MinMax[i].second);
    widgets[i]->setValue(DE[p_model->current_de].p[i]);
    widgets[i]->setVisible(true);
    widgetl[i]->setText(DE[p_model->current_de].PNames[i]);
    widgetl[i]->setVisible(true);
  }

  // Update Pde Examples
  tgui::ListBox::Ptr pde_box = pgui->get<tgui::ListBox>("pde_box");
  pde_box->removeAllItems();

  for (auto ex : DE[p_model->current_de].Examples) {
    string exs("");
    for (auto exp : ex) {
      exs += to_string(exp).substr(0, to_string(exp).find(".") + 3) + ", ";
    }
    pde_box->addItem(exs);
  }
  string exs("");
  for (auto pn : DE[p_model->current_de].PNames) {
    exs += pn + ", ";
  }
  pde_box->addItem(exs);

  tgui::Slider::Ptr widgetdt = pgui->get<tgui::Slider>("dt_slider");
  DE[p_model->current_de].current_dt = DE[p_model->current_de].dt;
  widgetdt->setValue(DE[p_model->current_de].dt);

  tgui::Slider::Ptr widgetps = pgui->get<tgui::Slider>("particle_size_slider");
  DE[p_model->current_de].current_particle_size =
      DE[p_model->current_de].particle_size;
  widgetps->setValue(DE[p_model->current_de].current_particle_size);
}

// When some per attractor slider changes: params,theta,particles,trail
void setGuiElementsFromModel(shared_ptr<tgui::Gui> &pgui,
                             shared_ptr<DeStrangeSystem> &p_model) {
  tgui::Slider::Ptr widgets[6] = {pgui->get<tgui::Slider>("p0_slider"),
                                  pgui->get<tgui::Slider>("p1_slider"),
                                  pgui->get<tgui::Slider>("p2_slider"),
                                  pgui->get<tgui::Slider>("p3_slider"),
                                  pgui->get<tgui::Slider>("p4_slider"),
                                  pgui->get<tgui::Slider>("p5_slider")};

  for (size_t i = 0; i < DE[p_model->current_de].p.size(); ++i) {
    widgets[i]->setValue(DE[p_model->current_de].p[i]);
  }

  tgui::Slider::Ptr widgett[3] = {pgui->get<tgui::Slider>("Thetax_slider"),
                                  pgui->get<tgui::Slider>("Thetay_slider"),
                                  pgui->get<tgui::Slider>("Thetaz_slider")};

  for (size_t i = 0; i < 3; ++i) {
    widgett[i]->setValue(R.theta[i]);
  }

  tgui::Slider::Ptr sliderp = pgui->get<tgui::Slider>("particle_slider");
  sliderp->setValue(p_model->getParticles());

  tgui::Slider::Ptr slidert = pgui->get<tgui::Slider>("trail_slider");
  slidert->setValue(p_model->getTrail());
}

// update current things every draw even if no change
void updateCurrentGuiElements(shared_ptr<tgui::Gui> &pgui,
                              shared_ptr<DeStrangeSystem> &p_model, float secs,
                              float fps) {
  // secs vertices
  // fps particles x/y   trail
  // params
  // theta
  auto current = pgui->get<tgui::Label>("secs_label");
  current->setText("Secs: " + to_string(secs));

  current = pgui->get<tgui::Label>("fps_label");
  current->setText("fps: " + to_string(fps));

  current = pgui->get<tgui::Label>("particles_label");
  current->setText("particles: " +
                   to_string(p_model->getParticles() - p_model->getEscaped()) +
                   "/" + to_string(p_model->getParticles()));

  current = pgui->get<tgui::Label>("trail_label");
  current->setText("trail: " + to_string(p_model->getTrail()));

  current = pgui->get<tgui::Label>("vertices_label");
  current->setText("vertices: " +
                   to_string(p_model->getTrail() * p_model->getParticles()));

  current = pgui->get<tgui::Label>("attractor_label");
  current->setText("Attractor: " + DE[p_model->current_de].name);

  string params("");
  for (size_t i = 0; i < DE[p_model->current_de].p.size(); ++i) {
    params +=
        to_string(DE[p_model->current_de].p[i])
            .substr(0, to_string(DE[p_model->current_de].p[i]).find(".") + 3) +
        ", ";
  }
  current = pgui->get<tgui::Label>("params_label");
  current->setText(params);

  string theta("");
  for (size_t i = 0; i < 3; ++i) {
    theta +=
        to_string(R.theta[i]).substr(0, to_string(R.theta[i]).find(".") + 3) +
        ", ";
  }
  current = pgui->get<tgui::Label>("theta_label");
  current->setText(theta);
}

void display_all_widgets(shared_ptr<tgui::Gui> &pgui, bool yes) {
  std::vector<tgui::Widget::Ptr> widgets;
  widgets = pgui->getWidgets();

  for (auto &w : widgets) {
    w->setVisible(yes);
  }
}

// respond to mouse wheel zoom
void zoom_view(sf::View &view, int delta, unsigned int origx,
               unsigned int origy) {
  static float current_zoom = 0.0;

  if (delta < 0) {
    // zoom in
    current_zoom -= 0.03;
    // cout << "zoom: " << current_zoom << endl;
    view.zoom(0.97);
    if (current_zoom < -5.0) {
      view.setSize(origx, origy);
      current_zoom = 0.0;
    }
  } else {
    // zoom out
    current_zoom += 0.03;
    // cout << "zoom: " << current_zoom << endl;
    view.zoom(1.03);
    if (current_zoom > 5.0) {
      view.setSize(origx, origy);
      current_zoom = 0.0;
    }
  }
}

//save a screenshot 
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

int main() {
  // create the particle system (i.e. Model)
  //    note: we are passing this shared ptr to signals so they can change the
  //    model (MVC)
  auto p_model = make_shared<DeStrangeSystem>(100, 5000);

  sf::Vector2u screenDimensions(1200, 1200);
  sf::RenderWindow window(sf::VideoMode(screenDimensions.x, screenDimensions.y),
                          "ODE Strange Attractor Simulator");
  window.setKeyRepeatEnabled(false);

  // Main window
  sf::View model;
  shared_ptr<sf::View> p_view{&model};
  sf::Vector2u viewD(200, 200);
  R.xcenter = 0;
  R.ycenter = 0;
  model.setCenter(R.xcenter, R.ycenter);
  model.setSize(viewD.x, viewD.y);
  window.setView(model);

  // Create the gui and attach it to the window
  auto pgui = make_shared<tgui::Gui>(window);
  bool display_gui = true;
  bool display_logo = true;
  tgui::Theme theme{"themes/BabyBlue.txt"};
  tgui::Theme::setDefault(&theme);
  createGuiElements(pgui, p_model, p_view);
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

      //Handle keyboard control commands
      if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::F) {
          if (p_model->display_family == false)
            p_model->display_family = true;
          else
            p_model->display_family = false;
        }
      }

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
        if (event.key.code == sf::Keyboard::L) {
          if (display_logo == false) {
            display_logo = true;
          } else {
            display_logo = false;
          }
        }
      }

      if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::S) {
          save_screenshot(window, DE[p_model->current_de].name);
        }
      }
      
      //Handle Mouse
      // Zoom the whole sim if mouse wheel moved
      if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
          zoom_view(model, event.mouseWheelScroll.delta, viewD.x, viewD.y);
        }
      }

      //Handle the control coming from the gui
      pgui->handleEvent(
          event);  // Pass the User slider changes event to the widgets
    }
    ++frames;

    //Evolve the model independantly
    sf::Time elapsed = clock_e.restart();
    p_model->update(elapsed);  // move the particles dt time forward

    //Draw the GUI and the MODEL both of which are controlled by
    // Keyboard, mouse, and gui elements
    updateCurrentGuiElements(pgui, p_model, start.asSeconds(),
                             frames / start.asSeconds());

    start = clock_s.getElapsedTime();

    window.clear();
    window.setView(model);
    window.draw(*p_model);  // draw particles, particle trails, and axes
    pgui->draw();           // Draw all widgets

    window.display();
  }

  return 0;
}
