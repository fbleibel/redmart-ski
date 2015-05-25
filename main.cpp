// A solution to http://geeks.redmart.com/2015/01/07/skiing-in-singapore-a-coding-diversion/
// Note: uses C++11 features
#include <iostream>
#include <fstream>
#include <vector>

struct ElevationMap {
    int rows = 0;
    int columns = 0;
    int size = 0; // = rows * columns
    std::vector<int> data; // row-major

    // Assumes input is well-formed!
    void read(std::istream& is) {
        is >> columns >> rows;
        size = rows * columns;
        data.resize(size);
        for (int& datum: data) {
            is >> datum;
        }
    }

    // Returns indices in data[] corresponding to neighboring squares on the map
    // (left, right, top or bottom) which we can move to if we're skiing from
    // 'index'.
    std::vector<int> lower_neighbors(int index) const {
        std::vector<int> result;
        const int row = index / columns;
        const int column = index % columns;
        const int elevation = data[index];

        // left
        if (column > 0) {
            if (elevation > data[index - 1]) {
                result.push_back(index - 1);
            }
        }
        // right
        if (column < columns - 1) {
            if (elevation > data[index + 1]) {
                result.push_back(index + 1);
            }
        }
        // top
        if (row > 0) {
            if (elevation > data[index - columns]) {
                result.push_back(index - columns);
            }
        }
        // bottom
        if (row < rows - 1) {
            if (elevation > data[index + columns]) {
                result.push_back(index + columns);
            }
        }
        return result;
    }
};

/* Solve the problem for the given elevation map:
 *
 * There is an implicit directed graph in which vertices are positions on the
 * map and edge u -> v exists iff we can ski from u to v (adjacent position,
 * and u at higher elevation than v). It's also acyclic as we can never go up
 * the slope back to a previous vertex. Leaf nodes are local minima, where ski
 * trips end.
 *
 * This allows us to use a depth-first search to visit nodes in reverse
 * topological order, and assign them an integer distance - the maximum distance
 * to a leaf node. This solves "question 1". At the same time we record the
 * maximum drop achievable from this node. This solves "question 2".
 */
struct NodeState {
    bool visited = false;
    // Maximum distance to any reachable leaf node.
    int distance = 0;
    // Maximum drop obtained by skiing one of the longest paths from this node.
    int drop = 0;
};

// Complexity: O(|V| + |E|) = O(N) where N is the number of points on the map.
void dfs(const ElevationMap& map, int v, std::vector<NodeState>& states) {
    std::vector<int> neighbors = map.lower_neighbors(v);
    for (int u: neighbors) {
        if (!states[u].visited) {
            dfs(map, u, states);
        }

        const int distance = states[u].distance + 1;
        const int drop = states[u].drop + map.data[v] - map.data[u];

        // A new maximum distance found will invalidate the previously computed
        // maximum drop.
        if (distance > states[v].distance) {
            states[v].distance = distance;
            states[v].drop = drop;
        } else if (distance == states[v].distance) {
            states[v].drop = std::max(states[v].drop, drop);
        }
    }
    states[v].visited = true;
}

void solve(const ElevationMap& map) {
    std::vector<NodeState> states(map.size);
    // max distance of skip trips (length of slope = distance + 1)
    int max_distance = 0;
    // max recorded drop for nodes which have max_distance.
    int max_drop = 0;

    for (int v = 0; v < map.size; ++v) {
        if (!states[v].visited) {
            dfs(map, v, states);
        }
        if (states[v].distance > max_distance) {
            max_distance = states[v].distance;
            max_drop = states[v].drop;
        } else if (states[v].distance == max_distance) {
            max_drop = std::max(max_drop, states[v].drop);
        }
    }

    // Print out the solution
    std::cout << "Length: " << max_distance + 1 << std::endl;
    std::cout << "Drop: " << max_drop << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: redmart /path/to/map/file" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Can't open " << argv[1] << std::endl;
        return 1;
    }

    ElevationMap map;
    map.read(file);
    solve(map);

    return 0;
}
