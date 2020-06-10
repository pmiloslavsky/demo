// Copyright Philip Miloslavsky 2020
#include <getopt.h>
#include <signal.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <queue>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "graph_class_interface.h"

// DTRACE
#include <sys/sdt.h>

using namespace std;

void signal_callback_handler(int signum) {
  cout << "Caught signal " << signum << endl;
  // Terminate program
  exit(signum);
}

// command line args
bool play_hex = false;
bool play_hex_human = false;
bool use_networkx = false;
int hex_board_edge = 9;
int hex_simulations = 1000;
int hex_algorithm = 0;
bool graph_demo = false;
int graph_size = 20;

void PrintHelp() {
  cout << "--play:              Play Hex   AI vs AI\n"
          "--playhuman:         Play Hex   AI vs Human\n"
          "--hex_board <val>:   Hex board edge length\n"
          "--hex_sim <val>:     Hex Monte Carlo simulations for AI\n"
          "--hex_alg <val>:     Hex Algorithm 0=Traversal 1=Union Find\n"
          "--graph_demo         Demo basic graph functionality"
          "  graph_demo options:\n"
          "--graph_size <val>   nodes in randomly generated graph\n"
          "--nx                 print graph in python networkx\n"
          "\n"
          "--help:              Show help\n";
  exit(1);
}

void ProcessArgs(int argc, char** argv) {
  const char* const short_opts = "p:b:s:a:g:n";
  const option long_opts[] = {{"play", no_argument, nullptr, 'p'},
                              {"playhuman", no_argument, nullptr, 'l'},
                              {"hex_board", required_argument, nullptr, 'b'},
                              {"hex_sim", required_argument, nullptr, 's'},
                              {"hex_alg", required_argument, nullptr, 'a'},
                              {"graph_demo", no_argument, nullptr, 'd'},
                              {"graph_size", required_argument, nullptr, 'g'},
                              {"nx", no_argument, nullptr, 'n'},
                              {nullptr, no_argument, nullptr, 0}};

  while (true) {
    const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

    if (-1 == opt) break;

    // stoi(optarg);  stof(optarg); string(optarg);
    switch (opt) {
      case 'p':
        play_hex = true;
        cout << "Request to play Hex  AI vs AI" << endl;
        break;

      case 'l':
        play_hex_human = true;
        cout << "Request to play Hex  Human vs AI" << endl;
        break;

      case 'b':
        hex_board_edge = stoi(optarg);
        cout << "Hex Board is " << hex_board_edge << "x" << hex_board_edge
             << endl;
        break;

      case 's':
        hex_simulations = stoi(optarg);
        cout << "Monte Carlo Simulation Count per Move is: " << hex_simulations
             << endl;
        break;

      case 'a':
        hex_algorithm = stoi(optarg);
        if (hex_algorithm == 0)
          cout << "Hex Algorithm to detect win uses Traversal " << endl;
        else
          cout << "Hex Algorithm to detect win uses Union Find " << endl;
        break;

      case 'd':
        graph_demo = true;
        cout << "Request to Demo Graph functionality" << endl;
        break;

      case 'g':
        graph_size = stoi(optarg);
        cout << "Creating random graphs with : " << graph_size << " nodes"
             << endl;
        break;

      case 'n':
        use_networkx = true;
        cout << "Request to print graphs in python networkx" << endl;
        break;

      case 'h':  // -h or --help
      case '?':  // Unrecognized option
      default:
        PrintHelp();
        break;
    }
  }
}

