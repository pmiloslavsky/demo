#include "buddha_cuda_kernel.h"
#include <chrono>
#include <cmath>
#include <complex>
#include <iostream>
#include <random>
#include <string>
#include <utility>

#include <curand_kernel.h>
#include <ctime>
#include <cuComplex.h>

//emacs M-X c++-mode

//With cuda we need to check return codes often
#define PRINT_ON_SUCCESS (0)

void checkError(cudaError_t code, char const * func, const char *file, const int line, bool abort)
{
    if (code != cudaSuccess) 
    {
        const char * errorMessage = cudaGetErrorString(code);
        fprintf(stderr, "CUDA error returned from \"%s\" at %s:%d, Error code: %d (%s)\n", func, file, line, code, errorMessage);
        if (abort){
            cudaDeviceReset();
            exit(code);
        }
    }
    else if (PRINT_ON_SUCCESS)
    {
        const char * errorMessage = cudaGetErrorString(code);
        fprintf(stderr, "CUDA error returned from \"%s\" at %s:%d, Error code: %d (%s)\n", func, file, line, code, errorMessage);
    }
}
 
void checkLastError(char const * func, const char *file, const int line, bool abort)
{
    cudaError_t code = cudaGetLastError();
    if (code != cudaSuccess)
    {
        const char * errorMessage = cudaGetErrorString(code);
        fprintf(stderr, "CUDA error returned from \"%s\" at %s:%d, Error code: %d (%s)\n", func, file, line, code, errorMessage);
        if (abort) {
            cudaDeviceReset();
            exit(code);
        }
    }
    else if (PRINT_ON_SUCCESS)
    {
        const char * errorMessage = cudaGetErrorString(code);
        fprintf(stderr, "CUDA error returned from \"%s\" at %s:%d, Error code: %d (%s)\n", func, file, line, code, errorMessage);
    }
}



// To be used around calls that return an error code, ex. cudaDeviceSynchronize or cudaMallocManaged
void checkError(cudaError_t code, char const * func, const char *file, const int line, bool abort = true);
#define checkCUDAError(val) { checkError((val), #val, __FILE__, __LINE__); }    // in-line regular function
#define checkCUDAError2(val) check((val), #val, __FILE__, __LINE__) // typical macro 
 
// To be used after calls that do not return an error code, ex. kernels to check kernel launch errors
void checkLastError(char const * func, const char *file, const int line, bool abort = true);
#define checkLastCUDAError(func) { checkLastError(func, __FILE__, __LINE__); }
#define checkLastCUDAError_noAbort(func) { checkLastError(func, __FILE__, __LINE__, 0); }
 
using namespace std;

void cudaPrintDeviceProperties(cudaDeviceProp & devProp) {
    printf("Major revision number:         %d\n",  devProp.major);
    printf("Minor revision number:         %d\n",  devProp.minor);
    printf("Name:                          %s\n",  devProp.name);
    printf("Total global memory:           %lu\n",  devProp.totalGlobalMem);
    printf("Total shared memory per block: %lu\n",  devProp.sharedMemPerBlock);
    printf("Total registers per block:     %d\n",  devProp.regsPerBlock);
    printf("Warp size:                     %d\n",  devProp.warpSize);
    printf("Maximum memory pitch:          %lu\n",  devProp.memPitch);
    printf("Maximum threads per block:     %d\n",  devProp.maxThreadsPerBlock);
    for (int i = 0; i < 3; ++i)
    printf("Maximum dimension %d of block:  %d\n", i, devProp.maxThreadsDim[i]);
    for (int i = 0; i < 3; ++i)
    printf("Maximum dimension %d of grid:   %d\n", i, devProp.maxGridSize[i]);
    printf("Clock rate:                    %d\n",  devProp.clockRate);
    printf("Total constant memory:         %lu\n",  devProp.totalConstMem);
    printf("Texture alignment:             %lu\n",  devProp.textureAlignment);
    printf("Concurrent copy and execution: %s\n",  (devProp.deviceOverlap ? "Yes" : "No"));
    printf("Number of multiprocessors:     %d\n",  devProp.multiProcessorCount);
    printf("Kernel execution timeout:      %s\n",  (devProp.kernelExecTimeoutEnabled ? "Yes" : "No"));
}

