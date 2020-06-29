//Copyright 2020 Philip Miloslavsky
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <execution>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <numeric>
#include <random>
#include <tuple>
#include <vector>
#include <thread>

using namespace std;

#define WHITE_ESCAPE ("\e[1;37m")
#define RED_ESCAPE ("\e[1;31m")
#define BLUE_ESCAPE ("\e[1;34m")
#define GREEN_ESCAPE ("\e[1;32m")
#define YEL_ESCAPE (static_cast<string>("\e[1;33m"))
#define RED_BLINK_ESCAPE ("\e[5;31m")
#define BLUE_BLINK_ESCAPE ("\e[5;34m")
#define GREEN_BLINK_ESCAPE ("\e[5;32m")
#define RESET_ESCAPE (static_cast<string>("\e[0m"))

const long unsigned int num_elements = 100'000'000;
const long unsigned int min_num_elements = num_elements / pow(2, 12);
double time_results[12][4];  // no policy, seq, par, par_unseq
const auto policies =
    make_tuple(0, execution::seq, execution::par, execution::par_unseq);
const vector<string> policies_string{(YEL_ESCAPE + "(no_policy)  " + RESET_ESCAPE),
                                     (YEL_ESCAPE + "(seq      )  " + RESET_ESCAPE),
                                     (YEL_ESCAPE + "(par      )  " + RESET_ESCAPE),
                                     (YEL_ESCAPE + "(par_unseq)  " + RESET_ESCAPE)};
enum c_execution_policy { NO_POLICY = 0, SEQ, PAR, PAR_UNSEQ };
const int num_iterations = 1;
const bool debug = false;



string pr_algo_name(const string &algo_name) {
  return (GREEN_ESCAPE + algo_name + RESET_ESCAPE);
}

string pr_policy_name(const string &policy_name) {
  return (YEL_ESCAPE + policy_name + RESET_ESCAPE);
}
    

//Analyze the results for each execution policy and for num of element in vector
//raise alerts if the execution policies are not working
void print_time_results(double (&time_results)[12][4], string algo_name) {
  cout << endl << algo_name << " times in ms:" << endl;
  cout << "execution policy:" << endl;
  //setw doesnt work on escape sequences
  cout << left << setw(13) << policies_string[NO_POLICY];
  cout << left << setw(13) << policies_string[SEQ];
  cout << left << setw(13) << policies_string[PAR];
  cout << left << setw(13) << policies_string[PAR_UNSEQ] << endl;

  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 4; j++) {
      cout << fixed << left << setw(12) << time_results[i][j] << " ";
    }

    cout << "ms"
         << "  num_elements: " << right << setw(11)
         << static_cast<long unsigned int>(num_elements / pow(2, i));
    if (time_results[i][NO_POLICY] < time_results[i][SEQ])
      cout << RED_ESCAPE << "  (no policy) faster than (seq) by "
           << setprecision(0) << fixed
           << 100.0 *
                  (time_results[i][SEQ] / time_results[i][NO_POLICY] - 1)
           << "\%" << RESET_ESCAPE;
    if (time_results[i][SEQ] < time_results[i][PAR])
      cout << RED_ESCAPE << "  (seq) faster than (par) by " << setprecision(0)
           << fixed << 100.0 * (time_results[i][PAR] / time_results[i][SEQ] - 1)
           << "\%" << RESET_ESCAPE;
    if (time_results[i][PAR] < time_results[i][PAR_UNSEQ])
      cout << RED_ESCAPE << "  (par) faster than (par_unseq) by "
           << setprecision(0) << fixed
           << 100.0 * (time_results[i][PAR_UNSEQ] / time_results[i][PAR] - 1)
           << "\%" << RESET_ESCAPE;
    cout << setprecision(5) << endl;
  }
}

