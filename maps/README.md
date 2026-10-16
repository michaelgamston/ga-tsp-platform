# Test maps

Format: square, symmetric distance matrix. No header. Row `i`, column `j` = distance from node `i` to node `j`.
- Diagonal is `0`
- `-1` = no edge between the nodes
- Distances are positive integers (Euclidean distances between random points, rounded)

Every map is guaranteed to have at least one valid tour.

| File | Nodes | Missing edges | Optimal tour length |
|---|---|---|---|
| map_05.csv | 5 | 1 | 280 |
| map_08.csv | 8 | 1 | 282 |
| map_12.csv | 12 | 3 | 319 |
| map_16.csv | 16 | 6 | 349 |
| map_20.csv | 20 | 10 | unknown |
