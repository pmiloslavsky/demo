#include <math.h>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <random>

using namespace std;

class LorenzSystem : public sf::Drawable, public sf::Transformable {
 public:
  LorenzSystem(unsigned int _particle_count, unsigned int _trail_length,
               float _sigma = 10.0, float _beta = 8.0 / 3.0, float _rho = 28.0,
               float _anglex = 0.0, float _angley = 0.0, float _anglez = 0.0)
      : particle_count(_particle_count),
        trail_length(_trail_length),
        sigma(_sigma),
        beta(_beta),
        rho(_rho),
        anglex(_anglex),
        angley(_angley),
        anglez(_anglez)

  {     
    cur_trail_ix = 0;
    setParticles(particle_count);
    setTrail(trail_length);

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
        rotatePoint(uaxes[a_ix][p].x, uaxes[a_ix][p].y, uaxes[a_ix][p].z, xnew,
                    ynew, znew, false);
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
    sigma = 10.0;
    beta = 8.0 / 3.0;
    rho = 28.0;
    cur_trail_ix = 0;

    // need to wipe out all particles and trails first
    m_circles.resize(0);
    m_circles_z.resize(0);
    m_particle_trail.resize(0);
    m_particle_trail_z.resize(0);

    setParticles(20);
    setTrail(1000);
    
    disable_update.unlock();
  }

