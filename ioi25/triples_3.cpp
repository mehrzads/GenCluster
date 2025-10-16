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
#include <vector>
#include <algorithm>
#include <cmath>

/*
  Construction based on a Sidon set (Bose–Chowla construction).
  We pick a prime p such that 2 * p * p - 2 <= M-1.
  For each i = 0 .. p-1 we define a_i = i * p + (i * i) % p.
  The set {a_i} is a Sidon set: all pairwise sums a_i + a_j are distinct.
  For each unordered pair (i,j) (i<j) we create a peak at position
      pos = a_i + a_j                // unique because of Sidon property
      height = a_j - a_i   (positive)
  All other positions get height 1.  The array length is
      N = max_pos + 1, where max_pos = 2 * (p * p - 1).
  Every triple of vertices (i<j<k) from the Sidon set yields a
  mythical triple of positions (a_i + a_j, a_i + a_k, a_j + a_k).
*/

std::vector<int> construct_range(int M, int K) {
    // Choose the largest prime p such that 2 * p * p - 2 <= M - 1
    // Pre‑computed list of primes up to 400.
    const int primes[] = {
        2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,
        73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,
        151,157,163,167,173,179,181,191,193,197,199,211,223,227,
        229,233,239,241,251,257,263,269,271,277,281,283,293,307,
        311,313,317,331,337,347,349,353,359,367,373,379,383,389,
        397
    };
    int bestP = 2;
    for (int p : primes) {
        if (2LL * p * p - 2 <= M - 1) bestP = p;
        else break;
    }
    int p = bestP;                     // prime to use
    // Build Sidon set S of size p
    std::vector<int> S(p);
    for (int i = 0; i < p; ++i) {
        int val = i * p + (int)((1LL * i * i) % p);
        S[i] = val;                    // values are in [0, p*p-1]
    }
    // Determine maximal index needed
    int maxVal = p * p - 1;            // maximum element in S
    int maxPos = 2 * maxVal;           // maximum sum of two elements
    int N = maxPos + 1;
    if (N > M) N = M;                 // safety, though it shouldn't happen

    std::vector<int> H(N, 1);          // fill with 1 (valid height)

    // For each unordered pair, set the appropriate height
    for (int i = 0; i < p; ++i) {
        for (int j = i + 1; j < p; ++j) {
            int pos = S[i] + S[j];
            if (pos < N) {
                H[pos] = S[j] - S[i]; // positive by construction
            }
        }
    }
    // Ensure all heights are within [1, N-1]
    for (int &h : H) {
        if (h < 1) h = 1;
        if (h > N - 1) h = N - 1;
    }
    return H;
}