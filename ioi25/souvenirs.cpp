#include "souvenirs.h"
#include <bits/stdc++.h>
using namespace std;

const int MAXN = 105;          // N ≤ 100

struct Equation {
    bitset<MAXN> mask;   // unknown indices (0‑based for types 1..N‑1)
    long long rhs;       // sum of their prices
};

int N_;
long long P0_;

vector<long long> price;          // -1 while unknown, size N (0..N‑1)
vector<int> bought;               // how many copies we already have
vector<Equation> eqs;             // linear equations for unknowns

/*---------------------------------------------------------------*/

static long long min_unknown_higher_price(int idx)   // smallest known price among types < idx, -1 if none
{
    long long best = -1;
    for (int i = 0; i < idx; ++i) {
        if (price[i] != -1) {
            if (best == -1 || price[i] < best) best = price[i];
        }
    }
    return best;                 // -1 means "none known"
}

/* try to solve all equations that became singletons */
static void reduce_system() {
    bool changed = true;
    while (changed) {
        changed = false;
        for (size_t e = 0; e < eqs.size(); ++e) {
            if (eqs[e].mask.none()) continue;
            if (eqs[e].mask.count() == 1) {
                int idx = (int)eqs[e].mask._Find_first();   // index 0..N-2  (type = idx+1)
                long long val = eqs[e].rhs;
                if (price[idx] == -1) {
                    price[idx] = val;
                }
                // substitute into all equations
                for (size_t k = 0; k < eqs.size(); ++k) {
                    if (eqs[k].mask[idx]) {
                        eqs[k].rhs -= val;
                        eqs[k].mask.reset(idx);
                    }
                }
                // also subtract from bought counts –
                // the copy obtained in the transaction that gave this equation
                // has already been counted when the transaction was processed.
                eqs[e].mask.reset();   // make it empty
                changed = true;
            }
        }
    }
}

/*---------------------------------------------------------------*/

void buy_souvenirs(int N, long long P0) {
    N_ = N;
    P0_ = P0;

    price.assign(N, -1);          // price[0] is known (P0), others -1
    price[0] = P0;
    bought.assign(N, 0);          // counts for all types, we never buy type 0

    // ---------- discovery phase ----------
    // start with a safe upper bound for the cheapest unknown price
    long long UB_initial = P0 - (N - 1);          // Lemma 1

    // we will repeat until all prices (1..N-1) are known
    while (true) {
        bool all_known = true;
        for (int i = 1; i < N; ++i) if (price[i] == -1) { all_known = false; break; }
        if (all_known) break;

        // 1) compute an upper bound for the smallest unknown price
        long long bound1 = (long long)4e18;   // INF
        // find the smallest known price among indices smaller than the current smallest unknown
        int smallest_unknown = -1;
        for (int i = N - 1; i >= 1; --i) if (price[i] == -1) { smallest_unknown = i; break; }
        long long higher = min_unknown_higher_price(smallest_unknown);
        if (higher != -1) bound1 = higher - 1;          // must be < any more expensive known price

        long long bound2 = (long long)4e18;
        for (auto &eq : eqs) {
            if (eq.mask.none()) continue;
            long long cnt = (long long)eq.mask.count();
            long long cand = eq.rhs / cnt;   // floor
            if (cand < bound2) bound2 = cand;
        }

        long long UB = min(bound1, bound2);
        if (UB == (long long)4e18) UB = UB_initial;    // fallback, should not happen

        // 2) perform the transaction
        auto resp = transaction(UB);
        const vector<int> &L = resp.first;
        long long R = resp.second;
        long long sum = UB - R;          // Σ P[t] over all bought types

        // 3) update data structures
        bitset<MAXN> curMask;
        for (int t : L) {
            ++bought[t];                 // we obtained one copy of this type
            int idx = t;                 // type number = index (0..N-1)
            if (idx == 0) continue;     // we never get type 0 because M < P0
            if (price[idx] != -1) {
                sum -= price[idx];       // known contribution
            } else {
                curMask.set(idx);
            }
        }

        if (curMask.any()) {
            Equation ne;
            ne.mask = curMask;
            ne.rhs = sum;
            eqs.push_back(ne);
        }

        // 4) try to solve singletons that appeared
        reduce_system();
    }

    // ---------- finishing purchases ----------
    for (int i = 1; i < N; ++i) {
        int need = i - bought[i];
        for (int k = 0; k < need; ++k) {
            transaction(price[i]);      // buys exactly one copy of type i
        }
    }
}