int cuda_info() {
  int nDevices = 0;
  cudaGetDeviceCount(&nDevices);

  if (nDevices == 0) {
    cout << "No CUDA Found" << endl;
  }
  else
  {
    for (int i = 0; i < nDevices; i++) {
      cudaDeviceProp prop;
      cudaGetDeviceProperties(&prop, i);
      cudaPrintDeviceProperties(prop);
    }
  }

  return nDevices;
}


__global__ void vec_add_kernel(float *a, float *b, float *c, int n) {
    int i = threadIdx.x + blockDim.x * blockIdx.x;
    if (i < n) c[i] = a[i] + b[i];
}

//Simple Test to see things are working
int cuda_vec_add(unsigned int w, unsigned int h) {
  cout << "CUDA Test: vector add" << endl;
  const int n = w*h;

  vector<float> h_a(n, 1.1);
  vector<float> h_b(n, 2.2);
  vector<float> h_c(n, 0.0);

  float *d_a, *d_b, *d_c;
  cudaMalloc(&d_a, n*sizeof(float));
  cudaMalloc(&d_b, n*sizeof(float));
  cudaMalloc(&d_c, n*sizeof(float));

  cudaMemcpy(d_a, &h_a[0], n*sizeof(float), cudaMemcpyHostToDevice);
  cudaMemcpy(d_b, &h_b[0], n*sizeof(float), cudaMemcpyHostToDevice);

  vec_add_kernel<<<((n-1)*256)/256 + 1,256>> >(d_a, d_b, d_c, n);

  cudaMemcpy(&h_c[0], d_c, n*sizeof(float), cudaMemcpyDeviceToHost);

  cudaFree(d_a); cudaFree(d_b); cudaFree(d_c);

  cout << "First and last elements of vector should be 3.3: " << h_c[0] << " " << h_c[w*h - 1] << endl;
  
  return 0;
}


#if 0
__device__ bool skipInSettest(complex<double> sample) {
    if ((abs(sample - complex<double>(-1, 0)) < 0.25) ||
        (abs(1.0 - sqrt(1.0 - 4.0 * sample))) < 1.0)
      return true;
    return false;
  }
#endif


//prototype of what I will actually need
//2D array on device is one block of memory
__global__ void generate_hits_prototype_kernel(unsigned long long *rH, int w, int h) {
  int i = threadIdx.x + blockDim.x * blockIdx.x; //dim is 1 and tix is 1
  
  if (i >= h*w)
    return;

  //curandState state;
  curandStateMRG32k3a state;
  curand_init((unsigned long long)clock() + i, 0, 0, &state);

  //generate enough samples to overcome cuda overhead
  for (int sample_ix = 0; sample_ix < 1000; ++sample_ix)
  {
    double pr = curand_uniform_double (&state);
    double pi = curand_uniform_double (&state);

    //scale the random numbers
    pr = -2 + 3.0 *pr;
    pi = -1 + 2.0 *pi;

    cuDoubleComplex p = make_cuDoubleComplex(pr, pi);
    cuDoubleComplex c = cuCmul(p, p);

    //STL doesnt compile on devices
    //complex<double> sample(0.0,0.0);
    //if (true == skipInSettest(sample))
    //  printf("complex works");

    //do some rejection testing sort of like what we need for fractals
    
    c = cuCadd(cuCmul(c, c), p);

    if (cuCabs(c) < 0.5)
    {
      //See which box to put the hit in
      double minx = -2.0;
      double maxx = 1.0;
      double miny = -1.0;
      double maxy = 1.0;
      if ((cuCreal(c) <= maxx) && (cuCreal(c) >= minx) && (cuCimag(c) <= maxy) &&
          (cuCimag(c) >= miny)) {
        int x = ((cuCreal(c) - minx) * h) /
            (maxx - minx);
        int y = ((cuCimag(c) - miny) * w) /
            (maxy - miny);
	int ii = x + w*y;

	//printf("%d %f %f\n",i,cuCreal(c),cuCimag(c));
	if (ii < h*w)
	  atomicAdd(&rH[x + w*y], 1); //need atomics here -> in practice collisions should be very rare
      }
    }
    else
      continue;

  }
}

