// Copyright Philip Miloslavsky 2020
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <queue>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//#define DTRACE_INTEGRATION (1)
// DTRACE
//#include <sys/sdt.h>

//#define PYTHON_INTEGRATION (1)
// PYTHON
//#include <python3.8/Python.h>

#include "graph_class_interface.h"



using namespace std;
string colors_string[8] = {"<WHIT>", "<RED >", "<BLUE>", "<GREN>",
                           "<NONE>", "<R BL>", "<B BL>", "<G BL>"};

string colors_escape_string[9] = {
    WHITE_ESCAPE,      RED_ESCAPE,         BLUE_ESCAPE,
    GREEN_ESCAPE,      YEL_ESCAPE,         RED_BLINK_ESCAPE,
    BLUE_BLINK_ESCAPE, GREEN_BLINK_ESCAPE, RESET_ESCAPE};

ostream& operator<<(ostream& out, const c_color color) {
  out << colors_escape_string[static_cast<int>(color)];
  out.width(6);
  out << colors_string[static_cast<int>(color)] << RESET_ESCAPE;
  return out;
}

struct s_node {
  int node_ix;
  string name;
  s_node(int node_ix, string name) : node_ix(node_ix), name(name){};
};

struct s_colored_node : public s_node {
  c_color node_color;
  s_colored_node(c_color color, int node_ix, string name)
      : s_node::s_node(node_ix, name), node_color(color){};
};

struct s_weighted_colored_node : public s_colored_node {
  int weight;
  s_weighted_colored_node(int weight, c_color color, int node_ix, string name)
      : s_colored_node::s_colored_node(color, node_ix, name), weight(weight){};
  s_weighted_colored_node()
      : s_colored_node::s_colored_node(c_color::NONE, 0, "NONE"), weight(0){};
};

struct s_path_algorithm_node : public s_weighted_colored_node {
  int distance_from_source;
  int parent_ix;
};

struct s_prim_node {
  int node_ix;
  int distance_from_source;
};

struct compare_prim {
  bool operator()(const s_prim_node& l, const s_prim_node& r) {
    return l.distance_from_source > r.distance_from_source;
  }
};

// when the node has a color
struct s_colored_prim_node {
  int node_ix;
  int distance_from_source;
  c_color node_color;
};

struct compare_colored_prim {
  bool operator()(const s_colored_prim_node& l, const s_colored_prim_node& r) {
    return l.distance_from_source > r.distance_from_source;
  }
};

struct s_edge {
  int source_node_ix;
  int destination_node_ix;
  bool directional;
  s_edge(int six, int dix, bool dir = false)
      : source_node_ix(six), destination_node_ix(dix), directional(dir){};
};

struct s_colored_edge : public s_edge {
  c_color edge_color;
};

struct s_weighted_edge : public s_edge {
  int weight;
  s_weighted_edge(int six, int dix, int weight)
      : s_edge(six, dix), weight(weight){};
};

// no diamond inheritance
// could just collect these into one struct and use aggregate initialization
struct s_colored_weighted_edge : public s_weighted_edge {
  c_color edge_color;
  s_colored_weighted_edge(int six, int dix, int weight, c_color color)
      : s_weighted_edge(six, dix, weight), edge_color(color){};
};

struct compare_wc_edge {
  bool operator()(const s_colored_weighted_edge& l,
                  const s_colored_weighted_edge& r) {
    return l.weight > r.weight;
  }
};

inline ostream& operator<<(ostream& out, const s_colored_weighted_edge edge) {
  out << colors_escape_string[static_cast<int>(edge.edge_color)];
  out.width(2);
  out << edge.destination_node_ix;
  out << "(";
  out.width(2);
  out << edge.weight << ") " << RESET_ESCAPE;
  return out;
}

struct s_weight_color {
  int weight;
  c_color color;
};

inline ostream& operator<<(ostream& out, const s_weight_color wc) {
  out << colors_escape_string[static_cast<int>(wc.color)];
  out.width(2);
  out << wc.weight << RESET_ESCAPE;
  return out;
}

// Used in Hex only, not base
struct s_placed_piece : public s_position {
 public:
  c_color color;
  //  s_position position;
  s_placed_piece(s_position p, c_color c) : s_position(p), color(c){};
};

inline ostream& operator<<(ostream& out, const s_placed_piece piece) {
  out << colors_escape_string[static_cast<int>(piece.color)];
  out << "<";
  out.width(2);
  out.fill('0');
  out << piece.x;
  out.width(2);
  out.fill('0');
  out.width(2);
  out << (piece.y % 100);
  out << ">";
  out << RESET_ESCAPE;
  return out;
}

// End Hex

// Union find
struct s_subset {
  int parent;
  int rank;
};

int find(vector<s_subset>& subsets, int i) {
  if (subsets[i].parent != i)
    subsets[i].parent = find(subsets, subsets[i].parent);

  return subsets[i].parent;
}

void Union(vector<s_subset>& subsets, int x, int y) {
  int xroot = find(subsets, x);
  int yroot = find(subsets, y);

  //more advanced union find code:
  // if (subsets[xroot].rank < subsets[yroot].rank)
  // subsets[xroot].parent = yroot;
  // else if (subsets[yroot].rank < subsets[xroot].rank)
  // subsets[yroot].parent = xroot;
  // else
  {
    subsets[yroot].parent = xroot;
    // subsets[xroot].rank++;
  }
}

// graph could be a template instantiated with the different
// edge and vertex types but too much work, just use most complex
class c_graph {
 public:
  const int node_max;
  string name;
  vector<s_colored_weighted_edge>** p_adjacency_list;
  vector<vector<s_weight_color>> p_adjacency_matrix;
  // weight 1 means connected color of edge not node

  // stores other node attributes
  unordered_map<int, s_weighted_colored_node> nodes;

 public:
  c_graph(const int m, string name) : node_max(m), name(name) {
    cout << endl << "Creating graph with " << node_max << " nodes" << endl;

    // set up connectivity
    p_adjacency_list = new vector<s_colored_weighted_edge>*[node_max];

    for (int i = 0; i < node_max; ++i)
      p_adjacency_list[i] = new vector<s_colored_weighted_edge>;

    p_adjacency_matrix.resize(node_max);

    // uses default constructor to give no weight and WHITE nodes
    // weight could be infinity
    for (int i = 0; i < node_max; ++i) p_adjacency_matrix[i].resize(node_max);
  }

  ~c_graph() {
    // delete any heap memory we used
    for (int i = 0; i < node_max; ++i) delete p_adjacency_list[i];

    delete[] p_adjacency_list;

    cout << "Deleting " << name << " with " << node_max << " nodes" << endl;
  }

