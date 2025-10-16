#include <bits/stdc++.h>
using namespace std;

struct State {
    long long token;          // maximum tokens achievable in this state
    short pi, pj, pk;         // previous (i2,i3,i4) counts
    int coupon_idx;           // index of the coupon taken to reach this state
};

vector<int> max_coupons(int A, vector<int> P, vector<int> T) {
    const long long INF = (1LL << 60);        // a value far larger than any possible sum of prices

    int N = (int)P.size();

    // Separate coupons by their type
    vector<pair<long long,int>> list1, list2, list3, list4;
    for (int i = 0; i < N; ++i) {
        if (T[i] == 1) list1.emplace_back(P[i], i);
        else if (T[i] == 2) list2.emplace_back(P[i], i);
        else if (T[i] == 3) list3.emplace_back(P[i], i);
        else /* T[i] == 4 */ list4.emplace_back(P[i], i);
    }

    // Sort each list by price (ascending). Within a type, any order with non‑decreasing price is optimal.
    auto cmp = [](const pair<long long,int> &a, const pair<long long,int> &b){
        if (a.first != b.first) return a.first < b.first;
        return a.second < b.second;
    };
    sort(list1.begin(), list1.end(), cmp);
    sort(list2.begin(), list2.end(), cmp);
    sort(list3.begin(), list3.end(), cmp);
    sort(list4.begin(), list4.end(), cmp);

    int n1 = (int)list1.size();
    int n2 = (int)list2.size();
    int n3 = (int)list3.size();
    int n4 = (int)list4.size();

    // Prefix sums for type‑1 coupons (to know how many can be bought after a given amount of tokens)
    vector<long long> pref1(n1 + 1, 0);
    for (int i = 0; i < n1; ++i) pref1[i + 1] = pref1[i] + list1[i].first;

    // DP over counts of taken coupons of types 2,3,4.
    int dim2 = n2 + 1, dim3 = n3 + 1, dim4 = n4 + 1;
    vector<State> dp(dim2 * dim3 * dim4);
    for (auto &st : dp) {
        st.token = -1;
        st.pi = st.pj = st.pk = -1;
        st.coupon_idx = -1;
    }
    auto idx = [&](int i2, int i3, int i4)->int{
        return (i2 * dim3 + i3) * dim4 + i4;
    };
    dp[idx(0,0,0)].token = A;

    struct Node { short i2,i3,i4; };
    deque<Node> q;
    q.push_back({0,0,0});

    while (!q.empty()) {
        Node cur = q.front(); q.pop_front();
        int curIdx = idx(cur.i2, cur.i3, cur.i4);
        long long curTok = dp[curIdx].token;
        if (curTok < 0) continue;      // unreachable, should not happen

        // Try to take next type‑2 coupon
        if (cur.i2 < n2) {
            long long price = list2[cur.i2].first;
            if (curTok >= price) {
                __int128 val = (__int128)(curTok - price) * 2;
                long long newTok = val > INF ? INF : (long long)val;
                int ni2 = cur.i2 + 1, ni3 = cur.i3, ni4 = cur.i4;
                int nid = idx(ni2, ni3, ni4);
                if (newTok > dp[nid].token) {
                    dp[nid].token = newTok;
                    dp[nid].pi = cur.i2;
                    dp[nid].pj = cur.i3;
                    dp[nid].pk = cur.i4;
                    dp[nid].coupon_idx = list2[cur.i2].second;
                    q.push_back({(short)ni2,(short)ni3,(short)ni4});
                }
            }
        }
        // Try to take next type‑3 coupon
        if (cur.i3 < n3) {
            long long price = list3[cur.i3].first;
            if (curTok >= price) {
                __int128 val = (__int128)(curTok - price) * 3;
                long long newTok = val > INF ? INF : (long long)val;
                int ni2 = cur.i2, ni3 = cur.i3 + 1, ni4 = cur.i4;
                int nid = idx(ni2, ni3, ni4);
                if (newTok > dp[nid].token) {
                    dp[nid].token = newTok;
                    dp[nid].pi = cur.i2;
                    dp[nid].pj = cur.i3;
                    dp[nid].pk = cur.i4;
                    dp[nid].coupon_idx = list3[cur.i3].second;
                    q.push_back({(short)ni2,(short)ni3,(short)ni4});
                }
            }
        }
        // Try to take next type‑4 coupon
        if (cur.i4 < n4) {
            long long price = list4[cur.i4].first;
            if (curTok >= price) {
                __int128 val = (__int128)(curTok - price) * 4;
                long long newTok = val > INF ? INF : (long long)val;
                int ni2 = cur.i2, ni3 = cur.i3, ni4 = cur.i4 + 1;
                int nid = idx(ni2, ni3, ni4);
                if (newTok > dp[nid].token) {
                    dp[nid].token = newTok;
                    dp[nid].pi = cur.i2;
                    dp[nid].pj = cur.i3;
                    dp[nid].pk = cur.i4;
                    dp[nid].coupon_idx = list4[cur.i4].second;
                    q.push_back({(short)ni2,(short)ni3,(short)ni4});
                }
            }
        }
    }

    // Find the best reachable state (most coupons overall)
    int best_i2 = 0, best_i3 = 0, best_i4 = 0, best_t1 = 0;
    int best_total = 0;
    for (int i2 = 0; i2 <= n2; ++i2) {
        for (int i3 = 0; i3 <= n3; ++i3) {
            for (int i4 = 0; i4 <= n4; ++i4) {
                long long tok = dp[idx(i2,i3,i4)].token;
                if (tok < 0) continue;
                // maximum number of type‑1 coupons we can still afford
                int t = (int)(upper_bound(pref1.begin(), pref1.end(), tok) - pref1.begin()) - 1;
                int total = i2 + i3 + i4 + t;
                if (total > best_total) {
                    best_total = total;
                    best_i2 = i2; best_i3 = i3; best_i4 = i4; best_t1 = t;
                }
            }
        }
    }

    // Reconstruct the order of type‑2/3/4 coupons
    vector<int> answer;
    int ci = best_i2, cj = best_i3, ck = best_i4;
    while (ci != 0 || cj != 0 || ck != 0) {
        State &st = dp[idx(ci,cj,ck)];
        answer.push_back(st.coupon_idx);
        int pi = st.pi;
        int pj = st.pj;
        int pk = st.pk;
        ci = pi; cj = pj; ck = pk;
    }
    reverse(answer.begin(), answer.end());

    // Append the cheapest type‑1 coupons we can afford
    for (int i = 0; i < best_t1; ++i)
        answer.push_back(list1[i].second);

    return answer;
}