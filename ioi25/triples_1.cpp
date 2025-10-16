#include <bits/stdc++.h>
using namespace std;

long long count_triples(vector<int> H) {
    int N = (int)H.size();

    // auxiliary arrays
    vector<int> L(N), D(N);
    vector<vector<int>> left(N);                     // left[v] : i with i+H[i]=v
    vector<vector<int>> right(N);                    // not needed, only sets
    vector<unordered_set<int>> rightVals(N);         // set of a = H[k] for each v = k-H[k]

    for (int i = 0; i < N; ++i) {
        L[i] = i + H[i];
        if (L[i] >= 0 && L[i] < N) left[L[i]].push_back(i);
        int v = i - H[i];
        if (v >= 0 && v < N) {
            right[v].push_back(i);
            rightVals[v].insert(H[i]);               // a = H[i] = k - v
        }
        D[i] = H[i] - i;
    }

    // groups by D value
    unordered_map<int, vector<int>> dGroups;
    dGroups.reserve(N * 2);
    for (int i = 0; i < N; ++i) dGroups[D[i]].push_back(i);

    // ----- pattern A -------------------------------------------------
    long long cntA = 0;
    for (int j = 0; j < N; ++j) {
        int i = j - H[j];
        if (i < 0) continue;
        int b = H[i] - H[j];
        if (b <= 0) continue;
        int k = j + b;
        if (k < N && H[k] == b) ++cntA;
    }

    // ----- pattern B -------------------------------------------------
    long long cntB = 0;
    for (int j = 0; j < N; ++j) {
        int k = j + H[j];
        if (k >= N) continue;
        int a = H[k];
        int i = j - a;
        if (i >= 0 && H[i] == a + H[j]) ++cntB;
    }

    // ----- pattern C -------------------------------------------------
    long long cntC = 0;
    for (int i = 0; i < N; ++i) {
        int j = i + H[i];
        if (j >= N) continue;
        int b = H[j];
        int k = j + b;
        if (k < N && H[k] == H[i] + b) ++cntC;
    }

    // ----- pattern D -------------------------------------------------
    long long cntD = 0;
    for (int j = 0; j < N; ++j) {
        int i = j - H[j];
        if (i < 0) continue;
        int b = H[i];
        int k = j + b;
        if (k < N && H[k] == b + H[j]) ++cntD;
    }

    // ----- pattern E (cntLeft) ----------------------------------------
    long long cntLeft = 0;   // pattern E
    for (int j = 0; j < N; ++j) {
        int sum = H[j];
        for (int i : left[j]) {
            int k = i + sum;
            if (k < N && (k - H[k]) == j) ++cntLeft;
        }
    }

    // ----- pattern F (cntF) ------------------------------------------
    long long cntF = 0;      // pattern F
    for (auto &kv : dGroups) {
        const vector<int> &vec = kv.second; // already sorted
        int sz = (int)vec.size();
        for (int pos = 0; pos < sz; ++pos) {
            int i = vec[pos];
            int v = L[i];
            if (v < 0 || v >= N) continue;
            const unordered_set<int> &setA = rightVals[v];
            if (setA.empty()) continue;

            int remaining = sz - pos - 1;
            // iterate over the smaller of the two sets
            if (remaining <= (int)setA.size()) {
                for (int idx = pos + 1; idx < sz; ++idx) {
                    int j = vec[idx];
                    int a = j - i;
                    if (setA.find(a) != setA.end()) ++cntF;
                }
            } else {
                for (int a : setA) {
                    int j = i + a;
                    if (j > i && j < N && D[j] == D[i]) ++cntF;
                }
            }
        }
    }

    // ----- duplicate triples (a = b) ----------------------------------
    long long dup = 0;
    for (int j = 0; j < N; ++j) {
        int a = H[j];
        int i = j - a;
        int k = j + a;
        if (i >= 0 && k < N) {
            if (H[i] == 2 * a && H[k] == a) ++dup;
            else if (H[i] == a && H[k] == 2 * a) ++dup;
        }
        if (a % 2 == 0) {
            int a2 = a / 2;
            i = j - a2;
            k = j + a2;
            if (i >= 0 && k < N && H[i] == a2 && H[k] == a2) ++dup;
        }
    }

    long long answer = cntA + cntB + cntC + cntD + cntLeft + cntF - dup;
    return answer;
}