int main() {

  // Its prob possible to encapsulate these test generically
  // but we would need setup and teardown for tracking sanity check stuff

  cout << "Machine supports "<< thread::hardware_concurrency() << " simultaneous threads" << endl;
  //
  // vector reduce
  //
  string algo_name = pr_algo_name("reduce(accumulate int)");
  cout << endl
       << "Timing " << algo_name
       << " vs num_elements and execution policies:"
       << endl
       << endl;

  int result_ix = 0;
  // vector reduce
  for (long unsigned int ne = num_elements; ne > min_num_elements;
       ne = ne / 2) {
    vector<int> v(ne, 1);
    if (debug) {
      cout << endl
           << "Comparing " << v.size()
           << " element vector accumulate for different execution policies:"
           << endl
           << endl;
    }
    for (long unsigned int policy = 0;
         policy < tuple_size<decltype(policies)>::value; policy++) {
      auto start = chrono::high_resolution_clock::now();
      int result = 0;
      switch (policy)  // C++ sucks - maybe fold expressions?
      {
        case 0:
          result = reduce(v.begin(), v.end(), 0.0);
        case 1:
          result = reduce(get<1>(policies), v.begin(), v.end(), 0.0);
        case 2:
          result = reduce(get<2>(policies), v.begin(), v.end(), 0.0);
        case 3:
          result = reduce(get<3>(policies), v.begin(), v.end(), 0.0);
      }

      auto end = chrono::high_resolution_clock::now();
      chrono::duration<double, std::milli> time = end - start;
      if (debug) {
        cout << fixed << algo_name << " " << policies_string[policy]
             << " took " << time.count() << "ms" << endl;
        cout << " sum = " << result << endl;
      }
      time_results[result_ix][policy] = time.count();
    }
    result_ix++;
  }

  print_time_results(time_results, algo_name);

  //
  // vector sort
  //
  algo_name = pr_algo_name("sort(double)");
  cout << endl
       << "Timing " << algo_name
       << " vs num_elements and execution policies:"
       << endl
       << endl;
  

  cout << "starting create vector of " << num_elements << " elements" << endl;
  random_device rd;
  vector<double> doubles(num_elements);
  for (auto& d : doubles) {
    d = static_cast<double>(rd());
  }
  cout << "finished creating vector of " << num_elements << " elements" << endl;

  result_ix = 0;
  // vector sort
  for (long unsigned int ne = num_elements; ne > min_num_elements; ne = ne / 2) {
    vector<double> v(doubles.begin(), ne + doubles.begin());
    if (debug) {
      cout << endl
           << "Comparing " << v.size()
           << " element vector " << algo_name << " for different execution policies:" << endl
           << endl;
    }
    for (long unsigned int policy = 0; policy < tuple_size<decltype(policies)>::value; policy++) {
      vector<double> sorted(v);
      auto start = chrono::high_resolution_clock::now();
      switch (policy)  // C++ sucks - maybe fold expressions?
      {
        case 0:
          sort(sorted.begin(), sorted.end());
        case 1:
          sort(get<1>(policies), sorted.begin(), sorted.end());
        case 2:
          sort(get<2>(policies), sorted.begin(), sorted.end());
        case 3:
          sort(get<3>(policies), sorted.begin(), sorted.end());
      }

      auto end = chrono::high_resolution_clock::now();
      chrono::duration<double, std::milli> time = end - start;
      if (debug) {
        cout << fixed << algo_name << " " << policies_string[policy]
             << " took " << time.count() << "ms" << endl;
        cout << " front():" << sorted.front() << " back():" << sorted.back()
             << endl;
      }
      time_results[result_ix][policy] = time.count();
      cout << "." << flush;
    }
    result_ix++;
  }
  cout << endl;

  print_time_results(time_results, "sort");

  //
  // Bugs with optimization
  //

  vector<int> v(num_elements, 1);
  cout << endl
       << "Demonstrating issues with unparellizable and unvectorizable code:"
       << endl
       << endl;
  cout << "Baseline:" << endl;
  algo_name = pr_algo_name("accumulate(ints)");
  cout << "correct " << policies_string[NO_POLICY] << algo_name << " result:" << endl;
  int sum = 0;
  {
    auto start = chrono::high_resolution_clock::now();
    sum = 0;
    sum = accumulate(v.begin(), v.end(), 0.0);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << "sum = " << sum << " " << fixed << algo_name << policies_string[NO_POLICY] << " took " << time.count() << "ms"
         << endl;
  }
  algo_name = pr_algo_name("reduce(ints)");
  cout << "correct " << policies_string[NO_POLICY] << algo_name << " result:" << endl;
  {
    auto start = chrono::high_resolution_clock::now();
    sum = 0;
    sum = reduce(v.begin(), v.end(), 0.0);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << "sum = " << sum << " " << fixed << algo_name << policies_string[NO_POLICY] << " took " << time.count() << "ms"
         << endl;
  }
  cout << "correct " << policies_string[SEQ] << algo_name << " result:" << endl;
  {
    auto start = chrono::high_resolution_clock::now();
    sum = 0;
    sum = reduce(get<1>(policies), v.begin(), v.end(), 0.0);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << "sum = " << sum << " " << fixed << algo_name << policies_string[SEQ] << " took " << time.count() << "ms"
         << endl;
  }
    cout << "correct " << policies_string[PAR_UNSEQ] << algo_name << " result:" << endl;
  {
    auto start = chrono::high_resolution_clock::now();
    sum = 0;
    sum = reduce(get<3>(policies), v.begin(), v.end(), 0.0);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << "sum = " << sum << " " << fixed << algo_name << policies_string[PAR_UNSEQ] << " took " << time.count() << "ms"
         << endl;
  }

  cout << "End Baseline:" << endl << endl;

  algo_name = pr_algo_name("for_each(sum ints)");
  cout << "correct: ";
  {
    auto start = chrono::high_resolution_clock::now();
    sum = 0;
    for_each(begin(v), end(v),
             [&](int i) { sum += i; });
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << "sum = " << sum << " " << fixed << algo_name << policies_string[NO_POLICY] << " took " << time.count() << "ms"
         << endl;
  }
  {
    auto start = chrono::high_resolution_clock::now();
    sum = 0;
    for_each(execution::seq, begin(v), end(v),
             [&](int i) { sum += i; });
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << "sum = " << sum << " " << fixed << algo_name << policies_string[SEQ] << " took " << time.count() << "ms"
         << endl;
  }
  cout << "incorrect result, data race:" << endl;
  // below 2 are broken - data race
  {
    auto start = chrono::high_resolution_clock::now();
    sum = 0;
    for_each(execution::par, begin(v), end(v),
             [&](int i) { sum += i; });
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << "sum = " << sum << " " << fixed << algo_name << policies_string[PAR] << " took " << time.count() << "ms"
         << endl;
  }
  {
    auto start = chrono::high_resolution_clock::now();
    sum = 0;
    for_each(execution::par, begin(v), end(v),
             [&](int i) { sum += i; });
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << "sum = " << sum << " " << fixed << algo_name << policies_string[PAR] << " took " << time.count() << "ms"
         << endl;
  }

  cout << "correct result(mutex protected):" << endl;
  {
    auto start = chrono::high_resolution_clock::now();
    sum = 0;
    mutex m;
    for_each(execution::par, begin(v), end(v), [&](int i) {
      std::lock_guard<mutex> lock{m};
      sum += i;
    });
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << "sum = " << sum << " " << fixed << algo_name << policies_string[PAR] << "with mutex took " << time.count() << "ms"
         << endl;

  }

  cout << "correct but may deadlock due to vectorization of mutex:"
       << endl;
  // this is actually supposed to deadlock if the compiler does the wrong thing
  // and does 2 locks and 2 unlocks in different orders
  // i havent seen that happen though
  {
    auto start = chrono::high_resolution_clock::now();
    sum = 0;
    mutex m;
    for_each(execution::par_unseq, begin(v), end(v), [&](int i) {
      std::lock_guard<mutex> lock{m};
      sum += i;
    });
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << "sum = " << sum << " " << fixed << algo_name << policies_string[PAR_UNSEQ] << "with mutex took " << time.count() << "ms"
         << endl;
  }

  cout << "correct vectorization safe using atomic:" << endl;
  {
    auto start = chrono::high_resolution_clock::now();
    atomic<int> sum = 0;
    for_each(execution::par_unseq, begin(v), end(v),
             [&](int i) { sum.fetch_add(i, std::memory_order_relaxed); });
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << "sum = " << sum << " " << fixed << algo_name << policies_string[PAR_UNSEQ] << "with atomic took " << time.count() << "ms"
         << endl;
  }
  cout << endl
       << "what we just learned is the parallel version of " << endl
       << algo_name << " can not be made better than seq on my hw" << endl
       << "also strangely enough its more optimized than reduce even though reduce API is simpler" << endl
       << "We also saw that (seq) can be slower than (no policy) by 2x for reduce" << endl
       << "Don't assume anything! Measure!" << endl;

  return 0;
}