  void testRotation() {
    float xnew, ynew, znew;
    cout << "ThetaXYZ: " << anglex << "," << angley << "," << anglez << endl;
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

  void setSigma(float new_sigma) { sigma = new_sigma; }

  float getSigma(void) { return sigma; }

  void setBeta(float new_beta) { beta = new_beta; }

  float getBeta(void) { return beta; }

  void setRho(float new_rho) { rho = new_rho; }

  float getRho(void) { return rho; }

  float getAx(void) { return anglex; }

  float getAy(void) { return angley; }

  float getAz(void) { return anglez; }

  void setAx(float val) { anglex = val; }

  void setAy(float val) { angley = val; }

  void setAz(float val) { anglez = val; }

  std::size_t getParticles(void) { return m_circles.size(); }

  void setParticles(int count) {
    std::size_t start_p_ix = 0;
    bool new_particles = false;
    if (count - m_circles.size() > 0) {
      new_particles = true;
      start_p_ix = m_circles.size();
    }

    particle_count = count;
    m_circles.resize(count, sf::CircleShape(.33));
    m_circles_z.resize(count, 0.0f);

    m_particle_trail.resize(particle_count);
    m_particle_trail_z.resize(particle_count);

    if (false == new_particles) return;

    std::random_device rd;
    std::default_random_engine e2(rd());
    std::uniform_real_distribution<> dist(-100, 100);

    for (std::size_t i = start_p_ix; i < m_circles.size(); ++i) {
      float xnew, ynew, znew;
      float xu = dist(e2);
      float yu = dist(e2);
      float zu = dist(e2);
      //cout << xu << yu << zu << endl;
      rotatePoint(xu, yu, zu, xnew,
                  ynew, znew, false);
      m_circles[i].setPosition(xnew, ynew);
      m_circles[i].setFillColor(sf::Color(0, 255 - i * 20, 120 + i * 20));
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

  void rotatePoint(float x, float y, float z, float &xnew, float &ynew,
                   float &znew, bool back) {
    // Now make xnew,ynew,znew rotate for the given euler angles
    float xin = x;
    float yin = y;
    float zin = z;

    float xout = x;
    float yout = y;
    float zout = z;

    if (back == false) {
      if (anglex != 0) {
        float xangle = (M_PI * anglex) / 180.0;
        float cosx = cos(xangle);
        float sinx = sin(xangle);

        xout = xin;
        yout = yin * cosx - zin * sinx;
        zout = yin * sinx + zin * cosx;
        xin = xout;
        yin = yout;
        zin = zout;
      }

      if (angley != 0) {
        float yangle = (M_PI * angley) / 180.0;
        float cosy = cos(yangle);
        float siny = sin(yangle);

        xout = xin * cosy + zin * siny;
        yout = yin;
        zout = zin * cosy - xin * siny;
        xin = xout;
        yin = yout;
        zin = zout;
      }

      if (anglez != 0) {
        float zangle = (M_PI * anglez) / 180.0;
        float cosz = cos(zangle);
        float sinz = sin(zangle);

        xout = xin * cosz - yin * sinz;
        yout = xin * sinz + yin * cosz;
        zout = zin;
        xin = xout;
        yin = yout;
        zin = zout;
      }
    } else {
      if (anglez != 0) {
        float zangle = -(M_PI * anglez) / 180.0;
        float cosz = cos(zangle);
        float sinz = sin(zangle);

        xout = xin * cosz - yin * sinz;
        yout = xin * sinz + yin * cosz;
        zout = zin;
        xin = xout;
        yin = yout;
        zin = zout;
      }

      if (angley != 0) {
        float yangle = -(M_PI * angley) / 180.0;
        float cosy = cos(yangle);
        float siny = sin(yangle);

        xout = xin * cosy + zin * siny;
        yout = yin;
        zout = zin * cosy - xin * siny;
        xin = xout;
        yin = yout;
        zin = zout;
      }

      if (anglex != 0) {
        float xangle = -(M_PI * anglex) / 180.0;
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

    // Doesnt work
    //
    // float xangle = (M_PI*anglex)/180.0;
    // if (back == true) xangle = -xangle;
    // float cosx = cos(xangle);
    // float sinx = sin(xangle);

    // float yangle = (M_PI*angley)/180.0;
    // if (back == true) yangle = -yangle;
    // float cosy = cos(yangle);
    // float siny = sin(yangle);

    // float zangle = (M_PI*anglez)/180.0;
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

  // apply pde only in original reference frame
  void apply_lorenz_pde(float x, float y, float z, float dt, float &xnew,
                        float &ynew, float &znew) {
    // 10.0,8.0/3.0,28.0
    // xnew = x + dt*sigma*(y - x);
    // ynew = y + dt* (x*(rho - z) - y);
    // znew = z + dt * (x*y - beta*z);

    // rotate existing x,y,z back x degrees
    float xrot, yrot, zrot;
    rotatePoint(x, y, z, xrot, yrot, zrot, true);

    // apply the pde
    float xpde = xrot + dt * sigma * (yrot - xrot);
    float ypde = yrot + dt * (xrot * (rho - zrot) - yrot);
    float zpde = zrot + dt * (xrot * yrot - beta * zrot);

    // rotate xnew ynew znew forwards x degrees
    rotatePoint(xpde, ypde, zpde, xnew, ynew, znew, false);
  }

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

  void update(sf::Time elapsed) {
    //a little bit of protection since we are changing array size in callback signal context
    disable_update.lock();
    // change the position of the circles and add a new element to the trail
    ++cur_trail_ix;

    if (cur_trail_ix >= static_cast<int>(getTrail())) {
      cur_trail_ix = 0;
    }

    for (std::size_t p_ix = 0; p_ix < m_circles.size(); ++p_ix) {
      float dt = 0.001;  // slower is smaller
      sf::Vector2f cur_pos = m_circles[p_ix].getPosition();
      float xnew;
      float ynew;
      float znew;
      apply_lorenz_pde(cur_pos.x, cur_pos.y, m_circles_z[p_ix], dt, xnew, ynew,
                       znew);
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

    // draw rotated x,y,z axis
    for (std::size_t a_ix = 0; a_ix < axes.size(); ++a_ix) {
      target.draw(axes[a_ix], states);
    }
  }

  int particle_count;
  int trail_length;
  int cur_trail_ix;
  vector<sf::VertexArray> m_particle_trail;
  vector<std::vector<float>> m_particle_trail_z;

  std::vector<sf::CircleShape> m_circles;
  std::vector<float> m_circles_z;

  vector<sf::VertexArray> axes;        // x,y,z drawable
  vector<vector<sf::Vector3f>> uaxes;  // unrotated

  // pde params
  float sigma;
  float beta;
  float rho;

  // hue
  sf::Uint8 r = 255;
  sf::Uint8 g = 0;
  sf::Uint8 b = 0;

  // view rotation (euler theta)
  float anglex;
  float angley;
  float anglez;

  mutex disable_update;
};

void signalButton(shared_ptr<LorenzSystem> p_lorenz) {
  // std::cout << "Button pressed" << std::endl;
  p_lorenz->resetDefaults();
}

void signalPde(shared_ptr<LorenzSystem> p_lorenz, tgui::ListBox::Ptr box) {
  // std::cout << "List changed to " <<  box->getSelectedItemIndex() <<
  // std::endl;
  p_lorenz->setParticles(p_lorenz->getParticles());
  switch (box->getSelectedItemIndex()) {
    case 0:
    default:
      p_lorenz->setSigma(10.0);
      p_lorenz->setBeta(8.0 / 3.0);
      p_lorenz->setRho(28.0);
      break;
    case 1:
      p_lorenz->setSigma(12.69);
      p_lorenz->setBeta(0.13);
      p_lorenz->setRho(53.03);
      break;
    case 2:
      p_lorenz->setSigma(95.03);
      p_lorenz->setBeta(0.19);
      p_lorenz->setRho(82.7);
      break;
    case 3:
      p_lorenz->setSigma((std::rand() % 10000) / 100.0);
      p_lorenz->setBeta((std::rand() % 10000) / 100.0);
      p_lorenz->setRho((std::rand() % 10000) / 100.0);
      break;
  }
}

void signalParticles(shared_ptr<LorenzSystem> p_lorenz,
                     tgui::Slider::Ptr slider) {
  // std::cout << "Particles changed to " << slider->getValue() << std::endl;
  p_lorenz->setParticles(slider->getValue());
}

void signalTrail(shared_ptr<LorenzSystem> p_lorenz, tgui::Slider::Ptr slider) {
  // std::cout << "Particle Trail Length changed to " << slider->getValue() <<
  // std::endl;
  p_lorenz->setTrail(slider->getValue());
}

void signalSigma(shared_ptr<LorenzSystem> p_lorenz, tgui::Slider::Ptr slider) {
  // std::cout << "Sigma changed to " << slider->getValue() << std::endl;
  p_lorenz->setSigma(slider->getValue());
}

void signalBeta(shared_ptr<LorenzSystem> p_lorenz, tgui::Slider::Ptr slider) {
  // std::cout << "Beta changed to " << slider->getValue() << std::endl;
  p_lorenz->setBeta(slider->getValue());
}

void signalRho(shared_ptr<LorenzSystem> p_lorenz, tgui::Slider::Ptr slider) {
  // std::cout << "Rho changed to " << slider->getValue() << std::endl;
  p_lorenz->setRho(slider->getValue());
}

void signalAx(shared_ptr<LorenzSystem> p_lorenz, tgui::Slider::Ptr slider) {
  p_lorenz->setAx(slider->getValue());
  p_lorenz->rotateAxes();
  // p_lorenz->testRotation();
}
void signalAy(shared_ptr<LorenzSystem> p_lorenz, tgui::Slider::Ptr slider) {
  p_lorenz->setAy(slider->getValue());
  p_lorenz->rotateAxes();
  // p_lorenz->testRotation();
}
void signalAz(shared_ptr<LorenzSystem> p_lorenz, tgui::Slider::Ptr slider) {
  p_lorenz->setAz(slider->getValue());
  p_lorenz->rotateAxes();
  // p_lorenz->testRotation();
}

void createGuiElements(tgui::Gui &gui, shared_ptr<LorenzSystem> &p_lorenz) {

  auto button = tgui::Button::create();
  button->setPosition(0, 1000);
  button->setText("Defaults");
  button->setSize(100, 30);
  gui.add(button, "Defaults");

  auto pde_box = tgui::ListBox::create();
  pde_box->setSize(200, 85);
  pde_box->setItemHeight(20);
  pde_box->setPosition(300, 975);
  pde_box->addItem("sigma:10 beta:8/3 rho:28");
  pde_box->addItem("12.69 0.13 53.03");
  pde_box->addItem("95.03 0.19 82.7");
  pde_box->addItem("Random");
  gui.add(pde_box, "pde_box");

  auto trail_slider = tgui::Slider::create(1.0, 30000.0);
  trail_slider->setValue(1000);
  trail_slider->setStep(1);
  auto particles_slider = tgui::Slider::create(1.0, 200.0);
  particles_slider->setValue(20);
  particles_slider->setStep(1);
  // 10.0,8.0/3.0,28.0
  auto sigma_slider = tgui::Slider::create(1.0, 100.0);
  sigma_slider->setValue(10.0);
  auto beta_slider = tgui::Slider::create(0.0, 10.0);
  beta_slider->setValue(8.0 / 3.0);
  beta_slider->setStep(0.1);
  auto rho_slider = tgui::Slider::create(1.0, 100.0);
  rho_slider->setValue(28.0);

  auto Ax_slider = tgui::Slider::create(0.0, 90.0);
  Ax_slider->setValue(0.0);
  auto Ay_slider = tgui::Slider::create(0.0, 90.0);
  Ay_slider->setValue(0.0);
  auto Az_slider = tgui::Slider::create(0.0, 90.0);
  Az_slider->setValue(0.0);

  sf::Vector2u slider_size(200, 18);
  int slider_lx1 = 600;
  int slider_ty1 = 1000;

  particles_slider->setPosition(slider_lx1, slider_ty1);
  particles_slider->setSize(slider_size.x, slider_size.y);

  trail_slider->setPosition(slider_lx1, slider_ty1 + 25);
  trail_slider->setSize(slider_size.x, slider_size.y);

  sigma_slider->setPosition(slider_lx1 + 320, slider_ty1);
  sigma_slider->setSize(slider_size.x, slider_size.y);

  beta_slider->setPosition(slider_lx1 + 320, slider_ty1 + 25);
  beta_slider->setSize(slider_size.x, slider_size.y);

  rho_slider->setPosition(slider_lx1 + 320, slider_ty1 + 50);
  rho_slider->setSize(slider_size.x, slider_size.y);

  Ax_slider->setPosition(slider_lx1 + 320, slider_ty1 + 100);
  Ax_slider->setSize(slider_size.x, slider_size.y);

  Ay_slider->setPosition(slider_lx1 + 320, slider_ty1 + 125);
  Ay_slider->setSize(slider_size.x, slider_size.y);

  Az_slider->setPosition(slider_lx1 + 320, slider_ty1 + 150);
  Az_slider->setSize(slider_size.x, slider_size.y);

  gui.add(particles_slider, "particle_slider");
  gui.add(trail_slider, "trail_slider");
  gui.add(sigma_slider, "sigma_slider");
  gui.add(beta_slider, "beta_slider");
  gui.add(rho_slider, "rho_slider");
  gui.add(Ax_slider, "Ax_slider");
  gui.add(Ay_slider, "Ay_slider");
  gui.add(Az_slider, "Az_slider");

  button->connect("Pressed", signalButton, p_lorenz);
  particles_slider->connect("ValueChanged", signalParticles, p_lorenz,
                            particles_slider);
  trail_slider->connect("ValueChanged", signalTrail, p_lorenz, trail_slider);
  sigma_slider->connect("ValueChanged", signalSigma, p_lorenz, sigma_slider);
  beta_slider->connect("ValueChanged", signalBeta, p_lorenz, beta_slider);
  rho_slider->connect("ValueChanged", signalRho, p_lorenz, rho_slider);
  pde_box->connect("ItemSelected", signalPde, p_lorenz, pde_box);
  pde_box->connect("MousePressed", signalPde, p_lorenz, pde_box);
  Ax_slider->connect("ValueChanged", signalAx, p_lorenz, Ax_slider);
  Ay_slider->connect("ValueChanged", signalAy, p_lorenz, Ay_slider);
  Az_slider->connect("ValueChanged", signalAz, p_lorenz, Az_slider);

  auto label = tgui::Label::create();
  label->setText("Particles");
  label->setPosition("particle_slider.left - width - 10",
                     "particle_slider.top");
  label->setTextSize(14);
  gui.add(label, "particles_label");

  label = tgui::Label::create();
  label->setText("Trail");
  label->setPosition("trail_slider.left - width - 10", "trail_slider.top");
  label->setTextSize(14);
  gui.add(label, "trail_label");

  // "σ" "β" "ρ" "θ"  Dont display in GUI
  label = tgui::Label::create();
  label->setText("Sigma");
  label->setPosition("sigma_slider.left - width - 10", "sigma_slider.top");
  label->setTextSize(14);
  gui.add(label, "sigma_label");

  label = tgui::Label::create();
  label->setText("Beta");
  label->setPosition("beta_slider.left - width - 10", "beta_slider.top");
  label->setTextSize(14);
  gui.add(label, "beta_label");

  label = tgui::Label::create();
  label->setText("Rho");
  label->setPosition("rho_slider.left - width - 10", "rho_slider.top");
  label->setTextSize(14);
  gui.add(label, "rho_label");

  label = tgui::Label::create();
  label->setText("ThetaX");
  label->setPosition("Ax_slider.left - width - 10", "Ax_slider.top");
  label->setTextSize(14);
  gui.add(label, "Ax_label");

  label = tgui::Label::create();
  label->setText("ThetaY");
  label->setPosition("Ay_slider.left - width - 10", "Ay_slider.top");
  label->setTextSize(14);
  gui.add(label, "Ay_label");

  label = tgui::Label::create();
  label->setText("ThetaZ");
  label->setPosition("Az_slider.left - width - 10", "Az_slider.top");
  label->setTextSize(14);
  gui.add(label, "Az_label");
}

//respond to mouse wheel zoom
void zoom_view(sf::View &view, int delta, unsigned int origx,
               unsigned int origy) {
  static float current_zoom = 0.0;

  if (delta < 0) {
    // zoom in
    current_zoom -= 0.1;
    // cout << "zoom: " << current_zoom << endl;
    view.zoom(0.9);
    if (current_zoom < -2.0) {
      view.setSize(origx, origy);
      current_zoom = 0.0;
    }
  } else {
    // zoom out
    current_zoom += 0.1;
    // cout << "zoom: " << current_zoom << endl;
    view.zoom(1.1);
    if (current_zoom > 2.0) {
      view.setSize(origx, origy);
      current_zoom = 0.0;
    }
  }
}

int main() {
  // create the particle system (i.e. Model)
  //    note: we are passing this shared ptr to signals so they can change the model (MVC)
  auto p_lorenz = make_shared<LorenzSystem>(20, 1000);

  sf::Vector2u screenDimensions(1200, 1200);
  sf::RenderWindow window(sf::VideoMode(screenDimensions.x, screenDimensions.y),
                          "Lorenz Attractor");
  tgui::Gui gui{window};  // Create the gui and attach it to the window
  tgui::Theme theme{"themes/BabyBlue.txt"};
  tgui::Theme::setDefault(&theme);
  createGuiElements(gui, p_lorenz);

  //Main window
  sf::View small;
  sf::Vector2u viewD(140, 200);
  small.setCenter(0, 0);
  small.setSize(viewD.x, viewD.y);
  small.move(0, 0);
  window.setView(small);

  //Text and gui elements at bottom
  sf::View smaller;
  smaller.setCenter(400, 60);
  smaller.setSize(800, 120);
  smaller.setViewport(sf::FloatRect(0.f, 0.90f, 1.0f, 0.1f));

  // create a clock to track the elapsed time
  sf::Clock clock_s;  // start
  sf::Clock clock_e;  // elapsed between each draw cycle

  sf::Time start = clock_s.restart();
  sf::Text log;
  sf::Font font;
  if (!font.loadFromFile("font.ttf")) exit(-1);
  log.setFont(font);
  log.setCharacterSize(20);
  log.setFillColor(sf::Color::Red);
  // log.setStyle(sf::Text::Bold | sf::Text::Underlined);
  int frames = 0;
  bool saved_screenshot = false;

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) window.close(); //breaks out above

      //Zoom the whole sim if mouse wheel moved
      if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
          zoom_view(small, event.mouseWheelScroll.delta, viewD.x, viewD.y);
        }
      }

      gui.handleEvent(event);  // Pass the User slider changes event to the widgets
    }
    ++frames;

    sf::Time elapsed = clock_e.restart();
    p_lorenz->update(elapsed); //move the particles dt time forward

    start = clock_s.getElapsedTime();

    log.setString("secs: " + to_string(start.asSeconds()) + " vertices: " +
                  to_string(p_lorenz->getTrail() * p_lorenz->getParticles()) +
                  "\n" + "fps: " + to_string(frames / start.asSeconds()) +
                  " particles: " +
                  to_string(p_lorenz->getParticles() - p_lorenz->getEscaped()) +
                  "/" + to_string(p_lorenz->getParticles()) +
                  +" trail: " + to_string(p_lorenz->getTrail()) + "\n" +
                  "sigma: " + to_string(p_lorenz->getSigma()) +
                  " beta: " + to_string(p_lorenz->getBeta()) +
                  " rho: " + to_string(p_lorenz->getRho()) + "\n" +
                  "Ax: " + to_string(p_lorenz->getAx()) +
                  " Ay: " + to_string(p_lorenz->getAy()) +
                  " Az: " + to_string(p_lorenz->getAz()) + "\n" +
                  "Use Mouse Wheel to Zoom");

    window.clear();
    window.setView(small);
    window.draw(*p_lorenz); //draw particles, particle trails, and axes
    window.setView(smaller);
    window.draw(log); //draw text at bottom
    gui.draw();  // Draw all widgets

    //Save a screenshot after 50 seconds
    if ((saved_screenshot == false) && (start.asSeconds() > 50.0)) {
      sf::Vector2u windowSize = window.getSize();
      sf::Texture texture;
      texture.create(windowSize.x, windowSize.y);
      texture.update(window);
      sf::Image screenshot = texture.copyToImage();
      screenshot.saveToFile("lorentz_widgets.png");
      saved_screenshot = true;
    }

    window.display();
  }

  return 0;
}
