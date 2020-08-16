#pragma once
#include <vector>
#include "fractals.h"

//if it returns zero, you have no cuda devices
int cuda_info(void);

//basic test program
int cuda_vec_add(unsigned int w, unsigned int h);

//a prototype of stuff similar to what i need for buddhabrot
int cuda_generate_hits_no_fractal(unsigned int w, unsigned int h);

//The actual nebulabrot implementation
int cuda_generate_buddhabrot_hits(unsigned int w, unsigned int h,
                                  SupportedFractal & frac,
				  SampleStats & stats,
                                  std::vector<std::vector<long long unsigned int>> &redHits,
                                  std::vector<std::vector<long long unsigned int>> &greenHits,
                                  std::vector<std::vector<long long unsigned int>> &blueHits);

struct cuda_kernel_stats {
  unsigned long long rejected; // skipInSet check
  unsigned long long in_set;
  unsigned long long escaped_set;
  unsigned long long total;
};
