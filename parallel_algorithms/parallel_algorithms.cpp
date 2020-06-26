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

using namespace std;

const long unsigned int num_elements = 100'000'000;
const long unsigned int min_num_elements = num_elements / pow(2, 12);
double time_results[12][4];  // no policy, seq, par, par_unseq
const auto policies =
    make_tuple(0, execution::seq, execution::par, execution::par_unseq);
const vector<string> policies_string{"Not Specified", "seq", "par", "par_unseq"};
enum c_execution_policy { NOT_SPECIFIED = 0, SEQ, PAR, PAR_UNSEQ };
const int num_iterations = 1;

#define RED_ESCAPE ("\e[1;31m")
#define RESET_ESCAPE ("\e[0m")

//Analyze the results for each execution policy and for num of element in vector
//raise alerts if the execution policies are not working
void print_time_results(double (&time_results)[12][4], string header) {
  cout << endl << header << " times in ms:" << endl;
  cout << "execution policy:" << endl;
  cout << left << setw(13) << "no_policy";
  cout << left << setw(13) << "seq";
  cout << left << setw(13) << "par";
  cout << left << setw(13) << "par_unseq" << endl;

  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 4; j++) {
      cout << fixed << left << setw(12) << time_results[i][j] << " ";
    }

    cout << "ms"
         << "  num_elements: " << right << setw(11)
         << static_cast<long unsigned int>(num_elements / pow(2, i));
    if (time_results[i][NOT_SPECIFIED] < time_results[i][SEQ])
      cout << RED_ESCAPE << "  (no policy) faster than (seq) by "
           << setprecision(0) << fixed
           << 100.0 *
                  (time_results[i][SEQ] / time_results[i][NOT_SPECIFIED] - 1)
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
  
  //
  // vector reduce
  //
  cout << endl
       << "Comparing different number of elements vector reduce(accumulate) "
          "for different execution policies:"
       << endl
       << endl;

  int result_ix = 0;
  // vector reduce
  for (long unsigned int ne = num_elements; ne > min_num_elements;
       ne = ne / 2) {
    vector<int> v(ne, 1);
    cout << endl
         << "Comparing " << v.size()
         << " element vector accumulate for different execution policies:"
         << endl
         << endl;
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
      cout << fixed << "reduce(" << policies_string[policy]
                << ") took " << time.count() << "ms" << endl;
      cout << " sum = " << result << endl;
      time_results[result_ix][policy] = time.count();
    }
    result_ix++;
  }

  print_time_results(time_results, "reduce");

  //
  // vector sort
  //

  cout << endl
       << "comparing vector sort of different number of elements with "
          "different execution policies:"
       << endl
       << endl;

  cout << "starting creating vector of " << num_elements << " elements" << endl;
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
    cout << endl
         << "Comparing " << v.size()
         << " element vector sort for different execution policies:" << endl
         << endl;
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
      cout << fixed << "sort(" << policies_string[policy] << ") took "
           << time.count() << "ms" << endl;
      cout << " front():" << sorted.front() << " back():" << sorted.back()
           << endl;
      time_results[result_ix][policy] = time.count();
    }
    result_ix++;
  }

  print_time_results(time_results, "sort");

  //
  // Bugs with optimization
  //

  vector<int> v(num_elements, 1);
  cout << endl
       << "Demonstrating bugs with unparellizable and unvectorizable code:"
       << endl
       << endl;
  string header("for each sum(i.e. accumulate of ints)");
  cout << "correct "<< header <<" result:" << endl;
  int sum = 0;
  {
    auto start = chrono::high_resolution_clock::now();
    sum = 0;
    for_each(execution::seq, begin(v), end(v),
             [&](int i) { sum += i; });
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << fixed << header << "(seq) took " << time.count() << "ms"
         << endl;
    cout << " sum = " << sum << endl;
  }
  cout << "incorrect " << header << " result, data race:" << endl;
  // below 2 are broken - data race
  {
    auto start = chrono::high_resolution_clock::now();
    sum = 0;
    for_each(execution::par, begin(v), end(v),
             [&](int i) { sum += i; });
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << fixed << header << "(par) took " << time.count() << "ms"
         << endl;
    cout << " sum = " << sum << endl;
  }
  {
    auto start = chrono::high_resolution_clock::now();
    sum = 0;
    for_each(execution::par, begin(v), end(v),
             [&](int i) { sum += i; });
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << fixed << header << "(par) took " << time.count() << "ms"
         << endl;
    cout << " sum = " << sum << endl;
  }

  cout << "correct " << header << " result(mutex protected):" << endl;
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
    cout << fixed << header << "(par with mutex) took "
         << time.count() << "ms" << endl;
    cout << " sum = " << sum << endl;
  }

  cout << header << "  may deadlock due to vectorization of mutex:"
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
    cout << fixed << header << "(par_unseq with mutex) took "
         << time.count() << "ms" << endl;
    cout << " sum = " << sum << endl;
  }

  cout << header <<" vectorization safe using atomic:" << endl;
  {
    auto start = chrono::high_resolution_clock::now();
    atomic<int> sum = 0;
    for_each(execution::par_unseq, begin(v), end(v),
             [&](int i) { sum.fetch_add(i, std::memory_order_relaxed); });
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time = end - start;
    cout << fixed << header << "(par_unseq with atomic sum) took "
         << time.count() << "ms" << endl;
    cout << " sum = " << sum << endl;
  }
  cout << endl
       << "what we just learned is dont use the parallel version of for each "
       << header << " algorithm! (on my hw)"
       << endl;

  return 0;
}