int cuda_generate_hits_prototype(unsigned int w, unsigned int h)
{
  cout << "CUDA Test: generate buddhabrot hits prototype" << endl;
  auto start = chrono::high_resolution_clock::now();
  vector<unsigned long long> redHits;

  redHits.resize(w*h,0);



  //Try several modes of parallelization
  //Mode 1
  unsigned long long * drH;

  checkCUDAError(cudaMalloc(&drH, w*h*sizeof(redHits[0])));
  
  //copy from host to cuda memory
  
  checkCUDAError(cudaMemcpy(drH, &redHits[0], w*h*sizeof(redHits[0]), cudaMemcpyHostToDevice));
  
  generate_hits_prototype_kernel<<<256,256>>>(drH, w, h);
  
  checkLastCUDAError_noAbort("kernelA");		   
  
  checkCUDAError(cudaMemcpy(&redHits[0], drH, w*h*sizeof(redHits[0]), cudaMemcpyDeviceToHost));
     
  checkCUDAError(cudaFree(drH));
  
  auto end = chrono::high_resolution_clock::now();
  //cout << "sample time " << tix << " " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl; 

   unsigned long long  hitsum = 0;
   for (auto e: redHits) {
       hitsum += e; }

   auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
   cout << "cuda generated pseudo hits using execution mode <<<256,256>>>: " << hitsum << " in " << duration <<
     " ms. hits per second: " <<  1000*hitsum/duration << endl;

   //Mode 2
   checkCUDAError(cudaMalloc(&drH, w*h*sizeof(redHits[0])));
  
   //copy from host to cuda memory
   
   checkCUDAError(cudaMemcpy(drH, &redHits[0], w*h*sizeof(redHits[0]), cudaMemcpyHostToDevice));
  
   generate_hits_prototype_kernel<<<256*256,1>>>(drH, w, h);
   
   checkLastCUDAError_noAbort("kernelA");		   
   
   checkCUDAError(cudaMemcpy(&redHits[0], drH, w*h*sizeof(redHits[0]), cudaMemcpyDeviceToHost));
   
   checkCUDAError(cudaFree(drH));
  
   end = chrono::high_resolution_clock::now();
   //cout << "sample time " << tix << " " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl; 
   
   hitsum = 0;
   for (auto e: redHits) {
     hitsum += e; }

   duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
   cout << "cuda generated pseudo hits using execution mode <<<256*256,1>>>: " << hitsum << " in " << duration <<
       " ms. hits per second: " <<  1000*hitsum/duration << endl;

   //Mode 3 Doesnt work
   // checkCUDAError(cudaMalloc(&drH, w*h*sizeof(redHits[0])));
  
   // //copy from host to cuda memory
   
   // checkCUDAError(cudaMemcpy(drH, &redHits[0], w*h*sizeof(redHits[0]), cudaMemcpyHostToDevice));
  
   // generate_hits_prototype_kernel<<<1,256*256>>>(drH, w, h);
   
   // checkLastCUDAError_noAbort("kernelA");		   
   
   // checkCUDAError(cudaMemcpy(&redHits[0], drH, w*h*sizeof(redHits[0]), cudaMemcpyDeviceToHost));
   
   // checkCUDAError(cudaFree(drH));
  
   // end = chrono::high_resolution_clock::now();
   // //cout << "sample time " << tix << " " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl; 
   
   // hitsum = 0;
   // for (auto e: redHits) {
   //   hitsum += e; }

   // duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
   // cout << "cuda generated red hits for <<<1,256*256>>>: " << hitsum << " in " << duration <<
   //     " ms. hits per second: " <<  1000*hitsum/duration << endl;
   
   return 0;
   
}