  void clear_connectivity(void) {
    // Clear connectivity
    for (int i = 0; i < node_max; ++i) p_adjacency_list[i]->clear();

    (*p_adjacency_list)->clear();

    s_weight_color zero = {0, c_color::WHITE};
    for (int i = 0; i < node_max; ++i)
      for (int j = 0; j < node_max; ++j) p_adjacency_matrix[i][j] = zero;

    // Clear anything needed for MST algorithms
    // this->verts.clear();
  }

  int edge_weight(int source_node_ix, int destination_node_ix) {
    auto edge = p_adjacency_list[source_node_ix]->begin();
    while (edge != p_adjacency_list[source_node_ix]->end()) {
      if (edge->destination_node_ix == destination_node_ix) return edge->weight;
      ++edge;
    }
    return numeric_limits<int>::max();

    // Replace with adjacency matrix lookup for speed
  }

  void add_node(int node_ix, string name, c_color color, int weight) {
    if (nodes.find(node_ix) != nodes.end()) {
      cout << name << " already in nodes" << endl;
    } else {
      // does this thing actually have a move constructor?
      nodes.emplace(node_ix,
                    s_weighted_colored_node(weight, color, node_ix, name));
    }
  }

  // not WHITE which is the default uninitialized after connection. NONE is default initialized
  void add_edge(int source_ix, int destination_ix, int weight,
                c_color color = c_color::NONE) {
    if (source_ix == destination_ix) return;

    if ((source_ix == -1) || (destination_ix == -1)) return;

    s_colored_weighted_edge edge(source_ix, destination_ix, weight, color);
    s_weight_color wc = {weight, color};

    this->p_adjacency_list[source_ix]->push_back(edge);
    swap(edge.source_node_ix, edge.destination_node_ix);
    this->p_adjacency_list[destination_ix]->push_back(edge);

    // no directionality for now
    this->p_adjacency_matrix[source_ix][destination_ix] = wc;
    this->p_adjacency_matrix[destination_ix][source_ix] = wc;
  }

  // colored edge aware, if color not specified, color is ONE and there are
  // no color tests
  int minimum_spanning_tree_kruskal(c_graph& mst_graph,
                                    c_color color = c_color::NONE) {
    // Have to use a PQ on edges not nodes
    priority_queue<s_colored_weighted_edge, vector<s_colored_weighted_edge>,
                   compare_wc_edge>
        pq_local;
    vector<s_subset> subsets(this->node_max);  // Union find
    int mst_length = 0;
    int edge_count = 0;
    mst_graph.name = "Kruskal MST of " + this->name + " using " +
                     colors_string[static_cast<int>(color)];

    for (int i = 0; i < this->node_max; ++i) {
      subsets[i].parent = i;
      subsets[i].rank = 0;
    }

    cout << endl << "Forming Kruskal MST graph with color " << color << endl;

    // this way of doing it adds duplicate edges in undirected graphs
    for (int i = 0; i < this->node_max; i++) {
      for (auto it = this->p_adjacency_list[i]->begin();
           it != this->p_adjacency_list[i]->end(); ++it) {
        // only accept edges of required color
        if (color != c_color::NONE) {
          if (it->edge_color != color) continue;
        }

        // if graph undirected, dont add a dupe
        if (it->destination_node_ix < i) continue;

        s_colored_weighted_edge edge = {i, it->destination_node_ix, it->weight,
                                        it->edge_color};
        pq_local.push(edge);
      }
    }

    while (edge_count < this->node_max - 1) {
      if (pq_local.empty()) {
        cout << "Could not form Kruskal MST!" << endl;
        break;
      }

      s_colored_weighted_edge cur = pq_local.top();
      pq_local.pop();

      // Add if the picked edge does not form cycles

      int x = find(subsets, cur.source_node_ix);
      int y = find(subsets, cur.destination_node_ix);
      if (x == y) continue;

      Union(subsets, cur.source_node_ix, cur.destination_node_ix);

      // Populate the new graph, could use node info from "this" graph
      if (mst_graph.nodes.find(cur.source_node_ix) == mst_graph.nodes.end())
        mst_graph.add_node(cur.source_node_ix,
                           "KMST" + to_string(cur.source_node_ix),
                           c_color::NONE, 0);
      if (mst_graph.nodes.find(cur.destination_node_ix) ==
          mst_graph.nodes.end())
        mst_graph.add_node(cur.destination_node_ix,
                           "KMST" + to_string(cur.destination_node_ix),
                           c_color::NONE, 0);

      mst_graph.add_edge(cur.source_node_ix, cur.destination_node_ix,
                         cur.weight, cur.edge_color);

      mst_length += cur.weight;
      edge_count++;

      // cout<< "edge_count: " << mst_graph.edge_count << "mst_length: " <<
      // mst_length;
    }

    if (edge_count != this->node_max - 1)
      mst_length = numeric_limits<int>::max();

    cout << "MST path length: " << mst_length << endl;

    return mst_length;
  }  // minimum_spanning_tree_kruskal

