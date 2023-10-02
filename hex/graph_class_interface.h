// Copyright Philip Miloslavsky 2020
#pragma once
#ifndef GCI_INCLUDED
#define GCI_INCLUDED

#include <string>

using namespace std;

// Color for:
// edge, node, placed piece, or winning placed piece
//
// c_color used in base graph and hex game
enum class c_color {
  WHITE = 0,
  RED = 1,
  BLUE,
  GREEN,
  NONE, //actually yellow
  RED_BLINK,
  BLUE_BLINK,
  GREEN_BLINK,
};

// https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
// Here, \033 is the ESC character, ASCII 27. It is followed by [,
// then zero or more numbers separated by ;, and finally the letter m.

//          foreground background
// black        30         40
// red          31         41
// green        32         42
// yellow       33         43
// blue         34         44
// magenta      35         45
// cyan         36         46
// white        37         47

// reset             0  (back to normal)
// bold/bright       1
// underline         4
// blink             5
// inverse           7  (swap foreground and background colors)
// bold/bright off  21
// underline off    24
// inverse off      27

//Note the implementation will also use terminal cursor up sequences i.e. cout << "\e[A"
// \033  is same as \e
#define CURSOR_UP_ESCAPE ("\e[A")

#define WHITE_ESCAPE ("\e[1;37m")
#define RED_ESCAPE ("\e[1;31m")
#define BLUE_ESCAPE ("\e[1;34m")
#define GREEN_ESCAPE ("\e[1;32m")
#define YEL_ESCAPE ("\e[1;33m")
#define RED_BLINK_ESCAPE ("\e[5;31m")
#define BLUE_BLINK_ESCAPE ("\e[5;34m")
#define GREEN_BLINK_ESCAPE ("\e[5;32m")
#define RESET_ESCAPE ("\e[0m")



ostream& operator<<(ostream& out, const c_color color);

// Hex board position
// NOTE edge_length below for hex is actually 2 more than the
// length of the Player board to make the algorithms easier
struct s_position {
  // alternative coordinate system for pretty printing
  int x;
  int y;
  // main coordinate system (need board edge length from graph) x+y*(el+2)
  int position_ix;

  int get_ix(int edge_length) { return x + y * (edge_length); }

  int get_ix(int xx, int yy, int edge_length) {
    return xx + yy * (edge_length);
  }

  void set_ix(int edge_length) { this->position_ix = x + y * (edge_length); }

  s_position(int a, int b, int edge_length)
      : x(a), y(b), position_ix(get_ix(a, b, edge_length)){};
  s_position(int node_ix, int edge_length)
      : x(node_ix % (edge_length)),
        y((int)(node_ix / (edge_length))),
        position_ix(node_ix){};

  friend bool operator==(const s_position a, const s_position b);
};

inline bool operator==(const s_position a, const s_position b) {
  if ((a.x == b.x) && (a.y == b.y)) return true;

  return false;
}

inline ostream& operator<<(ostream& out, const s_position position) {
  out << "(" << position.x << " " << position.y << ")";
  return out;
}

// For now dont export node and edge build functions
// or the weighted and colored edges

class c_graph;  // private but standalone
// c_graph supports undirected, directed, edges with [weight,color], nodes with
// [weight,color]
class c_graphI {
 public:
  ~c_graphI();
  c_graphI(int max_nodes, string name);

  // could use strategy pattern here
  //uses Union find to find MST
  int minimum_spanning_tree_kruskal(c_graphI& g_mstk, c_color color);
  //uses traversal to find MST
  int minimum_spanning_tree_prim(int start_node_ix, c_graphI& g_mstp,
                                 c_color color);

  // traversal algorithm
  int min_path(int start_node_ix, int end_node_ix);

  //called to reuse MST sub graph objects (not on the main graph)
  void clear_connectivity(void);

  // makes name.txt file for python to parse
  void display_graph_in_python(string name);

  void connect_graph_densely_and_locally(
      void);  // random dense red green blue connectivity
  void connect_graph_linearly(void);  // random linear connectivity of one color

  friend ostream& operator<<(ostream& out, const c_graphI& g);

 protected:
  unique_ptr<c_graph> pImpl;
};

ostream& operator<<(ostream& out, const c_graphI& g);

class c_hex_game;
class c_hex_gameI {
 public:
  ~c_hex_gameI();
  c_hex_gameI(string name, int player_edge_length, int max_simulations);

  //could be in constructor
  void connect_board(void);

  // place a piece on the board - pieces are basically color overlays, not new
  // nodes - called after AI or human move recommendation
  void add_piece(s_position pos, c_color color);

  // did we win functions implemented via traversal or union find (faster)
  bool min_path_hex(int start_node_ix, int destination_node_ix, c_color color,
                    bool verbose = false, s_position* p_move = nullptr,
                    bool use_open_spots = false, bool color_win_path = false);
  bool min_path_hex_uf(int start_node_ix, int destination_node_ix,
                       c_color color, bool verbose = false,
                       bool color_win_path = false);

  // run max_simulations Monte Carlo simulations to figure out the best move
  // uses threads
  void monte_carlo_move(c_color color, s_position* p_move, int algorithm);

  //ask human for their move
  void get_human_move(c_color color, s_position* p_move);

  // using colors and a drawing of a hexagonal board
  void print_board(bool backup, bool print_cost);

  // makes name.txt file with nodes and edges for separate python script to parse
  void display_graph_in_python(string name);

  friend ostream& operator<<(ostream& out, const c_hex_gameI& h);

 protected:
  //shared for threads in the implementation
  shared_ptr<c_hex_game> pImplh;
};

ostream& operator<<(ostream& out, const c_hex_gameI& h);

#endif
