#include "migrations.h"
#include <bits/stdc++.h>
using namespace std;

static const int LOG = 15;                // 2^14 = 16384 > 10000

/* ------------------------------------------------------------------ */
/*  Team side: send_message                                          */
/* ------------------------------------------------------------------ */
int send_message(int N, int i, int Pi) {
    // static data that survives across calls of the same test case
    static bool initialized = false;
    static int Nglob = -1;
    static vector<int> depth;
    static vector<array<int, LOG>> up;
    static int cur_u, cur_v, cur_d;        // current diameter ends and length
    static bool vChangedAtNminus2;         // true iff cur_v became N-2 at step N-2

    // first call for a test case (i == 1) -> initialise everything
    if (!initialized || i == 1) {
        initialized = true;
        Nglob = N;
        depth.assign(N, 0);
        up.assign(N, {});
        for (int k = 0; k < LOG; ++k) up[0][k] = 0;   // ancestors of root are root
        cur_u = cur_v = 0;
        cur_d = 0;
        vChangedAtNminus2 = false;
    }

    /* ---- add node i to the tree ----------------------------------- */
    depth[i] = depth[Pi] + 1;
    up[i][0] = Pi;
    for (int k = 1; k < LOG; ++k) {
        int anc = up[i][k - 1];
        up[i][k] = up[anc][k - 1];
    }

    // helper: LCA using binary lifting
    auto lca = [&](int a, int b) -> int {
        if (depth[a] < depth[b]) swap(a, b);
        int diff = depth[a] - depth[b];
        for (int k = LOG - 1; k >= 0; --k)
            if (diff & (1 << k)) a = up[a][k];
        if (a == b) return a;
        for (int k = LOG - 1; k >= 0; --k) {
            if (up[a][k] != up[b][k]) {
                a = up[a][k];
                b = up[b][k];
            }
        }
        return up[a][0];
    };
    auto dist = [&](int a, int b) -> int {
        int w = lca(a, b);
        return depth[a] + depth[b] - 2 * depth[w];
    };

    // update the diameter
    int du = dist(i, cur_u);
    int dv = dist(i, cur_v);
    if (du > cur_d) {
        cur_v = i;
        cur_d = du;
    } else if (dv > cur_d) {
        cur_u = i;
        cur_d = dv;
    }

    /* ---- decide what to send ------------------------------------- */
    // tiny instances
    if (N <= 3) {
        if (i == N - 1) {
            // encode the final pair (cur_u,cur_v) into a small integer
            int code = cur_u * N + cur_v + 1;   // 1 … N*N (≤ 9 for N=3)
            return code;
        }
        return 0;
    }

    // N >= 4
    if (i == Nglob - 3) {
        // store the current cur_v (after processing i = N-3)
        return cur_v + 1;
    }
    if (i == Nglob - 2) {
        // remember whether cur_v became the new leaf N-2
        vChangedAtNminus2 = (cur_v == i);
        // store cur_u after processing i = N-2
        return cur_u + 1;
    }
    if (i == Nglob - 1) {
        int c1 = 0;                 // change type at the last step
        if (cur_u == i) c1 = 2;
        else if (cur_v == i) c1 = 1;
        else c1 = 0;
        int c2 = vChangedAtNminus2 ? 1 : 0;
        int flag = 1 + c1 + 3 * c2; // value between 1 and 6
        return flag;
    }
    return 0;                       // no message
}

/* ------------------------------------------------------------------ */
/*  Museum side: longest_path                                        */
/* ------------------------------------------------------------------ */
std::pair<int, int> longest_path(std::vector<int> S) {
    int N = (int)S.size();

    if (N == 1) return {0, 0};
    if (N == 2) return {0, 1};

    if (N == 3) {
        // the only non‑zero entry is S[2]
        int code = S[2];
        int x = code - 1;
        int u = x / N;
        int v = x % N;
        return {u, v};
    }

    // N >= 4
    int u_saved = S[N - 2] - 1;   // cur_u after step N-2
    int v_saved = S[N - 3] - 1;   // cur_v after step N-3 (may be outdated)
    int flag = S[N - 1] - 1;      // 0 … 5

    int c1 = flag % 3;            // 0 unchanged, 1 cur_v → N-1, 2 cur_u → N-1
    int c2 = flag / 3;            // 0 cur_v not N-2, 1 cur_v = N-2 after N-2

    int other = (c2 == 1) ? (N - 2) : v_saved;

    if (c1 == 0) {
        return {u_saved, other};
    } else if (c1 == 1) {
        return {u_saved, N - 1};
    } else { // c1 == 2
        return {other, N - 1};
    }
}