  // if color is NONE, this algorithm does not use edge color
  int minimum_spanning_tree_prim(int start, c_graph& mst_graph,
                                 c_color color = c_color::NONE) {
    int mst_length_during = 0;
    int mst_length_after = 0;
    mst_graph.name = "Prim MST of " + this->name + " using " +
                     colors_string[static_cast<int>(color)];

    cout << endl
         << "Forming Prim MST graph from start of " << start
         << " with color: " << color << endl;

    // need a priority queue with key update (really need update)....
    priority_queue<s_prim_node, vector<s_prim_node>, compare_prim> pq_local;

    vector<int> distance_from_mst(node_max);
    vector<int> parent(node_max);
    unordered_set<int> in_mst;

    auto it = this->nodes.begin();

    distance_from_mst.resize(node_max);

    // Initialize distances
    while (it != this->nodes.end()) {
      s_prim_node city = {it->first, numeric_limits<int>::max()};

      if (it->first == start) {
        city.distance_from_source = 0;
      }
      parent[it->first] = -1;
      distance_from_mst[it->first] = city.distance_from_source;

      if (it->first == start) {
        pq_local.push(city);
      }
      ++it;
    }

    // look at all the nighbors of the top node of the pq
    while (static_cast<int>(in_mst.size()) != this->node_max) {
      if (pq_local.empty()) {
        cout << "PQ Empty!" << endl;
        break;
      }

      s_prim_node cur = pq_local.top();
      pq_local.pop();

      if (in_mst.find(cur.node_ix) != in_mst.end()) continue;

      // cout << "Analyzing Vertex: " << cur.node_ix << " d: " <<
      // cur.distance_from_source << endl;

      // no pq update
      if (cur.distance_from_source == numeric_limits<int>::max()) {
        cout << "Reached end of graph without finding all elements" << endl;
        break;
      }

      // not in MST
      in_mst.insert(cur.node_ix);  // mark as visited
      mst_graph.add_node(cur.node_ix, "PMST" + to_string(cur.node_ix),
                         c_color::NONE, 0);
      mst_length_during += distance_from_mst[cur.node_ix];

      // go through adjacents of cur.node_ix
      // and put the in the pq

      for (auto edge = this->p_adjacency_list[cur.node_ix]->begin();
           edge != this->p_adjacency_list[cur.node_ix]->end(); ++edge) {
        int neighbor = edge->destination_node_ix;

        // if in MST dont consider - dont visit already visited
        if (in_mst.find(neighbor) != in_mst.end()) continue;

        int neighbordistance = edge->weight;

        if (neighbordistance < distance_from_mst[neighbor]) {
          if (color != c_color::NONE) {
            if (color != edge->edge_color) continue;
          }

          distance_from_mst[neighbor] = neighbordistance;
          parent[neighbor] = cur.node_ix;
          // erase neighbor and put back with smaller distance
          // when its put back the map is resorted
          s_prim_node ncity = {neighbor, neighbordistance};
          pq_local.push(ncity);
          // cout << "Added Vertex: " << ncity.node_ix << " d: " <<
          // ncity.distance_from_source << endl;
        }
      }
    }  // visit whole graph

    // Now add all parents as edges
    auto it2 = mst_graph.nodes.begin();
    while (it2 != mst_graph.nodes.end()) {
      mst_graph.add_edge(
          it2->first, parent[it2->first], distance_from_mst[it2->first],
          this->p_adjacency_matrix[it2->first][parent[it2->first]].color);
      mst_length_after += distance_from_mst[it2->first];

      ++it2;
    }

    if (static_cast<int>(in_mst.size()) != this->node_max)
      mst_length_during = numeric_limits<int>::max();

    cout << "MST path length: " << mst_length_during << " " << mst_length_after
         << " " << endl;
    return mst_length_during;
  }  // minimum_spanning_tree_prim

  int min_path(int start, int destination) {
    priority_queue<s_prim_node, vector<s_prim_node>, compare_prim> pq_local;

    vector<int> distance_from_source(node_max);
    vector<int> parent(node_max);
    unordered_set<int> visited;

    auto it = nodes.begin();

    while (it != nodes.end()) {
      s_prim_node city = {it->first, numeric_limits<int>::max()};

      if (it->first == start) {
        city.distance_from_source = 0;
      }

      parent[it->first] = -1;
      distance_from_source[it->first] = city.distance_from_source;
      pq_local.push(city);  // push all nodes unlike prim
      ++it;
    }

    while (!pq_local.empty()) {
      s_prim_node cur = pq_local.top();
      pq_local.pop();

      // shortcut due to not having a pq.update()
      if (distance_from_source[cur.node_ix] < cur.distance_from_source)
        continue;

      for (auto edge = p_adjacency_list[cur.node_ix]->begin();
           edge != p_adjacency_list[cur.node_ix]->end(); ++edge) {
        int neighbor = edge->destination_node_ix;

        if (visited.find(cur.node_ix) != visited.end()) continue;

        int neighbordistance = cur.distance_from_source + edge->weight;
        if (neighbordistance < distance_from_source[neighbor]) {
          distance_from_source[neighbor] = neighbordistance;
          parent[neighbor] = cur.node_ix;
          s_prim_node ncity = {neighbor, neighbordistance};
          pq_local.push(ncity);
        }
      }
      visited.insert(cur.node_ix);
    }  // pq

    if (distance_from_source[destination] == numeric_limits<int>::max()) {
      cout << "Cant get to destination" << endl;
      exit(0);
    }

    cout << "min distance from " << start << " to destination " << destination
         << " is " << distance_from_source[destination] << endl;

    list<string> answerpath;
    int end = destination;
    while (end != -1) {
      answerpath.push_back(nodes[end].name);
      end = parent[end];
    }

    for (auto st = answerpath.rbegin(); st != answerpath.rend(); ++st) {
      cout << *st << " -> ";
    }
    cout << endl;

    return distance_from_source[destination];
  }  // min path

  inline void display_graph_in_python(string graph_file) {
    cout << " Exporting file " << graph_file << ".txt" << endl;

    ofstream myFile(graph_file + ".txt");
    // file format via adjacency list
    // vertex ix
    // vertex name
    // edge_source_ix,
    // edge_destination_ix
    // edge_weight
    // edge_color

    // Send graph to stream
    for (int i = 0; i < node_max; ++i) {
      myFile << nodes[i].node_ix;
      if (i != node_max - 1) myFile << ",";  // No comma at end of line
    }
    myFile << "\n";
    for (int i = 0; i < node_max; ++i) {
      myFile << nodes[i].name;
      if (i != node_max - 1) myFile << ",";  // No comma at end of line
    }
    myFile << "\n";
    for (int i = 0; i < node_max; i++) {
      vector<s_colored_weighted_edge>::const_iterator it;
      for (it = p_adjacency_list[i]->begin(); it != p_adjacency_list[i]->end();
           ++it) {
        myFile << i;
        if (it != prev(p_adjacency_list[i]->end()))
          myFile << ",";  // No comma at end of line
      }
      if (i != node_max - 1) myFile << ",";
    }
    myFile << "\n";
    for (int i = 0; i < node_max; i++) {
      vector<s_colored_weighted_edge>::const_iterator it;
      for (it = p_adjacency_list[i]->begin(); it != p_adjacency_list[i]->end();
           ++it) {
        myFile << it->destination_node_ix;
        if (it != prev(p_adjacency_list[i]->end()))
          myFile << ",";  // No comma at end of line
      }
      if (i != node_max - 1) myFile << ",";
    }
    myFile << "\n";
    for (int i = 0; i < node_max; i++) {
      vector<s_colored_weighted_edge>::const_iterator it;
      for (it = p_adjacency_list[i]->begin(); it != p_adjacency_list[i]->end();
           ++it) {
        myFile << it->weight;
        if (it != prev(p_adjacency_list[i]->end()))
          myFile << ",";  // No comma at end of line
      }
      if (i != node_max - 1) myFile << ",";
    }
    myFile << "\n";
    for (int i = 0; i < node_max; i++) {
      vector<s_colored_weighted_edge>::const_iterator it;
      for (it = p_adjacency_list[i]->begin(); it != p_adjacency_list[i]->end();
           ++it) {
        if (it->edge_color == c_color::RED) myFile << "red";
        if (it->edge_color == c_color::BLUE) myFile << "blue";
        if (it->edge_color == c_color::GREEN) myFile << "green";
        if (it->edge_color == c_color::NONE) myFile << "cyan";
        if (it != prev(p_adjacency_list[i]->end()))
          myFile << ",";  // No comma at end of line
      }
      if (i != node_max - 1) myFile << ",";
    }

    myFile.close();

#ifdef PYTHON_INTEGRATION
    cout << " Calling python networkx" << endl;
    FILE* file;

    string str1 = "graph_print_networkx.py\0";
    string str2 = graph_file + ".txt" + "\0";

    wchar_t* pyargv[2];
    pyargv[0] = Py_DecodeLocale(&str1[0], NULL);
    pyargv[1] = Py_DecodeLocale(&str2[0], NULL);

    Py_SetProgramName(pyargv[0]);
    Py_Initialize();
    PySys_SetArgv(2, pyargv);
    file = fopen("graph_print_networkx.py", "r");
    PyRun_SimpleFile(file, "graph_print_networkx.py");
    fclose(file);
    // Py_Finalize();
    // cant initialize again if you finalize
#else
    cout << "Set PYTHON_INTEGRATION in code and provide graph_print_networkx.py"
         << endl;
#endif

  }  // display_graph_in_python

};  // c_graph

