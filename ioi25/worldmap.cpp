#include <bits/stdc++.h>
using namespace std;

vector<vector<int>> create_map(int N, int M, vector<int> A, vector<int> B) {
    // adjacency matrix
    vector<vector<char>> adj(N, vector<char>(N, 0));
    for (int i = 0; i < M; ++i) {
        int u = A[i] - 1;
        int v = B[i] - 1;
        adj[u][v] = adj[v][u] = 1;
    }

    // host list: for each vertex i, store neighbors j with i<j
    vector<vector<int>> host(N);
    for (int i = 0; i < M; ++i) {
        int u = A[i] - 1;
        int v = B[i] - 1;
        if (u > v) swap(u, v);
        host[u].push_back(v);
    }
    // optional: sort for determinism
    for (int i = 0; i < N; ++i) {
        sort(host[i].begin(), host[i].end());
    }

    // Build a spanning tree (BFS) rooted at vertex 0
    vector<int> parent(N, -1);
    vector<vector<int>> children(N);
    vector<char> seen(N, 0);
    queue<int> q;
    seen[0] = 1;
    q.push(0);
    while (!q.empty()) {
        int v = q.front(); q.pop();
        for (int u = 0; u < N; ++u) if (adj[v][u]) {
            if (!seen[u]) {
                seen[u] = 1;
                parent[u] = v;
                children[v].push_back(u);
                q.push(u);
            }
        }
    }

    // The problem guarantees a map exists, which implies the graph is connected.
    // (If not, the algorithm would miss some vertices.)

    // Compute heights for the nested rectangles
    vector<int> height(N, 0);
    function<int(int)> dfsHeight = [&](int v) -> int {
        int h = 3; // padding, host row, separator before children
        for (int c : children[v]) {
            int ch = dfsHeight(c);
            h += ch + 1; // child height + separator after child
        }
        height[v] = h;
        return h;
    };
    int totalRows = dfsHeight(0);
    int K = totalRows;                     // make square, width = K as well
    // Ensure width is enough for host edges
    int maxNeeded = 1;
    for (int i = 0; i < N; ++i) {
        int need = (int)host[i].size() * 2 + 1;
        maxNeeded = max(maxNeeded, need);
    }
    if (K < maxNeeded) K = maxNeeded; // just in case (should not happen)
    // Final K is max(totalRows, maxNeeded)
    // Allocate grid
    vector<vector<int>> grid(K, vector<int>(K, 1)); // temporary fill with 1

    // Recursive placement
    function<void(int,int)> place = [&](int v, int top) {
        // fill rectangle of size height[v] x K with color v+1
        int colStart = 0;
        for (int r = top; r < top + height[v]; ++r) {
            for (int c = colStart; c < K; ++c) {
                grid[r][c] = v + 1;
            }
        }
        // host row (second row of the rectangle)
        int hostRow = top + 1;
        for (size_t idx = 0; idx < host[v].size(); ++idx) {
            int col = 2 * (int)idx + 1; // guarantee col < K because K >= maxNeeded
            int u = host[v][idx];
            grid[hostRow][col] = u + 1;
        }
        // place children
        int cur = top + 3; // after padding, host row, separator before children
        for (int c : children[v]) {
            place(c, cur);
            cur += height[c];
            cur += 1; // separator row (already filled with v's color)
        }
    };
    place(0, 0);

    // The resulting grid is K x K
    return grid;
}