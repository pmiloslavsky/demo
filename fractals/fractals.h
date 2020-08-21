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
  std::vector<double> xMinMax;  // x min max
  std::vector<double> yMinMax;  // y min max
  std::vector<unsigned int> max_iters;
  double power;
  std::complex<double> zconst;
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