ostream& operator<<(ostream& out, const c_graph& g) {
  out << endl
      << "Printing " << g.name << " with " << g.node_max << " nodes" << endl;
  out << "Nodes:" << endl;

  unordered_map<int, s_weighted_colored_node>::const_iterator it =
      g.nodes.begin();
  while (it != g.nodes.end()) {
    out << it->second.name << " ";
    ++it;
  }
  out << endl;

  out << "Edges:" << endl;
  for (int i = 0; i < g.node_max; i++) {
    vector<s_colored_weighted_edge>::const_iterator it;
    out << i;
    for (it = g.p_adjacency_list[i]->begin();
         it != g.p_adjacency_list[i]->end(); ++it) {
      out << *it;
    }
    out << endl;
  }

  out << "Matrix:" << endl;
  // for (int i=0; i < g.node_max; i++)
  // {
  //   for (int j=0; j < g.node_max; j++)
  //   {
  // 	//system("Color 1A");
  // 	//out << RED_ESCAPE;
  // 	//out.width(2);
  // 	//out.fill('0');
  // 	out << g.p_adjacency_matrix[i][j] << " ";
  // 	//out << RESET_ESCAPE;
  //   }
  //   out << endl;
  // }

  // out << "Average Path Length: " << g.average_path_length() << endl;
  out << "End of display of Graph " << g.name << " with " << g.node_max
      << " nodes" << endl
      << endl;
  return out;
}  // operator<< c_graph

// locally means dont connect to the furthest nodes(node_ix is distance measure)
void c_graphI::connect_graph_densely_and_locally() {
  for (int i = 0; i < pImpl->node_max; ++i) {
    pImpl->add_node(i, "D" + to_string(i), c_color::NONE, 0);
  }

  for (int i = 0; i < pImpl->node_max; i++) {
    int j_start;
    int j_end;
    // make the graph have direct roads between "close" cities only
    j_start = max(0, i - pImpl->node_max / 2);
    j_end = min(pImpl->node_max - 1, i + pImpl->node_max / 2);
    for (int j = j_start; j < j_end; j++) {
      if (i == j) continue;

      random_device r;
      default_random_engine el(r());
      uniform_int_distribution<int> uniform_dist(0, 100);  // rand()
      uniform_int_distribution<int> uniform_dist2(1, 20);  // weight
      uniform_int_distribution<int> uniform_dist3(1, 3);   // color RGB
      if (((uniform_dist(el)) < 90) &&
          (pImpl->edge_weight(i, j) == numeric_limits<int>::max()))
        pImpl->add_edge(i, j, uniform_dist2(el),
                        static_cast<c_color>(uniform_dist3(el)));
    }
  }
}

void c_graphI::connect_graph_linearly() {
  for (int i = 0; i < pImpl->node_max; ++i) {
    pImpl->add_node(i, "L" + to_string(i), c_color::NONE, 0);
  }

  for (int i = 0; i < pImpl->node_max; i++) {
    int j_c = min(pImpl->node_max - 1, i + 1);

    if (i == j_c) continue;
    random_device r;
    default_random_engine el(r());
    uniform_int_distribution<int> uniform_dist2(1, 20);  // weight
    if (pImpl->edge_weight(i, j_c) == numeric_limits<int>::max()) {
      pImpl->add_edge(i, j_c, uniform_dist2(el), c_color::NONE);
    }
  }
}

class c_hex_game : public c_graph {
 public:
  const int edge_length;  // user edge + 2
  const int max_simulations;
  // Keep a cache of graph Nodes that have been recolored
  unordered_map<int, c_color>
      placed;  // int = x+ y*edge_length    for piece display

  // unordered_map<int, s_path_algorithm_node> saved_nodes[2]; //for print of
  // path algorithms

  c_hex_game(string name, const int el, const int ms)
      : c_graph((el + 2) * (el + 2), name),
        edge_length(el + 2),
        max_simulations(ms) {}

  // x,y+1  x+1,y+1  x-1,y  x+1,y  x,y-1  x-1,y-1
  vector<pair<int, int>> edge_morph{{0, 1},
                                    {1, 1},
                                    {-1,0},
                                    {1, 0},
                                    {0, -1},
                                    {-1, -1}};

  // color starters valid also
  bool point_valid(s_position position) {
    if ((position.x < 0) || (position.y < 0)) return false;

    if ((position.x >= edge_length) || (position.y >= edge_length))
      return false;

    return true;
  }

  void add_piece(s_position position, c_color color = c_color::NONE) {
    if (placed.find(position.get_ix(edge_length)) != placed.end()) {
      if ((placed[position.get_ix(edge_length)] == c_color::NONE) ||
          (color == c_color::GREEN) || (color == c_color::RED_BLINK) ||
          (color == c_color::BLUE_BLINK)) {
        placed[position.get_ix(edge_length)] = color;
      }
      // else
      // cout << position << color << " already placed" << endl;

    } else {
      // cout << position << " " << color << endl;
      placed[position.get_ix(edge_length)] = color;
    }
  }

  inline void add_placed_edge(s_position source, s_position destination,
                              c_color source_color = c_color::NONE,
                              c_color destination_color = c_color::NONE) {
    if (source == destination) return;

    if (!point_valid(destination)) return;

    if (!point_valid(source)) return;

    // base class node
    if (nodes.find(source.position_ix) == nodes.end()) {
      add_node(source.position_ix, "HEX" + to_string(source.position_ix),
               source_color, 0);
      // cout << source.position_ix << source_color << endl;
    }
    if (nodes.find(destination.position_ix) == nodes.end()) {
      add_node(destination.position_ix,
               "HEX" + to_string(destination.position_ix), destination_color,
               0);
      // cout << destination.position_ix << destination_color << endl;
    }

    // base class edge
    add_edge(source.position_ix, destination.position_ix, 0, c_color::NONE);

    // hex board overlay nodes
    add_piece(source, source_color);
    add_piece(destination, destination_color);
  }