//Helper functions for buddhabrot
__device__ void generate_buddhabrot_trail_cuda(const cuDoubleComplex &c, unsigned int iters_max,
                                               cuDoubleComplex * p_trail, unsigned int &trail_len, unsigned long long & in, unsigned long long & out) {
  unsigned int iter_ix = 0;
  cuDoubleComplex z = make_cuDoubleComplex(0.0, 0.0);

  trail_len=0;

  while (iter_ix < iters_max && cuCabs(z) < 2.0) {
    z = cuCadd(cuCmul(z,z), c);

    p_trail[iter_ix] = z;
    ++iter_ix;
  }

  // If point is in the set we wont use it to color
  if (iter_ix == iters_max) {
    ++in;
    trail_len=0;
  }
  else
  {
    ++out;
    trail_len = iter_ix;
  }
  // return trail
}

__device__ bool skipInSet_cuda(cuDoubleComplex sample) {
  // if ((abs(sample - complex<double>(-1, 0)) < 0.25) ||
  //     (abs(1.0 - sqrt(1.0 - 4.0 * sample))) < 1.0)
  //Need equivalent math in cuda TODO missing sqrt
  if (cuCabs(cuCsub(sample,make_cuDoubleComplex(-1, 0))) < 0.25)
      return true;
    return false;
}

__device__ void saveBuddhabrotTrailToColor_cuda(cuDoubleComplex * p_trail, const unsigned int &trail_len, int w, int h,
                                                unsigned long long * p_hits, double minx, double maxx, double miny, double maxy) {
  int max_ix = w*h;

  for (int i = 0; i < trail_len; ++i) {
    // if point is plottable, scale it to be on a pixel and increment the
    // value for the pixel
    cuDoubleComplex c = p_trail[i];
    if ((cuCreal(c) <= maxx) && (cuCreal(c) >= minx) && (cuCimag(c) <= maxy) &&
        (cuCimag(c) >= miny)) {
      //depending on the cast here you might get a faint gridline in your image
      //so be careful
      int x = ((cuCreal(c) - minx) * w) /
          (maxx - minx);
      int y = ((cuCimag(c) - miny) * h) /
          (maxy - miny);
      
      int ix = x+y*w;
      
      //check for overrun
      if (ix < max_ix)
        atomicAdd(&p_hits[ix], 1);
    }
  }
}

