#pragma once
#include <chrono>
#include <string>
#include <vector>
struct SupportedFractal {
  std::string name;
  bool cuda_mode; 
  std::vector<double> xMinMax;  // x min max
  std::vector<double> yMinMax;  // y min max
  unsigned int color_scheme; //not yet used
  std::vector<unsigned int> max_iters; //not yet used
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