  void connect_board() {
    // Also connect board edges to color nodes

    for (int y = 1; y < edge_length - 1; y++) {
      for (int x = 1; x < edge_length - 1; x++) {
        s_position source(x, y, edge_length);
        // x,y+1  x+1,y+1  x-1,y  x+1,y  x,y-1  x-1,y-1
        for (auto tuple = edge_morph.begin(); tuple != edge_morph.end();
             ++tuple) {
          s_position destination(x, y, edge_length);
          destination.x = x + tuple->first;
          destination.y = y + tuple->second;
          destination.set_ix(edge_length);

          if (!point_valid(destination)) continue;

          // dont connect here to color endpoints
          if ((destination.x > edge_length - 2) || (destination.x < 1))
            continue;

          if ((destination.y > edge_length - 2) || (destination.y < 1))
            continue;

          if (destination.y < y) continue;

          if (destination.x < x) continue;

          // cout << source << destination << tuple->first << " " <<
          // tuple->second<< endl;
          add_placed_edge(source, destination, c_color::NONE, c_color::NONE);
        }  // 6 edges
      }    // x
    }      // y
    // Now add the color endpoints red left right   blue top bottom
    for (int x = 1; x < edge_length - 1; x++) {
      s_position sourcebot(edge_length / 2, 0, edge_length);  // blue
      s_position sourcetop(edge_length / 2, edge_length - 1,
                           edge_length);  // blue
      s_position destinationbot(x, 1, edge_length);
      s_position destinationtop(x, edge_length - 2, edge_length);
      add_placed_edge(sourcebot, destinationbot, c_color::BLUE, c_color::NONE);
      add_placed_edge(sourcetop, destinationtop, c_color::BLUE, c_color::NONE);
    }

    for (int y = 1; y < edge_length - 1; y++) {
      s_position sourceleft(0, edge_length / 2, edge_length);  // red
      s_position sourceright(edge_length - 1, edge_length / 2,
                             edge_length);  // red
      s_position destinationleft(1, y, edge_length);
      s_position destinationright(edge_length - 2, y, edge_length);
      add_placed_edge(sourceleft, destinationleft, c_color::RED, c_color::NONE);
      add_placed_edge(sourceright, destinationright, c_color::RED,
                      c_color::NONE);
    }
  }  // connect_board

  void print_board(bool backup, bool min_path_costs = false) {
    //
    // WARNING THIS ROUTINE MOVES CURSOR
    //
    // Status 2 lines (this part scrolls up)
    // Board (edge_length lines)
    // Human Input 3 lines (dont make mistakes)

    // AI prints 2 lines Monte Carlo status
    // Human Takes 3 lines to get new coordinate if no error

    // print_board expects cursor to start at end of status
    // print_board will leave cursor at end of status

    for (int y = edge_length - 1; y >= 0; --y) {
      for (int j = 0; j < edge_length - y; j++) {
        cout << "   ";
      }
      for (int x = 0; x < edge_length; ++x) {
        s_position cur_pos(x, y, edge_length);
        cout.width(6);
        // dont print edges of board
        if ((x == 0) || (y == 0) || (x == edge_length - 1) ||
            (y == edge_length - 1)) {
          if (placed[cur_pos.get_ix(edge_length)] == c_color::NONE) {
            cout << "      ";
            continue;
          }
        }
        // cout << placed[x + edge_length*y];
        s_placed_piece piece = {cur_pos, placed[cur_pos.get_ix(edge_length)]};
        if (min_path_costs) {
          piece.x = 0;
          piece.y = 0;  // verts[cur_pos.get_ix()].distance_from_source;
          // piece.x = verts_saved[0][cur_pos.get_ix()].distance_from_source %
          // 100; //RED piece.y =
          // verts_saved[1][cur_pos.get_ix()].distance_from_source % 100;

          cout << piece;
        } else {
          cout << piece;
        }
      }
      // erase to end of line
      for (int j = 0; j < y; j++) {
        cout << "   ";
      }
      cout << endl;
    }

    if (true == backup) {
      for (int i = 0; i < (edge_length); i++) cout << CURSOR_UP_ESCAPE;
    }

  }  // print_board

  // user callable versions
  bool min_path_hex(int start, int destination, c_color color,
                    bool verbose = false, s_position* p_move = nullptr,
                    bool use_open_spots = false, bool color_win_path = false) {
    unordered_map<int, c_color> placed_copy = this->placed;
    return min_path_hex(placed_copy, start, destination, color, verbose, p_move,
                        use_open_spots, color_win_path);
  }

  bool min_path_hex_uf(int start, int destination, c_color color,
                       bool verbose = false, bool color_win_path = false) {
    unordered_map<int, c_color> placed_copy = this->placed;
    return min_path_hex_uf(placed_copy, start, destination, color, verbose,
                           color_win_path);
  }