//The actual kernel our fractals program uses
//2D array on device is one block of memory
__global__ void generate_hits_kernel(unsigned long long *rH, unsigned long long *gH, unsigned long long *bH, unsigned long long * p_stats,
                                     int w, int h, 
                                     double minx, double maxx, double miny, double maxy,
                                     int red_max, int green_max, int blue_max) {
  int i = threadIdx.x + blockDim.x * blockIdx.x; //dim is 1 and tix is 1
  
  if (i >= h*w)
    return;

  cuda_kernel_stats local_stats={0,0,0,0};

  curandState state;
  curand_init((unsigned long long)clock() + i, 0, 0, &state);


  unsigned long long max_samples = 100; //large enough to overcome thread sleep time and cuda overhead
  for (int sample_ix = 0; sample_ix < max_samples; ++sample_ix)
  {
    double pr = curand_uniform_double (&state);
    double pi = curand_uniform_double (&state);

    local_stats.total++;

    //scale the random sample    
    pr = minx + pr*(maxx - minx);
    pi = miny + pi*(maxy - miny);

    cuDoubleComplex sample = make_cuDoubleComplex(pr, pi);

    if (true == skipInSet_cuda(sample))
    {
      local_stats.rejected++;
      continue;
    }

    //We need memory to hold the escape trail - could be a problem
#define MAX_ITERS_CUDA (10000)
    cuDoubleComplex trail[MAX_ITERS_CUDA];
    unsigned int trail_len = 0;

    if ((red_max > MAX_ITERS_CUDA) || (green_max > MAX_ITERS_CUDA) || (blue_max > MAX_ITERS_CUDA))
    {
      printf("CUDA: Max iteration count not supported: %d %d %d\n", red_max, green_max, blue_max);
      return;;
    }
    

    generate_buddhabrot_trail_cuda(sample, red_max,
                                   &trail[0], trail_len, local_stats.in_set, local_stats.escaped_set);
    saveBuddhabrotTrailToColor_cuda(&trail[0], trail_len, w, h,
                                    rH, minx, maxx, miny, maxy);
    if (trail_len != 0) {
      sample = make_cuDoubleComplex(cuCreal(sample),-cuCimag(sample));
      generate_buddhabrot_trail_cuda(sample, red_max,
                                     &trail[0], trail_len, local_stats.in_set, local_stats.escaped_set);
      saveBuddhabrotTrailToColor_cuda(&trail[0], trail_len, w, h,
                                      rH, minx, maxx, miny, maxy);

    }

    generate_buddhabrot_trail_cuda(sample, green_max,
                                   &trail[0], trail_len, local_stats.in_set, local_stats.escaped_set);
    saveBuddhabrotTrailToColor_cuda(&trail[0], trail_len, w, h,
                                    gH, minx, maxx, miny, maxy);
    if (trail_len != 0) {
      sample = make_cuDoubleComplex(cuCreal(sample),-cuCimag(sample));
      generate_buddhabrot_trail_cuda(sample, green_max,
                                     &trail[0], trail_len, local_stats.in_set, local_stats.escaped_set);
      saveBuddhabrotTrailToColor_cuda(&trail[0], trail_len, w, h,
                                      gH, minx, maxx, miny, maxy);

    }

    generate_buddhabrot_trail_cuda(sample, blue_max,
                                   &trail[0], trail_len, local_stats.in_set, local_stats.escaped_set);
    saveBuddhabrotTrailToColor_cuda(&trail[0], trail_len, w, h,
                                    bH, minx, maxx, miny, maxy);
    if (trail_len != 0) {
      sample = make_cuDoubleComplex(cuCreal(sample),-cuCimag(sample));
      generate_buddhabrot_trail_cuda(sample, blue_max,
                                     &trail[0], trail_len, local_stats.in_set, local_stats.escaped_set);
      saveBuddhabrotTrailToColor_cuda(&trail[0], trail_len, w, h,
                                      bH, minx, maxx, miny, maxy);

    }
  }

  // now update p_stats for all samples for all kernels
  atomicAdd(&p_stats[0], local_stats.rejected);
  atomicAdd(&p_stats[1], local_stats.in_set);
  atomicAdd(&p_stats[2], local_stats.escaped_set);
  atomicAdd(&p_stats[3], local_stats.total);
}


//The main app will have one thread that will:
//1) Check for a future asking for thread termination
//2) Spawn a cuda kernel that takes about a second to run
//3) move the resulting Hits to the model under mutex
//4) back to 1)
//input: xyrange of fractal   pixel w and h  color max iterations
//output: stats

//kernel: move zeroed hits from host to device, skipInSet, generate trail, save trail in hits,
//move hits from device to host

