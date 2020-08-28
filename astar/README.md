## astar directory
Astar,dijkstra,double sided and directed search run on real US highway data (courtesy of MIT)
(can produce kml files that are displayable in google maps)
<br>
https://ocw.mit.edu/courses/electrical-engineering-and-computer-science/6-006-introduction-to-algorithms-fall-2011/assignments/
<br>
python dijkstra.py.single_sided < tests/0boston_berkeley.in
Path: BOSTON EAST BOST, MA -> BERKELEY, CA
Graph size: 90415 nodes, 250604 edges
Nodes visited: 84458
Path length: 54161784.7363
Number of roads: 732
Time used in seconds: 1.276

python dijkstra.py.single_sided_directed  < tests/0boston_berkeley.in
Path: BOSTON EAST BOST, MA -> BERKELEY, CA
Graph size: 90415 nodes, 250604 edges
Nodes visited: 47100
Path length: 54161784.7363
Number of roads: 732
Time used in seconds: 1.003

python dijkstra.py.double_sided < tests/0boston_berkeley.in
Path: BOSTON EAST BOST, MA -> BERKELEY, CA
Graph size: 90415 nodes, 250604 edges
Nodes visited: 79550
Path length: 54161784.7363
Number of roads: 732
Time used in seconds: 1.241

python dijkstra.py.double_sided_directed  < tests/0boston_berkeley.in
Path: BOSTON EAST BOST, MA -> BERKELEY, CA
Graph size: 90415 nodes, 250604 edges
Nodes visited: 20836
Path length: 54161784.7363
Number of roads: 732
Time used in seconds: 0.403