  // this version is for threaded simulations
  // maybe we could adapt the base class functions but the path is based on
  // the overlay
  bool min_path_hex(unordered_map<int, c_color>& placed_copy, int start,
                    int destination, c_color color, bool verbose = false,
                    s_position* p_move = nullptr, bool use_open_spots = false,
                    bool color_win_path = false) {
    priority_queue<s_prim_node, vector<s_prim_node>, compare_prim> pq_local;

    vector<int> distance_from_source(node_max);
    vector<int> parent(node_max);
    unordered_set<int> visited;

    auto it = placed_copy.begin();

    while (it != placed_copy.end()) {
      s_prim_node city = {it->first, numeric_limits<int>::max()};

      if (it->first == start) {
        city.distance_from_source = 0;
      }

      parent[it->first] = -1;
      distance_from_source[it->first] = city.distance_from_source;
      pq_local.push(city);  // push all nodes unlike prim
      ++it;
    }

    while (!pq_local.empty()) {
      s_prim_node cur = pq_local.top();
      pq_local.pop();

      if (true == use_open_spots) {
        if ((placed_copy[cur.node_ix] != color) &&
            (placed_copy[cur.node_ix] != c_color::NONE))
          continue;
      } else {
        if (placed_copy[cur.node_ix] != color) continue;
      }

      // shortcut due to not having a pq.update()
      if (distance_from_source[cur.node_ix] < cur.distance_from_source)
        continue;

      for (auto edge = p_adjacency_list[cur.node_ix]->begin();
           edge != p_adjacency_list[cur.node_ix]->end(); ++edge) {
        int neighbor = edge->destination_node_ix;

        if (visited.find(cur.node_ix) != visited.end()) continue;

        if (true == use_open_spots) {
          if ((placed_copy[neighbor] != color) &&
              (placed_copy[neighbor] != c_color::NONE))
            continue;
        } else {
          if (placed_copy[neighbor] != color) continue;
        }

        int neighbordistance = 0;
        // only do zero cost from blue to blue - not from NONE to BLUE
        // dont use edge->weight
        if ((placed_copy[neighbor] == color) &&
            (placed_copy[cur.node_ix] == color)) {
          neighbordistance = cur.distance_from_source;  // weight 0
        } else {
          neighbordistance = cur.distance_from_source + 1;  // weight 1
          // this will leave a nice pattern
        }

        if (neighbordistance < distance_from_source[neighbor]) {
          distance_from_source[neighbor] = neighbordistance;
          parent[neighbor] = cur.node_ix;
          s_prim_node ncity = {neighbor, neighbordistance};
          pq_local.push(ncity);
        }
      }
      visited.insert(cur.node_ix);
    }  // pq

    s_position start_pos(start, this->edge_length);
    s_position dest_pos(destination, this->edge_length);

    if (distance_from_source[destination] == numeric_limits<int>::max()) {
      // cout << "Cant connect " << start_pos << " " << dest_pos << endl;
      return false;
    }

    // TODO save off distances via deep copy for prints of board (to see the pattern)
    // int color_ix = static_cast<int>(color) - 1;

    // auto vit = verts.begin(); //existence is same as vert

    // while (vit != verts.end())
    // {
    //   verts_saved[color_ix][vit->first]= vit->second;
    //   ++vit;
    // }

    // there is a winner so color the winning path if requested

    int color_none_count = 0;
    list<s_position> answerpath;
    int end = destination;
    while (end != -1) {
      s_position cur_pos(end, this->edge_length);
      if (true == color_win_path) {
        if (!((cur_pos == start_pos) || (cur_pos == dest_pos))) {
          if (color == c_color::RED)
            add_piece(cur_pos, c_color::RED_BLINK);
          else if (color == c_color::BLUE)
            add_piece(cur_pos, c_color::BLUE_BLINK);
        }
      }
      if (placed_copy[end] == c_color::NONE) color_none_count += 1;
      answerpath.push_back(cur_pos);
      end = parent[end];
    }

    if (verbose) print_board(true);

    if (verbose) {
      for (int i = 0; i < edge_length; i++) cout << endl;

      cout << "min distance from " << start_pos << " to destination "
           << dest_pos << " is " << distance_from_source[destination]
           << "                                                                "
              "     "
           << endl;
    }

    // pick 1 out of color_none_count open spots
    random_device r;
    default_random_engine el(r());
    uniform_int_distribution<int> uniform_dist(1, color_none_count);  // rand()
    int picked_none = uniform_dist(el);
    int color_none_ix = 0;

    for (auto st = answerpath.rbegin(); st != answerpath.rend(); ++st) {
      if (verbose) cout << *st << " -> ";

      if (use_open_spots) {
        if (placed_copy[(*st).get_ix(this->edge_length)] == c_color::NONE) {
          color_none_ix++;
          if (color_none_ix == picked_none) {
            *p_move = *st;
            return true;
          }
        }
      }
    }
    string blank(80, ' ');
    if (verbose) cout << blank << endl;

    return true;
  }  // min_path

  // threaded version
  bool min_path_hex_uf(unordered_map<int, c_color>& placed_copy, int start,
                       int destination, c_color color, bool verbose = false,
                       bool color_win_path = false) {
    vector<int> distance_from_source(node_max);
    vector<int> parent(node_max);
    unordered_set<int> visited;

    priority_queue<s_colored_weighted_edge, vector<s_colored_weighted_edge>,
                   compare_wc_edge>
        pq_local;
    vector<s_subset> subsets(node_max);  // Union find
    for (int i = 0; i < node_max; ++i) {
      subsets[i].parent = i;
      subsets[i].rank = 0;
    }

    // this way of doing it adds duplicate edges in undirected graphs
    for (int i = 0; i < node_max; i++) {
      for (auto it = p_adjacency_list[i]->begin();
           it != p_adjacency_list[i]->end(); ++it) {
        // only accept edges between nodes of right color
        if ((placed_copy[i] != color) ||
            (placed_copy[it->destination_node_ix] != color))
          continue;

        // if graph undirected, dont add a dupe
        if (it->destination_node_ix < i) continue;

        s_colored_weighted_edge edge = {i, it->destination_node_ix, it->weight,
                                        color};
        pq_local.push(edge);
      }
    }

    int edge_count = 0;
    while (edge_count < node_max - 1) {
      if (pq_local.empty()) {
        break;
      }

      s_colored_weighted_edge cur = pq_local.top();
      pq_local.pop();

      //****Add if the picked edge does not form cycles
      int x = find(subsets, cur.source_node_ix);
      int y = find(subsets, cur.destination_node_ix);
      if (x == y) continue;

      Union(subsets, cur.source_node_ix, cur.destination_node_ix);

      edge_count++;

      // cout<< "edge_count: " << mst_graph.edge_count << "mst_length: " <<
      // mst_length;
    }

    // see if source and destination have the same root
    int x = find(subsets, start);
    int y = find(subsets, destination);
    if (x != y) return false;

    s_position start_pos(start, this->edge_length);
    s_position dest_pos(destination, this->edge_length);

    // there is a winner so color the winning path if requested (its a set not a
    // minimum path)

    list<s_position> answerpath;

    auto it = placed_copy.begin();

    while (it != placed_copy.end()) {
      s_position cur_pos(it->first, this->edge_length);

      int x = find(subsets, start);
      int y = find(subsets, it->first);
      if (x != y) {
        ++it;
        continue;
      }

      if (true == color_win_path) {
        if (!((cur_pos == start_pos) || (cur_pos == dest_pos))) {
          if (color == c_color::RED)
            add_piece(cur_pos, c_color::RED_BLINK);
          else if (color == c_color::BLUE)
            add_piece(cur_pos, c_color::BLUE_BLINK);
        }
      }

      answerpath.push_back(cur_pos);
      ++it;
    }

    if (verbose) print_board(true);

    if (verbose) {
      for (int i = 0; i < edge_length; i++) cout << endl;
    }

    for (auto st = answerpath.rbegin(); st != answerpath.rend(); ++st) {
      if (verbose) cout << *st << " -> ";
    }
    string blank(80, ' ');
    if (verbose) cout << blank << endl;

    return true;
  }  // min_path_uf