int cuda_generate_buddhabrot_hits(unsigned int w, unsigned int h, SupportedFractal &frac,
				  SampleStats & stats,
                                  vector<vector<long long unsigned int>> &redHits,
                                  vector<vector<long long unsigned int>> &greenHits,
                                  vector<vector<long long unsigned int>> &blueHits)
{
  //spawn a kernel that takes ~ a second to run

  //cout << "CUDA Main: generate buddhabrot hits of all 3 colors" << endl;
  auto start = chrono::high_resolution_clock::now();

  //Zero out passed in hits
  redHits.resize(0);
  greenHits.resize(0);
  blueHits.resize(0);


  //Create 1D arrays of hits for cuda (its not good with C++ 2D vector<vector<>> style)
  vector<unsigned long long> rH;
  rH.resize(w*h,0);
  vector<unsigned long long> gH;
  gH.resize(w*h,0);
  vector<unsigned long long> bH;
  bH.resize(w*h,0);

  cuda_kernel_stats cuda_stats;


  unsigned long long * drH;
  unsigned long long * dgH;
  unsigned long long * dbH;

  unsigned long long * dstats;

  checkCUDAError(cudaMalloc(&drH, w*h*sizeof(unsigned long long)));
  checkCUDAError(cudaMalloc(&dgH, w*h*sizeof(unsigned long long)));
  checkCUDAError(cudaMalloc(&dbH, w*h*sizeof(unsigned long long)));
  checkCUDAError(cudaMalloc(&dstats, 4*sizeof(unsigned long long)));
  
  //copy from host to cuda memory
  
  checkCUDAError(cudaMemcpy(drH, &rH[0], w*h*sizeof(unsigned long long), cudaMemcpyHostToDevice));
  checkCUDAError(cudaMemcpy(dgH, &gH[0], w*h*sizeof(unsigned long long), cudaMemcpyHostToDevice));
  checkCUDAError(cudaMemcpy(dbH, &bH[0], w*h*sizeof(unsigned long long), cudaMemcpyHostToDevice));

  //64,256 is pretty fast
  generate_hits_kernel<<<32,256>>>(drH, dgH, dbH, dstats,
				  w, h,
				  frac.xMinMax[0], frac.xMinMax[1], frac.yMinMax[0], frac.yMinMax[1],
				  frac.max_iters[0], frac.max_iters[1], frac.max_iters[2]);
  
  checkLastCUDAError_noAbort("kernel for buddhabrot");		   
  
  checkCUDAError(cudaMemcpy(&rH[0], drH, w*h*sizeof(unsigned long long), cudaMemcpyDeviceToHost));
  checkCUDAError(cudaMemcpy(&gH[0], dgH, w*h*sizeof(unsigned long long), cudaMemcpyDeviceToHost));
  checkCUDAError(cudaMemcpy(&bH[0], dbH, w*h*sizeof(unsigned long long), cudaMemcpyDeviceToHost));

  checkCUDAError(cudaMemcpy(&cuda_stats, dstats, 4*sizeof(unsigned long long), cudaMemcpyDeviceToHost));
     
  checkCUDAError(cudaFree(drH));
  checkCUDAError(cudaFree(dgH));
  checkCUDAError(cudaFree(dbH));
  checkCUDAError(cudaFree(dstats));

  stats.total = cuda_stats.total;
  stats.rejected = cuda_stats.rejected;
  stats.in_set = cuda_stats.in_set;
  stats.escaped_set = cuda_stats.escaped_set;

  //cout << "cuda total/rejected/in_set/escaped_set " << stats.total << "/" << stats.rejected << "/" << stats.in_set << "/" << stats.escaped_set << endl;
  
  auto end = chrono::high_resolution_clock::now();
  //cout << "sample time " << tix << " " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl; 

  unsigned long long  hitsumr = 0;
  unsigned long long  hitsumg = 0;
  unsigned long long  hitsumb = 0;
  for (auto e: rH) {
    hitsumr += e; }
  for (auto e: gH) {
    hitsumg += e; }
  for (auto e: bH) {
    hitsumb += e; }

  auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
  //cout << "cuda generated rgb hits for <<<4*256,128>>>: " << hitsumr << " " << hitsumg << " " << hitsumb << " in " << duration <<
  //  " ms.   hits per second: " <<  1000*(hitsumr+hitsumg+hitsumb)/duration << endl;




   //Copy the 3 cuda 1D arrays into the user provided 2D arrays
   // width and heigh may be switched
   // The need for this code probably shows we need to redo our arrays
   redHits.resize(w);
   for (auto &v : redHits) v.resize(h);
   for (int i = 0; i < w; ++i)
   {
     for (int j = 0; j < h; ++j) {
       redHits[i][j] = rH[i+j*w];
     }
   }
   
   greenHits.resize(w);
   for (auto &v : greenHits) v.resize(h);
   for (int i = 0; i < w; ++i)
   {
     for (int j = 0; j < h; ++j) {
       greenHits[i][j] = gH[i+j*w];
     }
   }
   
   blueHits.resize(w);
   for (auto &v : blueHits) v.resize(h);
      for (int i = 0; i < w; ++i)
   {
     for (int j = 0; j < h; ++j) {
       blueHits[i][j] = bH[i+j*w];
     }
   }

   auto realend = chrono::high_resolution_clock::now();
   duration = chrono::duration_cast<chrono::milliseconds>(realend - end).count();
   //cout << "1D -> 2D array copy: " << duration << " ms" << endl;

  return 0;
}