#include <bits/stdc++.h>
using namespace std;

struct Triple {
    uint16_t i, j, k;
    uint16_t d1, d3; // d1 = j-i, d3 = k-j
};

inline bool isMyth(int a, int b, int c, int d1, int d3) {
    int d2 = d1 + d3;
    if (a == d2) {
        return (b == d1 && c == d3) || (b == d3 && c == d1);
    }
    if (b == d2) {
        return (a == d1 && c == d3) || (a == d3 && c == d1);
    }
    if (c == d2) {
        return (a == d1 && b == d3) || (a == d3 && b == d1);
    }
    return false;
}

// count all triples (full check) – used only at the end
static long long fullCount(const vector<int> &H) {
    int N = (int)H.size();
    long long cnt = 0;
    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            int d1 = j - i;
            for (int k = j + 1; k < N; ++k) {
                int d3 = k - j;
                if (isMyth(H[i], H[j], H[k], d1, d3)) ++cnt;
            }
        }
    }
    return cnt;
}

std::vector<int> construct_range(int M, int K) {
    const int N = M;                // use all allowed positions
    const int D = 30;               // maximal distance we consider for local search
    const int maxHeight = 2 * D;    // all generated heights stay in range

    // -------------------- precompute small triples (d1<=D, d3<=D) --------------------
    vector<Triple> triples;
    triples.reserve((size_t)N * D * D);
    vector<vector<int>> adj(N);
    for (int i = 0; i < N; ++i) {
        for (int d1 = 1; d1 <= D && i + d1 < N; ++d1) {
            int j = i + d1;
            for (int d3 = 1; d3 <= D && j + d3 < N; ++d3) {
                int k = j + d3;
                Triple tr;
                tr.i = (uint16_t)i;
                tr.j = (uint16_t)j;
                tr.k = (uint16_t)k;
                tr.d1 = (uint16_t)d1;
                tr.d3 = (uint16_t)d3;
                int id = (int)triples.size();
                triples.push_back(tr);
                adj[i].push_back(id);
                adj[j].push_back(id);
                adj[k].push_back(id);
            }
        }
    }
    const int Tcnt = (int)triples.size();
    vector<char> mythSmall(Tcnt, 0); // 0/1 flag for each small triple

    // -------------------- initial height array (pattern 2,1,1) --------------------
    vector<int> H(N);
    const int pat[3] = {2, 1, 1};
    for (int i = 0; i < N; ++i) H[i] = pat[i % 3];

    // -------------------- initial mythical status for small triples --------------------
    long long curSmall = 0;
    for (int id = 0; id < Tcnt; ++id) {
        const Triple &tr = triples[id];
        bool ok = isMyth(H[tr.i], H[tr.j], H[tr.k], tr.d1, tr.d3);
        mythSmall[id] = ok;
        if (ok) ++curSmall;
    }

    // -------------------- simulated annealing / hill climbing --------------------
    std::mt19937 rng((unsigned)chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> distPos(0, N - 1);
    std::uniform_int_distribution<int> distHeight(1, maxHeight);
    std::uniform_int_distribution<int> distTriple(0, Tcnt - 1);
    std::uniform_real_distribution<double> distReal(0.0, 1.0);

    const int perms[6][3] = {
        {0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}
    };

    double temperature = 1.0;
    const double cooling = 0.99998;
    const double timeLimit = 1.7; // seconds
    auto startAll = chrono::steady_clock::now();

    // best solution remembered
    vector<int> bestH = H;
    long long bestSmall = curSmall;

    while (curSmall < K) {
        // time check
        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration<double>(now - startAll).count();
        if (elapsed > timeLimit) break;

        // pick a random small triple
        int tid = distTriple(rng);
        const Triple &tr = triples[tid];
        int idx[3] = {tr.i, tr.j, tr.k};
        int vals[3] = {tr.d1, tr.d3, tr.d1 + tr.d3};

        // build union of affected triple ids
        static vector<int> unionIds;
        unionIds.clear();
        unionIds.reserve(adj[idx[0]].size() + adj[idx[1]].size() + adj[idx[2]].size());
        unionIds.insert(unionIds.end(), adj[idx[0]].begin(), adj[idx[0]].end());
        unionIds.insert(unionIds.end(), adj[idx[1]].begin(), adj[idx[1]].end());
        unionIds.insert(unionIds.end(), adj[idx[2]].begin(), adj[idx[2]].end());
        sort(unionIds.begin(), unionIds.end());
        unionIds.erase(unique(unionIds.begin(), unionIds.end()), unionIds.end());

        int bestDelta = INT_MIN;
        int bestPerm = -1;

        // try all 6 permutations
        for (int p = 0; p < 6; ++p) {
            int nh[3];
            nh[0] = vals[perms[p][0]];
            nh[1] = vals[perms[p][1]];
            nh[2] = vals[perms[p][2]];
            // quick reject if any height out of allowed range (shouldn't happen)
            if (nh[0] < 1 || nh[0] > N-1) continue;
            if (nh[1] < 1 || nh[1] > N-1) continue;
            if (nh[2] < 1 || nh[2] > N-1) continue;

            int delta = 0;
            for (int uid : unionIds) {
                const Triple &t = triples[uid];
                int a = H[t.i];
                int b = H[t.j];
                int c = H[t.k];
                // replace if index matches
                if (t.i == idx[0]) a = nh[0];
                else if (t.i == idx[1]) a = nh[1];
                else if (t.i == idx[2]) a = nh[2];

                if (t.j == idx[0]) b = nh[0];
                else if (t.j == idx[1]) b = nh[1];
                else if (t.j == idx[2]) b = nh[2];

                if (t.k == idx[0]) c = nh[0];
                else if (t.k == idx[1]) c = nh[1];
                else if (t.k == idx[2]) c = nh[2];

                bool newStat = isMyth(a, b, c, t.d1, t.d3);
                bool oldStat = mythSmall[uid];
                delta += (newStat ? 1 : 0) - (oldStat ? 1 : 0);
            }
            if (delta > bestDelta) {
                bestDelta = delta;
                bestPerm = p;
            }
        }

        if (bestPerm == -1) continue; // should not happen

        // decide acceptance
        bool accept = false;
        if (bestDelta > 0) accept = true;
        else {
            double prob = exp(bestDelta / temperature);
            if (distReal(rng) < prob) accept = true;
        }

        if (accept) {
            int nh[3];
            nh[0] = vals[perms[bestPerm][0]];
            nh[1] = vals[perms[bestPerm][1]];
            nh[2] = vals[perms[bestPerm][2]];

            // apply changes and update myth flags
            for (int uid : unionIds) {
                const Triple &t = triples[uid];
                int a = H[t.i];
                int b = H[t.j];
                int c = H[t.k];
                if (t.i == idx[0]) a = nh[0];
                else if (t.i == idx[1]) a = nh[1];
                else if (t.i == idx[2]) a = nh[2];

                if (t.j == idx[0]) b = nh[0];
                else if (t.j == idx[1]) b = nh[1];
                else if (t.j == idx[2]) b = nh[2];

                if (t.k == idx[0]) c = nh[0];
                else if (t.k == idx[1]) c = nh[1];
                else if (t.k == idx[2]) c = nh[2];

                bool newStat = isMyth(a, b, c, t.d1, t.d3);
                mythSmall[uid] = newStat;
            }
            // update heights
            H[idx[0]] = nh[0];
            H[idx[1]] = nh[1];
            H[idx[2]] = nh[2];
            curSmall += bestDelta;
            if (curSmall > bestSmall) {
                bestSmall = curSmall;
                bestH = H;
            }
        }

        temperature *= cooling;
    }

    // If we stopped early without reaching K, keep the best we have
    if (curSmall < K) {
        H = bestH;
        curSmall = bestSmall;
    }

    // Final verification (optional)
    // long long total = fullCount(H);
    // if (total < K) {
    //     // fallback: just return current H (partial score)
    // }

    return H;
}