  void monte_carlo_move_subthread(unordered_map<int, c_color> placed,
                                  int open_spot, c_color color,
                                  vector<c_color> open_colors, int* num_wins,
                                  chrono::steady_clock::time_point start,
                                  int algorithm) {
    // cout << "subthread running on " << s_position(open_spot) << endl;
    unordered_map<int, c_color> placed_copy;

    for (int alt = 0; alt < this->max_simulations; ++alt) {
      auto start_simulation = chrono::steady_clock::now();
      placed_copy = placed;
      placed_copy[open_spot] = color;  // the first non random move

      random_shuffle(open_colors.begin(), open_colors.end());

      int count = 0;
      auto it_open = placed_copy.begin();

      while (it_open != placed_copy.end()) {
        if ((placed_copy[it_open->first] == c_color::BLUE) ||
            (placed_copy[it_open->first] == c_color::RED)) {
          ++it_open;
          continue;
        }

        s_position cur_pos(it_open->first, this->edge_length);
        if ((cur_pos.x < 1) || (cur_pos.y < 1) ||
            (cur_pos.x > this->edge_length - 2) ||
            (cur_pos.y > this->edge_length - 2)) {
          ++it_open;
          continue;
        }

        placed_copy[it_open->first] = open_colors[count];
        count++;
        ++it_open;
      }
      // cout << "open spots filled: " << count << " during simulation " << alt
      // << endl;

      s_position sourcebot(this->edge_length / 2, 0, this->edge_length);  // blue
      s_position sourcetop(this->edge_length / 2, this->edge_length - 1,
                           this->edge_length);  // blue
      // if blue won
      bool blue_won = false;
      if (algorithm == 0) {
        blue_won = this->min_path_hex(
            placed_copy, sourcebot.get_ix(this->edge_length),
            sourcetop.get_ix(this->edge_length), c_color::BLUE);
      } else {
        blue_won = this->min_path_hex_uf(
            placed_copy, sourcebot.get_ix(this->edge_length),
            sourcetop.get_ix(this->edge_length), c_color::BLUE);
      }

      if (blue_won == true) {
        if (color == c_color::BLUE) *num_wins += 1;
      } else {
        if (color == c_color::RED) *num_wins += 1;
      }

      auto end = chrono::steady_clock::now();
      auto diff = end - start;
      auto diff2 = end - start_simulation;
      auto diff_sec = chrono::duration_cast<chrono::seconds>(diff);
      auto diff2_msec = chrono::duration_cast<chrono::milliseconds>(diff2);
      if (alt == this->max_simulations / 2) {
        // cout << "On simulation " << alt << "/" << this->max_simulations << "
        // for " << s_position(open_spot) << "  wins: " << *num_wins << "  secs
        // so far: " << diff_sec.count() << endl;
      }
#ifdef DTRACE_INTEGRATION
      DTRACE_PROBE1(hex, hex_monte_carlo_move_subthread_probe,
                    diff2_msec.count());
      if (alt == this->max_simulations - 1)
        DTRACE_PROBE1(hex, hex_monte_carlo_move_subthread_probe_end,
                      diff_sec.count());
#endif
    }  // simulations

    // cout << "total wins: " << *num_wins << endl;

  };  // monte_carlo_move_subthread

};  // c_hex_game

inline bool pair_compare(const pair<int, int>& a, const pair<int, int>& b) {
  return (a.second < b.second);
}

