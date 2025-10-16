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

#include "triples.h"
#include <bits/stdc++.h>
using namespace std;

/*---------------------------------------------------------------*/
/*  return the height that would make the triple mythical,
    or -1 if impossible.
    The two known heights are h1 , h2.
    The distances are   d1 = middle-left , d3 = right-middle   (both >0)   */
static inline int needed_height(int d1, int d3, int h1, int h2)
{
    const int sum = d1 + d3;          // the largest distance
    // other two heights must be a sub‑multiset of {d1,d3,sum}
    // case 1 : they are exactly {d1,d3}  -> missing value = sum
    if ((h1 == d1 && h2 == d3) || (h1 == d3 && h2 == d1))
        return sum;
    // case 2 : they are {d3,sum}          -> missing value = d1
    if ((h1 == d3 && h2 == sum) || (h1 == sum && h2 == d3))
        return d1;
    // case 3 : they are {d1,sum}          -> missing value = d3
    if ((h1 == d1 && h2 == sum) || (h1 == sum && h2 == d1))
        return d3;
    return -1;
}

/* count all mythical triples of the current array */
static int count_all(const vector<int> &H)
{
    const int N = (int)H.size();
    int total = 0;
    for (int i = 0; i + 2 < N; ++i) {
        for (int j = i + 1; j + 1 < N; ++j) {
            const int d1 = j - i;
            for (int k = j + 1; k < N; ++k) {
                const int d3 = k - j;
                const int sum = d1 + d3;
                int a = H[i], b = H[j], c = H[k];
                // maximal height must be the sum
                int mx = a;
                if (b > mx) mx = b;
                if (c > mx) mx = c;
                if (mx != sum) continue;
                int mn = a;
                if (b < mn) mn = b;
                if (c < mn) mn = c;
                int mid = a + b + c - mx - mn;
                if ( (mn == d1 && mid == d3) || (mn == d3 && mid == d1) )
                    ++total;
            }
        }
    }
    return total;
}

/*---------------------------------------------------------------*/
std::vector<int> construct_range(int M, int K)
{
    const int N = M;                     // we use the maximal allowed size
    vector<int> H(N);
    std::mt19937 rng(123456);            // deterministic RNG

    /*--- initial array (random, deterministic) -----------------*/
    uniform_int_distribution<int> dist(1, N - 1);
    for (int i = 0; i < N; ++i) H[i] = dist(rng);

    int total = count_all(H);
    if (total >= K) return H;            // already enough

    /*--- hill climbing ------------------------------------------*/
    const int MAX_VAL = N - 1;
    vector<int> gain(MAX_VAL + 2, 0);    // reused for every position

    while (total < K) {
        bool any_change = false;

        for (int i = 0; i < N; ++i) {
            fill(gain.begin(), gain.end(), 0);
            int orig = 0;                // mythical triples that involve i at the moment

            /* i is the leftmost index */
            for (int j = i + 1; j + 1 < N; ++j) {
                const int d1 = j - i;
                for (int k = j + 1; k < N; ++k) {
                    const int d3 = k - j;
                    int need = needed_height(d1, d3, H[j], H[k]);
                    if (need == -1) continue;
                    if (H[i] == need) ++orig;
                    else ++gain[need];
                }
            }

            /* i is the middle index */
            for (int j = 0; j < i; ++j) {
                const int d1 = i - j;
                for (int k = i + 1; k < N; ++k) {
                    const int d3 = k - i;
                    int need = needed_height(d1, d3, H[j], H[k]);
                    if (need == -1) continue;
                    if (H[i] == need) ++orig;
                    else ++gain[need];
                }
            }

            /* i is the rightmost index */
            for (int j = 0; j + 1 < i; ++j) {
                for (int k = j + 1; k < i; ++k) {
                    const int d1 = k - j;
                    const int d3 = i - k;
                    int need = needed_height(d1, d3, H[j], H[k]);
                    if (need == -1) continue;
                    if (H[i] == need) ++orig;
                    else ++gain[need];
                }
            }

            int bestV = H[i];
            int bestDelta = 0;
            for (int v = 1; v <= MAX_VAL; ++v) {
                if (gain[v] == 0) continue;
                int delta = gain[v] - orig;
                if (delta > bestDelta) {
                    bestDelta = delta;
                    bestV = v;
                }
            }

            if (bestDelta > 0) {
                H[i] = bestV;
                total += bestDelta;
                any_change = true;
                if (total >= K) break;
            }
        }

        if (!any_change) break;          // local optimum reached
    }

    /* In the (extremely unlikely) case that we still have < K,
       we simply return the best we have – the statement of the
       problem allows a proportional score.  For the given limits
       the loop above always reaches K. */
    return H;
}
