#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

using namespace std;

int grid_size;

// we are not iterating recursively so we can
// get by with scan line type neighbors
vector<int> get_neighbors(int row, int col) {
  vector<int> neighbors{};
  int new_row;
  int new_col;

  // up
  new_row = row - 1;
  new_col = col;
  if (new_row >= 0) neighbors.push_back(grid_size * new_row + new_col);

  // left
  new_row = row;
  new_col = col - 1;
  if (new_col >= 0) neighbors.push_back(grid_size * new_row + new_col);

  return neighbors;
}

//
// Maximum connected squares of the same color along rows and columns (diagnals
// not allowed)
//
int max_connected_color_count(vector<vector<int>> &color_grid, int &largest_ix,
                              int verbose) {
  int maximum = 1;

  int largest_found_ix = 0;

  // values updated for set masters only
  // initially everyone is their own set master
  vector<int> cached_color_count(grid_size * grid_size, 1);

  // for a particular location stores the index of the master of the set
  //(where the counts are kept)
  vector<int> set_master(grid_size * grid_size, 0);

  // everyone gets their own set
  for (int i = 0; i < grid_size * grid_size; i++) set_master[i] = i;

  for (long unsigned int row = 0; row < color_grid.size(); ++row) {
    for (long unsigned int col = 0; col < color_grid[row].size(); ++col) {
      // examine all up and to the left neighbors (or unvisited neighbors if
      // recursion)
      //  if the neighbor has the same color
      //    if we dont have a master yet determine the set master
      //       (follow linked list if needed)
      //       increment cached_color_count of set master by 1
      //       cache the set master in our node
      //    if we already have a master that means we are joining sets
      //       get the master of the neighbor (maybe need linked list traversal)
      //       update the count of our master with sum of sets
      //       change neighbor master to be our master
      //    if our cached_color_count is greater than stored maximum
      //      replace stored maximum
      // mark this node as already visited
      int our_ix = grid_size * row + col;
      vector<int> neighbors{};
      neighbors = get_neighbors(row, col);

      for (auto &neighbor_ix : neighbors) {
        if (neighbor_ix == -1) break;  // should not happen
        // cout << our_ix << ":" << neighbor_ix << endl;
        // this_thread::sleep_for(chrono::milliseconds(100));
        int neighbor_value =
            color_grid[neighbor_ix / grid_size][neighbor_ix % grid_size];
        int our_value = color_grid[row][col];
        if (our_value == neighbor_value) {
          int set_master_ix = set_master[our_ix];

          if (set_master[our_ix] == our_ix) {
            // find the master starting from the neighbor
            set_master_ix = set_master[neighbor_ix];
            while (set_master_ix != set_master[set_master_ix]) {
              set_master_ix = set_master[set_master_ix];
            }

            set_master[our_ix] = set_master_ix;
            cached_color_count[set_master_ix] += 1;
          } else {
            // Hacked Union Find Algorithm
            // if its the second neighbor but we are already part of a set
            // if its a different set, add its count to our master
            //   make the master of second set point to the first set master

            // if its the same set then we already added one, no need to play
            // with counts
            if (set_master[neighbor_ix] != set_master[our_ix]) {
              // find the neighbor master starting from the neighbor
              int n_master_ix = set_master[neighbor_ix];
              while (n_master_ix != set_master[n_master_ix]) {
                n_master_ix = set_master[n_master_ix];
              }

              cached_color_count[set_master[our_ix]] +=
                  cached_color_count[n_master_ix];
              set_master[n_master_ix] = set_master[our_ix];
              // we wont bother resetting all the members of the second set to
              // the right masters as that requires parents
            }
          }

          // the master thats stored at our_ix is correct
          if (cached_color_count[set_master_ix] > maximum) {
            maximum = cached_color_count[set_master_ix];
            largest_found_ix = set_master_ix;
            if (verbose > 0) cout << our_ix << " m:" << maximum << endl;
          }
        }  // color match
      }    // neighbors
    }      // col
  }        // row

  if (verbose > 0) {
    cout << "set masters:" << endl;
    for (int i = 0; i < grid_size; i++) {
      for (int j = 0; j < grid_size; j++) {
        cout << setw(3) << set_master[i * grid_size + j] << ", ";
      }
      cout << endl;
    }
    cout << endl;
  }

  largest_ix = largest_found_ix;
  return maximum;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    cout << "pass matrix edge size" << endl;
    exit(0);
  }

  grid_size = atoi(argv[1]);

  random_device r;
  default_random_engine el(r());

  vector<vector<int>> color_grid(grid_size, vector<int>(grid_size, 0));

  cout << endl << endl << "Test 1:" << endl;

  for (auto &vec : color_grid) {
    for (auto &x : vec) {
      uniform_int_distribution<int> uniform_dist(0, 7);
      x = uniform_dist(el);
      cout << setw(3) << x << ", ";
    }
    cout << endl;
  }
  cout << endl;

  int largest_ix = 0;
  int largest_count = max_connected_color_count(color_grid, largest_ix, 0);

  cout << largest_count << " grouped "
       << color_grid[largest_ix / grid_size][largest_ix % grid_size]
       << "'s found at: " << largest_ix / grid_size << ","
       << largest_ix % grid_size << endl;

  // Test2:
  cout << endl << endl << "Test 2 (should be 23):" << endl;
  grid_size = 10;
  color_grid = {
      {0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
      {0, 1, 0, 1, 1, 0, 1, 1, 1, 1},
      {0, 0, 0, 1, 1, 0, 1, 1, 1, 1},
      {0, 1, 1, 1, 1, 0, 1, 1, 1, 1},
      {0, 1, 1, 1, 1, 0, 1, 1, 1, 1},
      {0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  };

  for (auto &vec : color_grid) {
    for (auto &x : vec) {
      uniform_int_distribution<int> uniform_dist(1, 7);
      if (x != 0) {
        x = uniform_dist(el);
      }
    }
  }

  for (int i = 0; i < grid_size; i++) {
    for (int j = 0; j < grid_size; j++) {
      cout << color_grid[i][j] << ", ";
    }
    cout << endl;
  }

  largest_ix = 0;
  largest_count = max_connected_color_count(color_grid, largest_ix, 0);

  cout << largest_count << " grouped "
       << color_grid[largest_ix / grid_size][largest_ix % grid_size]
       << "'s found at: " << largest_ix / grid_size << ","
       << largest_ix % grid_size << endl;

  // Now create another nasty example:
  cout << endl << endl << "Test 3 (should be 23):" << endl;
  grid_size = 10;
  color_grid = {
      {0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
      {1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
      {0, 0, 0, 1, 1, 0, 1, 1, 1, 1},
      {0, 1, 1, 1, 1, 0, 1, 1, 1, 1},
      {0, 1, 1, 1, 1, 0, 1, 1, 1, 1},
      {0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
      {1, 1, 1, 0, 1, 0, 1, 1, 1, 1},  // note this row
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  };

  for (auto &vec : color_grid) {
    for (auto &x : vec) {
      uniform_int_distribution<int> uniform_dist(1, 7);
      if (x != 0) {
        x = uniform_dist(el);
      }
    }
  }

  for (int i = 0; i < grid_size; i++) {
    for (int j = 0; j < grid_size; j++) {
      cout << color_grid[i][j] << ", ";
    }
    cout << endl;
  }

  largest_ix = 0;
  largest_count = max_connected_color_count(color_grid, largest_ix, 0);

  cout << largest_count << " grouped "
       << color_grid[largest_ix / grid_size][largest_ix % grid_size]
       << "'s found at: " << largest_ix / grid_size << ","
       << largest_ix % grid_size << endl;
}