void c_hex_gameI::get_human_move(c_color color, s_position* p_move = nullptr) {
  bool valid_move = false;
  s_position cur_pos(0, 0, pImplh->edge_length);

  // skip the board (dont erase it)
  for (int i = 0; i < pImplh->edge_length; ++i) cout << endl;

  // Erase the input lines
  string blank(80, ' ');
  cout << blank << endl;
  cout << blank << endl;
  cout << blank << endl;
  int input_line_skip = 3;
  for (int i = 0; i < input_line_skip; i++) cout << CURSOR_UP_ESCAPE;

  // get human move in 3 lines
  cout << "Enter move coordinates in x,y format ex: 1,2 or 3,2 " << endl;
  while (false == valid_move) {
    int x = -1;
    int y = -1;
    char dummy = ' ';

    // bad input sucks - weird mumbo jumbo
    if (cin.fail()) {
      cin.clear();
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cin.seekg(0, ios::end);
    cin.clear();
    cin >> x >> dummy >> y;

    cout << "You entered: " << x << " " << y << endl;

    // Check that the move is not in pImplh->placed
    // and that it is within board
    cur_pos.x = x;
    cur_pos.y = y;
    cur_pos.set_ix(pImplh->edge_length);

    valid_move = true;

    if ((pImplh->placed[cur_pos.get_ix(pImplh->edge_length)] ==
         c_color::BLUE) ||
        (pImplh->placed[cur_pos.get_ix(pImplh->edge_length)] == c_color::RED)) {
      cout << "The coordinates you entered already have a placed piece" << endl;
      valid_move = false;
    }

    if ((cur_pos.x < 1) || (cur_pos.y < 1) ||
        (cur_pos.x > pImplh->edge_length - 2) ||
        (cur_pos.y > pImplh->edge_length - 2)) {
      cout << "The coordinates you entered are not on the board" << endl;
      valid_move = false;
    }

    if (valid_move == true) {
      *p_move = cur_pos;

      // go back up to status end
      int input_line_skip = 3;
      for (int i = 0; i < input_line_skip; i++) cout << CURSOR_UP_ESCAPE;

      for (int i = 0; i < pImplh->edge_length; ++i) cout << CURSOR_UP_ESCAPE;

      return;
    }
  }
}  // get_human_move

// the placed map has the currently colored BLUE/RED hex tiles
// we want to randomly fill the remainder with valid moves and then run
// the min_path algorithm to see who won
// for each possible move, record how many winning and how many loosing
// alternatives there are and pick the first move that has the most winning
// alternatives use threads to run the simulations for each possible move
// remaining
void c_hex_gameI::monte_carlo_move(c_color color, s_position* p_move = nullptr,
                                   int algorithm = 0) {

  unordered_map<int, int> won;  // s_position id -> number of wins
  int open_count = 0;
  srand(time(0));
  chrono::steady_clock::time_point start = chrono::steady_clock::now();

  auto it = pImplh->placed.begin();

  while (it != pImplh->placed.end()) {
    if ((pImplh->placed[it->first] == c_color::BLUE) ||
        (pImplh->placed[it->first] == c_color::RED)) {
      ++it;
      continue;
    }

    s_position cur_pos(it->first, pImplh->edge_length);
    if ((cur_pos.x < 1) || (cur_pos.y < 1) ||
        (cur_pos.x > pImplh->edge_length - 2) ||
        (cur_pos.y > pImplh->edge_length - 2)) {
      ++it;
      continue;
    }

    open_count += 1;
    won[it->first] = 0;
    ++it;
  }

  // cout << "Currently there are " << won.size() << " open spots " << color <<
  // " moves first" << endl;
  thread threads[pImplh->node_max];
  int num_wins[pImplh->node_max] = {0}; //answer from each thread considering open spots

  //for each open spot left, simulate the effects of moving there
  for (auto it = won.begin(); it != won.end(); ++it) {
    int open_spot = it->first;

    if ((pImplh->placed[open_spot] == c_color::BLUE) ||
        (pImplh->placed[open_spot] == c_color::RED))
      continue;

    // simulate placed[open_spot] = color;

    // to run each simulation we have to have a copy of the placed map
    // TODO: print_board can use verts to print probabilities
    int oc = open_count - 1;
    c_color second_color;
    if (color == c_color::BLUE)
      second_color = c_color::RED;
    else
      second_color = c_color::BLUE;

    //this is what each thread will shuffle
    //to do the monte carlo simulation
    //We should prob just pass the numbers of BLUES and REDs
    vector<c_color> open_colors;
    
    open_colors.resize(oc);
    int half_count = oc / 2;
    if ((oc % 2) == 1)
      half_count = half_count + 1;

    fill(open_colors.begin(), half_count + open_colors.begin(), second_color);
    fill(open_colors.begin() + half_count, open_colors.end(), color);

    s_position cur_pos(open_spot, pImplh->edge_length);
    // cout<< "For " << cur_pos << " generating " << half_count << " " <<
    // second_color << " and " << oc - half_count << " " << color << endl;

    // thread:
    // input: placed, open_spot, color, open_colors
    // needs to pass back won[open_spot]
    threads[open_spot] = thread(&c_hex_game::monte_carlo_move_subthread, pImplh,
                                pImplh->placed, open_spot, color, open_colors,
                                &(num_wins[open_spot]), start, algorithm);

  }  // won

  for (auto it = won.begin(); it != won.end(); ++it) {
    int open_spot = it->first;
    if ((pImplh->placed[open_spot] == c_color::BLUE) ||
        (pImplh->placed[open_spot] == c_color::RED))
      continue;

    threads[open_spot].join();
    won[open_spot] = num_wins[open_spot];
  }

  // new vector to store
  vector<pair<int, int>> sorted_wins;
  for (auto itsort = won.begin(); itsort != won.end(); ++itsort) {
    sorted_wins.push_back(make_pair(itsort->first, itsort->second));
  }
  sort(sorted_wins.begin(), sorted_wins.end(), pair_compare);

  auto max_map =
      max_element(won.begin(), won.end(),
                  [](const pair<int, int>& p1, const pair<int, int>& p2) {
                    return p1.second < p2.second;
                  });

  s_position max_wins(max_map->first, pImplh->edge_length);
  *p_move = max_wins;

  auto end = chrono::steady_clock::now();
  auto diff = end - start;
  auto diff_sec = chrono::duration_cast<chrono::seconds>(diff);
  cout << "win count: ";
  cout << s_position(sorted_wins[0].first, pImplh->edge_length) << ":"
       << sorted_wins[0].second << " ";
  cout << s_position(sorted_wins[1].first, pImplh->edge_length) << ":"
       << sorted_wins[1].second << " ";
  cout << s_position(sorted_wins[2].first, pImplh->edge_length) << ":"
       << sorted_wins[2].second << " "
       << "... ";
  cout << s_position(sorted_wins[open_count - 3].first, pImplh->edge_length)
       << ":" << sorted_wins[open_count - 3].second << " ";
  cout << s_position(sorted_wins[open_count - 2].first, pImplh->edge_length)
       << ":" << sorted_wins[open_count - 2].second << " ";
  cout << s_position(sorted_wins[open_count - 1].first, pImplh->edge_length)
       << ":" << sorted_wins[open_count - 1].second << " " << endl;

  cout << color << max_wins << " has max " << max_map->second
       << " winning alternatives out of " << pImplh->max_simulations
       << " simulations   positions considered: " << won.size() << " in "
       << diff_sec.count() << " seconds" << endl;
  return;
}

c_graphI::c_graphI(const int m, string name)
    : pImpl(make_unique<c_graph>(m, name)) {
  cout << "Constructed Base Graph " << name << endl;
}

c_graphI::~c_graphI() { cout << "Destroying Base Graph "; }

int c_graphI::minimum_spanning_tree_kruskal(c_graphI& g_mstk,
                                            c_color color = c_color::NONE) {
  return pImpl->minimum_spanning_tree_kruskal(*g_mstk.pImpl, color);
}

int c_graphI::minimum_spanning_tree_prim(int start_node_ix, c_graphI& g_mstp,
                                         c_color color = c_color::NONE) {
  return pImpl->minimum_spanning_tree_prim(start_node_ix, *g_mstp.pImpl, color);
}

int c_graphI::min_path(int start_node_ix, int end_node_ix) {
  return pImpl->min_path(start_node_ix, end_node_ix);
}

void c_graphI::clear_connectivity(void) { return pImpl->clear_connectivity(); }

void c_graphI::display_graph_in_python(string name) {
  return pImpl->display_graph_in_python(name);
}

ostream& operator<<(ostream& out, const c_graphI& g) {
  out << g.pImpl->node_max << " (max nodes): " << *(g.pImpl);
  return out;
}

//Base class print not fancy user print
ostream& operator<<(ostream& out, const c_hex_gameI& h) {
  const c_graph* pg = h.pImplh.get();
  out << *pg;
  return out;
}

void c_hex_gameI::display_graph_in_python(string name) {
  return pImplh->display_graph_in_python(name);
}

c_hex_gameI::c_hex_gameI(string name, int player_edge_length,
                         int max_simulations)
    : pImplh(
          make_shared<c_hex_game>(name, player_edge_length, max_simulations)) {
  cout << "Constructed Hex Game " << name << endl;
}

c_hex_gameI::~c_hex_gameI() { cout << "Destroying Hex Game "; }

void c_hex_gameI::connect_board() { return pImplh->connect_board(); }

void c_hex_gameI::print_board(bool backup, bool print_cost) {
  return pImplh->print_board(backup, print_cost);
}

void c_hex_gameI::add_piece(s_position pos, c_color color) {
  return pImplh->add_piece(pos, color);
}

bool c_hex_gameI::min_path_hex(int start_node_ix, int destination_node_ix,
                               c_color color, bool verbose, s_position* p_move,
                               bool use_open_spots, bool color_win_path) {
  return pImplh->min_path_hex(start_node_ix, destination_node_ix, color,
                              verbose, p_move, use_open_spots, color_win_path);
}

bool c_hex_gameI::min_path_hex_uf(int start_node_ix, int destination_node_ix,
                                  c_color color, bool verbose,
                                  bool color_win_path) {
  return pImplh->min_path_hex_uf(start_node_ix, destination_node_ix, color,
                                 verbose, color_win_path);
}
