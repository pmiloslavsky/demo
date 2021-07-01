#pragma once
#include <chrono>
#include <string>
#include <vector>
#include <complex>
struct SupportedFractal {
  std::string name;
  bool cuda_mode;
  bool probabalistic; //i.e. like buddha - affects thread model
  bool julia;
  std::vector<double> xMinMax;  // default x min max
  std::vector<double> yMinMax;  // default min max
  std::vector<unsigned int> current_max_iters;
  std::vector<unsigned int> default_max_iters;
  double current_power;
  double default_power;
  std::complex<double> current_zconst;
  std::complex<double> default_zconst;
};

struct SampleStats {
  unsigned long long rejected; // skipInSet check
  unsigned long long in_set;
  unsigned long long escaped_set;
  unsigned long long total;
  //stats for thread efficiency and total thread progress
  unsigned long long samples_per_second; //i.e. "total" - not hits
  std::chrono::time_point<std::chrono::steady_clock> next_second_start;
  unsigned long long samples_last_second;
};
