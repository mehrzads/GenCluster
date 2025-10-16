#include <bits/stdc++.h>
using namespace std;

/***  segment tree for range maximum ***/
struct SegTree {
    int n;
    vector<int> seg;
    SegTree() {}
    SegTree(const vector<int> &a) { build(a); }
    void build(const vector<int> &a) {
        int sz = (int)a.size();
        n = 1;
        while (n < sz) n <<= 1;
        seg.assign(2 * n, INT_MIN);
        for (int i = 0; i < sz; ++i) seg[n + i] = a[i];
        for (int i = n - 1; i > 0; --i) seg[i] = max(seg[i << 1], seg[i << 1 | 1]);
    }
    // inclusive query [l, r]
    int query(int l, int r) const {
        l += n; r += n;
        int res = INT_MIN;
        while (l <= r) {
            if (l & 1) res = max(res, seg[l++]);
            if (!(r & 1)) res = max(res, seg[r--]);
            l >>= 1; r >>= 1;
        }
        return res;
    }
};

/***  DSU ***/
struct DSU {
    vector<int> p, r;
    DSU() {}
    DSU(int n) { init(n); }
    void init(int n) {
        p.resize(n);
        r.assign(n, 0);
        iota(p.begin(), p.end(), 0);
    }
    int find(int x) {
        if (p[x] == x) return x;
        return p[x] = find(p[x]);
    }
    void unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return;
        if (r[a] < r[b]) swap(a, b);
        p[b] = a;
        if (r[a] == r[b]) ++r[a];
    }
};

/***  global data for the queries ***/
static vector<int> colToSeed;   // -1 for non‑seed, otherwise seed index
static DSU dsu;
static vector<int> seedPos;     // column index of each seed (ordered by column)
static vector<int> maxDepthSeed; // maxDepth for each seed (by seed index)

/***  helpers ***/
static vector<int> prefMin;   // size N
static vector<int> prefMax;   // size N
static SegTree segH;          // range maximum over H

/* binary search: largest i with prefMin[i] > h   (seed guarantees existence) */
static int computeMaxDepth(int h) {
    int lo = 0, hi = (int)prefMin.size(); // hi == N means not found
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (prefMin[mid] > h) lo = mid + 1;
        else hi = mid;
    }
    // first index where prefMin <= h is 'lo'
    return lo - 1;            // may become N-1 if never <= h
}

/***  required functions ***/
void initialize(vector<int> T, vector<int> H) {
    int N = (int)T.size();
    int M = (int)H.size();

    /* prefix minima and maxima of temperatures */
    prefMin.resize(N);
    prefMax.resize(N);
    prefMin[0] = T[0];
    prefMax[0] = T[0];
    for (int i = 1; i < N; ++i) {
        prefMin[i] = min(prefMin[i - 1], T[i]);
        prefMax[i] = max(prefMax[i - 1], T[i]);
    }

    /* segment tree on H for interval maximum queries */
    segH.build(H);

    /* seeds */
    seedPos.clear();
    colToSeed.assign(M, -1);
    for (int j = 0; j < M; ++j) {
        if (H[j] < T[0]) {
            colToSeed[j] = (int)seedPos.size();
            seedPos.push_back(j);
        }
    }
    int K = (int)seedPos.size();
    dsu.init(K);
    maxDepthSeed.assign(K, -1);

    /* maxDepth for each seed */
    for (int idx = 0; idx < K; ++idx) {
        int col = seedPos[idx];
        int h = H[col];
        maxDepthSeed[idx] = computeMaxDepth(h);
    }

    /* order seeds by decreasing maxDepth */
    vector<int> order(K);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(),
         [&](int a, int b) {
             return maxDepthSeed[a] > maxDepthSeed[b];
         });

    /* active set of seed columns (ordered by column) */
    set<int> active;   // stores column numbers of already inserted seeds

    for (int idx : order) {
        int col = seedPos[idx];
        int d = maxDepthSeed[idx];          // depth up to which this seed works

        auto it = active.insert(col).first;

        // left neighbour
        if (it != active.begin()) {
            auto itL = prev(it);
            int colL = *itL;
            int idxL = colToSeed[colL];
            int intervalMax = segH.query(min(colL, col), max(colL, col));
            if (prefMax[d] > intervalMax) dsu.unite(idx, idxL);
        }
        // right neighbour
        auto itR = next(it);
        if (itR != active.end()) {
            int colR = *itR;
            int idxR = colToSeed[colR];
            int intervalMax = segH.query(min(col, colR), max(col, colR));
            if (prefMax[d] > intervalMax) dsu.unite(idx, idxR);
        }
    }
}

/* L and R are always 0 and M-1, they are ignored */
bool can_reach(int L, int R, int S, int D) {
    if (S == D) return true;
    int idS = colToSeed[S];
    int idD = colToSeed[D];
    if (idS == -1 || idD == -1) return false;   // should not happen
    return dsu.find(idS) == dsu.find(idD);
}