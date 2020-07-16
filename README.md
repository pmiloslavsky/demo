# Demonstration code
This contains several programs for demonstration purposes

## hex directory
Has a c++ implementation of the game hex
using threaded Monte Carlo simulation.
Concepts include these graph algorithms:
* Kruskal Minimum Spanning Tree
* Prim Minimum Spanning Tree
* Dijkstra’s shortest path
* Union Find
![Hex Screenshot](hex/hex.png?raw=true "Hex")

## lorenz_3d_simulation directory
Has a C++ 3D simulation of the lorenz system attractor. It was written using sfml and tgui libraries.
Features include:
* Loreniz PDE simulation and Frame of Reference display
* Changeable particle and trail settings
* Zoom via mouse and Rotation via Euler Angle sliders (Theta)
* Skinned Modern GUI elements (Sliders, Button, Menus)
* Escaped Particle Detection
* On Screen Axes to indicate rotation
![Lorenz Screenshot](lorenz_3d_simulation>/lorentz_widgets_save.png?raw=true "Lorenz")

## maximum_path_sum directory
Has a solution to the maximum sum of a non forked path in a binary tree.
I found it interesting because some of the solutions on the internet are deficient.
Concepts include:
* pretty printing binary tree
* testing for maximum sum invariance under left right symmetry and tree rotation

## max_connected_color_count directory
This is a harder variant of the flood fill algorithm. We have to find the biggest chunk of 4-way connected colors in the grid and return number of elements in the connected chunk. We dont use recursion. We end up using a tailored Union Find algorithm and memoizing the counts of joined sets in the set heads.

## parallel_algorithms directory
Looks at C++17 STL sort and reduce and for_each using seq,unseq,par,par_unseq execution policies.
Also looks at ways of writing code that make algorithms unparallelizable
and unvectorizable.
Conclusion:
* time your code any time you use an execution policy
* vector size may affect whether or not there is an improvement
![Parallel Algorithms Screenshot](parallel_algorithms/parallel.png?raw=true "Hex")


