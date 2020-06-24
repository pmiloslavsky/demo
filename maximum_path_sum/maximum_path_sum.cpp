// Copyright 2020 Philip Miloslavsky
#include <algorithm>
#include <iostream>
#include <limits>
#include <random>

using namespace std;

//
// Demonstration of how to get the max path sum in a binary tree (no forks in
// path allowed) NOT a Minimum Spanning Tree type path Also we test to see that
// you get the same answer if you swap left and right for each node

class c_node {
 public:
  int value;
  c_node *left;
  c_node *right;

  c_node(int value) : value(value), left(nullptr), right(nullptr) {}

  void set_left(c_node *c) { this->left = c; }

  void set_right(c_node *c) { this->right = c; }

  // make a binary search tree
  void insert(int value) {
    if (value < this->value) {
      if (this->left != NULL)
        left->insert(value);
      else {
        this->left = new c_node(value);
      }
    } else if (value >= this->value) {
      if (this->right != NULL)
        right->insert(value);
      else {
        this->right = new c_node(value);
      }
    }
  }
  friend void freetree(c_node *c);
  friend void swaptree(c_node *c);

};  // c_node

// root MAY be on stack not on heap, wont get the root - deliberately
// frees all the memory for the nodes recursively
void freetree(c_node *c) {
  if (c->left != nullptr) {
    freetree(c->left);
    delete c->left;
  }
  if (c->right != nullptr) {
    freetree(c->right);
    delete c->right;
  }
} // freetree

// swap left and right recursively
void swaptree(c_node *c) {
  if (c == nullptr) return;

  swaptree(c->left);
  swaptree(c->right);
  swap(c->left, c->right);
} // swaptree

// rotate the tree and return new root
c_node *rotate_right(c_node *y) {
  c_node *x = y->left;
  if (x == nullptr) return y;
  c_node *xr = x->right;

  x->right = y;
  y->left = xr;

  // return new root
  return x;
} // rotate_right

// modified from stackexchange
// pretty prints binary tree
vector<int> r_or_l(1000);
void print_binary_tree(c_node *c, int depth) {
  if (c == nullptr) return;

  if (static_cast<long unsigned int>(depth) > r_or_l.size()) {
    r_or_l.resize(2 * depth);
  }
                  
  cout << "    ";
  for (int i = 0; i < depth; i++)
    if (i == depth - 1)
      cout << (r_or_l[depth - 1] ? "├" : "└") << "────";
    else
      cout << (r_or_l[i] ? "│" : " ") << "    ";
  cout << c->value << endl;
  r_or_l[depth] = 1;
  print_binary_tree(c->right, depth + 1);
  r_or_l[depth] = 0;
  print_binary_tree(c->left, depth + 1);
}

// return root to leaf max
// save off leaf to leaf max
// amend max_sum with both ways of keeping track
int recursive_max_path_sum(const c_node *c, int &max_sum) {
  if (c == nullptr) return numeric_limits<int>::min();

  // not passed up via recursion
  // leaf to leaf: l+r+v
  // amend max_sum if needed

  // passed up via recursion
  // root to leaf: l+v, r+v, v
  // amend max_sum if needed

  int left = recursive_max_path_sum(c->left, max_sum);
  int right = recursive_max_path_sum(c->right, max_sum);

  int leaf_leaf = c->value;
  if (left != numeric_limits<int>::min()) leaf_leaf += left;
  if (right != numeric_limits<int>::min()) leaf_leaf += right;

  max_sum = max(leaf_leaf, max_sum);
  // cout << c->value << "LL:" << max_sum << endl;

  int root_leaf = numeric_limits<int>::min();

  root_leaf = max(root_leaf, c->value);
  if (left != numeric_limits<int>::min())
    root_leaf = max(root_leaf, left + c->value);
  if (right != numeric_limits<int>::min())
    root_leaf = max(root_leaf, right + c->value);

  max_sum = max(root_leaf, max_sum);

  // cout << c->value << "R:" << root_leaf << endl;
  return root_leaf;
} // recursive_max_path_sum

inline int max_path_sum(const c_node *c) {
  int max_sum = numeric_limits<int>::min();

  recursive_max_path_sum(c, max_sum);

  return max_sum;
} // max_path_sum

int main() {
  cout << "creating custom binary tree" << endl;

  c_node n1(5);
  c_node n20(-1);
  c_node n21(8);
  c_node n30(11);
  c_node n31(13);
  c_node n32(7);
  c_node n40(-7);
  c_node n41(2);
  c_node n42(-1);

  n1.set_left(&n20);
  n1.set_right(&n21);

  n20.set_left(&n30);
  n21.set_left(&n31);
  n21.set_right(&n32);

  n30.set_left(&n40);
  n30.set_right(&n41);
  n32.set_right(&n42);

  // cout << n1 << endl;

  print_binary_tree(&n1, 0);

  cout << "Max path sum (should be 38):" << max_path_sum(&n1) << endl;

  cout << "creating custom binary tree" << endl;
  c_node t(0);
  t.insert(-1);
  t.insert(1);
  t.insert(2);
  print_binary_tree(&t, 0);

  cout << "Max path sum (should be 3):" << max_path_sum(&t) << endl;
  freetree(&t);

  cout << "creating random binary tree" << endl;
  // not on stack because we rotate later and then call free recursively
  // so everything has to be from the same pool
  c_node *ptt1 = new c_node(0);
  random_device r;
  default_random_engine el(r());
  uniform_int_distribution<int> uniform_dist(-100, 101);
  vector<int> vec(10);

  for (auto it = vec.begin(); it != vec.end(); ++it) {
    *it = uniform_dist(el);
    cout << *it << " ";
  }

  for (auto it = vec.begin(); it != vec.end(); ++it) {
    ptt1->insert(*it);
  }
  cout << endl;
  print_binary_tree(ptt1, 0);
  cout << "Max path sum:" << max_path_sum(ptt1) << endl;

  // max path sum is invariant under swapping
  cout << "swapping left and right" << endl;
  swaptree(ptt1);
  print_binary_tree(ptt1, 0);
  cout << "Max path sum:" << max_path_sum(ptt1) << endl;

  // max path sum is NOT invariant under right rotation (may need several
  // rotates to see that)
  c_node *rt;
  rt = rotate_right(ptt1);
  cout << "rotating tree to right" << endl;
  print_binary_tree(rt, 0);
  cout << "Max path sum:" << max_path_sum(rt) << endl;

  cout << "rotating tree to right again" << endl;
  c_node *rrt;
  rrt = rotate_right(rt);
  print_binary_tree(rrt, 0);
  cout << "Max path sum:" << max_path_sum(rrt) << endl;

  freetree(rrt);
  delete rrt;

  cout << "creating another binary tree but sorting the random numbers before "
          "we insert"
       << endl;
  c_node *ptt2 = new c_node(0);
  sort(vec.begin(), vec.end());
  for (auto it = vec.begin(); it != vec.end(); ++it) {
    ptt2->insert(*it);
  }
  cout << endl;
  print_binary_tree(ptt2, 0);
  cout << "Max path sum:" << max_path_sum(ptt2) << endl;

  cout << "swapping left and right" << endl;
  swaptree(ptt2);
  print_binary_tree(ptt2, 0);
  cout << "Max path sum:" << max_path_sum(ptt2) << endl;

  freetree(ptt2);
  delete ptt2;
} // main
