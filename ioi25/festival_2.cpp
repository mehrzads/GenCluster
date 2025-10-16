#include "festival.h"
#include <bits/stdc++.h>
using namespace std;

struct Multiplier {
    int idx;            // original index
    int price;
    int mult;           // T > 1
    long long delta;    // T*price - (T-1)*A
};

struct Node {
    int couponIdx;      // index of the multiplier used to reach this state
    int prev;           // index of previous node in the chain, -1 for none
};

vector<int> max_coupons(int A, vector<int> P, vector<int> T) {
    const long long INF = (1LL << 60);
    int N = (int)P.size();

    /* separate cheap (T==1) and multipliers (T>1) */
    vector<pair<int,int>> cheap;        // (price, idx)
    vector<Multiplier> mults;
    cheap.reserve(N);
    mults.reserve(N);

    for (int i = 0; i < N; ++i) {
        if (T[i] == 1) {
            cheap.emplace_back(P[i], i);
        } else {
            long long delta = 1LL * T[i] * P[i] - 1LL * (T[i] - 1) * A; // >0
            mults.push_back({i, P[i], T[i], delta});
        }
    }

    /* sort cheap coupons by price (they will be taken after multipliers) */
    sort(cheap.begin(), cheap.end(),
         [](const pair<int,int>& a, const pair<int,int>& b){
             if (a.first != b.first) return a.first < b.first;
             return a.second < b.second;
         });
    int C = (int)cheap.size();
    vector<long long> cheapPref(C + 1, 0);
    for (int i = 1; i <= C; ++i)
        cheapPref[i] = cheapPref[i-1] + cheap[i-1].first;

    /* sort multipliers by increasing ratio R = P*T/(T-1) */
    auto cmpRatio = [](const Multiplier& a, const Multiplier& b)->bool{
        // compare a.price * a.mult * (b.mult-1)  ?  b.price * b.mult * (a.mult-1)
        long long left  = 1LL * a.price * a.mult * (b.mult - 1);
        long long right = 1LL * b.price * b.mult * (a.mult - 1);
        if (left != right) return left < right;
        if (a.price != b.price) return a.price < b.price;
        return a.idx < b.idx;
    };
    sort(mults.begin(), mults.end(), cmpRatio);
    int M = (int)mults.size();

    /* DP: dp[k] = minimal Y after taking k multipliers */
    vector<long long> dp(M + 1, INF);
    vector<int> nodeIdx(M + 1, -1);   // back‑pointer chain for reconstruction
    dp[0] = 0;
    int curMax = 0;                  // largest k with dp[k] < INF

    vector<Node> nodes;
    nodes.reserve(M * 30);           // enough for all updates

    for (int m = 0; m < M; ++m) {
        const Multiplier& cur = mults[m];
        for (int k = curMax; k >= 0; --k) {
            if (dp[k] == INF) continue;
            long long newY = dp[k] * cur.mult + cur.delta;
            if (newY <= A && newY < dp[k + 1]) {
                dp[k + 1] = newY;
                nodes.push_back({cur.idx, nodeIdx[k]});
                nodeIdx[k + 1] = (int)nodes.size() - 1;
            }
        }
        while (curMax + 1 <= M && dp[curMax + 1] != INF) ++curMax;
    }

    /* choose best total number of coupons */
    int bestK = 0, bestCheap = 0, bestTotal = 0;
    for (int k = 0; k <= curMax; ++k) {
        if (dp[k] == INF) continue;
        long long remain = (long long)A - dp[k];
        // maximal cheap coupons we can afford
        int cheapCnt = (int)(upper_bound(cheapPref.begin(), cheapPref.end(), remain) - cheapPref.begin()) - 1;
        int total = k + cheapCnt;
        if (total > bestTotal) {
            bestTotal = total;
            bestK = k;
            bestCheap = cheapCnt;
        }
    }

    /* reconstruct the list of multipliers */
    vector<int> answer;
    vector<int> multChosen;
    int nd = nodeIdx[bestK];
    while (nd != -1) {
        multChosen.push_back(nodes[nd].couponIdx);
        nd = nodes[nd].prev;
    }
    reverse(multChosen.begin(), multChosen.end());
    answer.insert(answer.end(), multChosen.begin(), multChosen.end());

    /* add the cheapest cheap coupons */
    for (int i = 0; i < bestCheap; ++i)
        answer.push_back(cheap[i].second);

    return answer;
}