// Demonstrate graph Minimum Spanning Tree and min path algorithms
void graph_demo_code(void) {
  int mst_lengthk = 0;
  int mst_lengthp = 0;
  vector<int> Nrange(graph_size);
  iota(begin(Nrange), end(Nrange), 0);
  for_each(Nrange.begin(), Nrange.end(), [](int v) { cout << v << " "; });

  c_graphI gd(graph_size, "Dense RGB Localized Graph");
  c_graphI glin(graph_size, "Linear Localized Graph");
  c_graphI gd_mstk(graph_size, "MST of Dense Local Graph");
  c_graphI gd_mstp(graph_size, "MST of Dense Local Graph");

  gd.connect_graph_densely_and_locally();  // dense red green blue connectivity

  cout << gd;

  glin.connect_graph_linearly();

  cout << glin;

  gd_mstk.clear_connectivity();
  mst_lengthk = gd.minimum_spanning_tree_kruskal(gd_mstk, c_color::NONE);
  cout << "Kruskal MST path length: " << mst_lengthk << endl << gd_mstk;

  gd_mstp.clear_connectivity();
  mst_lengthp = gd.minimum_spanning_tree_prim(0, gd_mstp, c_color::NONE);
  cout << "Prim MST path length: " << mst_lengthp << endl << gd_mstp;

  if (mst_lengthp != mst_lengthk) {
    cout << "MST length differs depending on Prim or Kruskal " << endl;
    exit(-1);
  }
  cout << "MST length is the same for Prim and Kruskal " << endl << endl;

  if (use_networkx == true) {
    gd.display_graph_in_python("gd");
    gd_mstp.display_graph_in_python("gd_mstp");
    gd_mstk.display_graph_in_python("gd_mstk");
  }

  cout << endl
       << endl
       << "Testing if MST depends on start point for Prim" << endl;

  for (auto i : Nrange) {
    c_graphI gdl_mst_temp(graph_size, "MST of Dense Local Graph");
    int mst_l = 0;
    mst_l = gd.minimum_spanning_tree_prim(i, gdl_mst_temp, c_color::NONE);
    if (mst_lengthp != mst_l) {
      cout << "MST length differs depending on start point "
           << " " << mst_lengthp;
      cout << " " << mst_l << " " << i << endl;
      exit(-1);
    }
  }
  cout << endl
       << "End of Testing if MST depends on start point for Prim" << endl
       << endl;

  mst_lengthk = gd.min_path(0, graph_size - 1);
  mst_lengthp = gd.min_path(graph_size - 1, 0);

  if (mst_lengthp != mst_lengthk) {
    cout << "Min path differs depending on start and end order " << endl;
    exit(-1);
  }
  return;
}


//Graph Demo
//Play Hex vs AI
//Play Hex vs Human
int main(int argc, char** argv) {
  signal(SIGINT, signal_callback_handler);

  ProcessArgs(argc, argv);

  if (true == graph_demo) graph_demo_code();

  if ((play_hex != true) && (play_hex_human != true)) {
    cout << "exiting without playing hex" << endl;
    exit(0);
  }

  c_hex_gameI h("Hex Board", hex_board_edge, hex_simulations);

  h.connect_board();
  // cout << h;
  if (use_networkx == true) {
    h.display_graph_in_python("h");
  }
  cout << "Machine supports " << thread::hardware_concurrency()
       << " simultaneous threads" << endl;
  h.print_board(false, false);

  int edge_length = hex_board_edge + 2;

  random_device r;
  default_random_engine el(r());
  uniform_int_distribution<int> uniform_dist(1, edge_length);  // rand()

  //leaked implementation details of how hex board is implemented
  s_position sourcebot(edge_length / 2, 0, edge_length);                // blue
  s_position sourcetop(edge_length / 2, edge_length - 1, edge_length);  // blue
  s_position sourceleft(0, edge_length / 2, edge_length);               // red
  s_position sourceright(edge_length - 1, edge_length / 2, edge_length);  // red

  c_color next_player = c_color::BLUE;
  s_position source(sourcebot);
  s_position dest(sourcetop);
  s_position pos(uniform_dist(el), uniform_dist(el), edge_length);
  while (1) {
    while (1) {
      // init to random element (not used)
      pos = {uniform_dist(el), uniform_dist(el)};

      if ((next_player == c_color::BLUE) && (true == play_hex_human)) {
        h.get_human_move(next_player, &pos);
      } else {
        // find highest winning probability move using monte carlo simulation
        h.monte_carlo_move(next_player, &pos, hex_algorithm);
      }

      // if  (h.placed[pos.get_ix(h.edge_length)] != c_color::NONE)
      // {
      //   cout << pos << "Algorithm generated wrong move, aborting" << endl;
      // 	abort();
      // }

      h.add_piece(pos, next_player);
      h.print_board(true, false);
      this_thread::sleep_for(chrono::milliseconds(50));
      break;
    };

    bool player_won = false;
    if (hex_algorithm == 0)
      player_won = h.min_path_hex(source.get_ix(edge_length),
                                  dest.get_ix(edge_length), next_player, false);
    else
      player_won =
          h.min_path_hex_uf(source.get_ix(edge_length),
                            dest.get_ix(edge_length), next_player, false);

    if (player_won == true) {
      string blank(80, ' ');
      cout << next_player << " won!" << blank << endl;
      if (hex_algorithm == 0)
        h.min_path_hex(source.get_ix(edge_length), dest.get_ix(edge_length),
                       next_player, true, &pos, /*use open*/ true,
                       /*color win path*/ true);
      else
        h.min_path_hex_uf(source.get_ix(edge_length), dest.get_ix(edge_length),
                          next_player, true /*verbose*/,
                          /*color win path*/ true);
      h.print_board(false, /*print cost*/ false);  // TODO print probability
      break;
    }

    if (next_player == c_color::BLUE) {
      next_player = c_color::RED;
      source = sourceleft;
      dest = sourceright;
    } else {
      next_player = c_color::BLUE;
      source = sourcebot;
      dest = sourcetop;
    }
  }

  cout << "exiting..." << endl;
}
