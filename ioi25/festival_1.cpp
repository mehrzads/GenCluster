#include <bits/stdc++.h>
using namespace std;

struct MultiCoupon {
    long long P;          // price
    int T;                // type (>1)
    int idx;              // original index
    long long S_num;      // Pi * Ti
    int S_den;            // Ti - 1
};

class SegTree {
    const vector<MultiCoupon>* arr;
    int n;
    vector<int> tree;   // stores index of best coupon, -1 means empty

    // compare two indices, return the better (smaller S, then smaller T, then smaller price)
    int better(int i, int j) const {
        if (i == -1) return j;
        if (j == -1) return i;
        const MultiCoupon& a = (*arr)[i];
        const MultiCoupon& b = (*arr)[j];
        __int128 lhs = (__int128)a.S_num * b.S_den;
        __int128 rhs = (__int128)b.S_num * a.S_den;
        if (lhs != rhs) return lhs < rhs ? i : j;          // smaller S
        if (a.T != b.T) return a.T < b.T ? i : j;        // smaller multiplier is better (less loss when S equal)
        if (a.P != b.P) return a.P < b.P ? i : j;        // smaller price
        return i;                                          // arbitrary
    }

    void build(int node, int l, int r) {
        if (l == r) {
            tree[node] = l;
            return;
        }
        int mid = (l + r) >> 1;
        build(node << 1, l, mid);
        build(node << 1 | 1, mid + 1, r);
        tree[node] = better(tree[node << 1], tree[node << 1 | 1]);
    }

    int query(int node, int l, int r, int ql, int qr) const {
        if (qr < l || r < ql) return -1;
        if (ql <= l && r <= qr) return tree[node];
        int mid = (l + r) >> 1;
        int left = query(node << 1, l, mid, ql, qr);
        int right = query(node << 1 | 1, mid + 1, r, ql, qr);
        return better(left, right);
    }

    void update(int node, int l, int r, int pos, int val) {
        if (l == r) {
            tree[node] = val;
            return;
        }
        int mid = (l + r) >> 1;
        if (pos <= mid) update(node << 1, l, mid, pos, val);
        else update(node << 1 | 1, mid + 1, r, pos, val);
        tree[node] = better(tree[node << 1], tree[node << 1 | 1]);
    }

public:
    SegTree() : arr(nullptr), n(0) {}
    explicit SegTree(const vector<MultiCoupon>* a) : arr(a) {
        n = (int)arr->size();
        if (n) {
            tree.assign(4 * n, -1);
            build(1, 0, n - 1);
        }
    }

    // query best index in [l,r]; returns -1 if none
    int query(int l, int r) const {
        if (n == 0 || l > r) return -1;
        return query(1, 0, n - 1, l, r);
    }

    // remove index pos (mark as unavailable)
    void remove(int pos) {
        if (n == 0) return;
        update(1, 0, n - 1, pos, -1);
    }
};

std::vector<int> max_coupons(int A, std::vector<int> P, std::vector<int> T) {
    const long long INF = (long long)4e18;   // large enough sentinel

    int N = (int)P.size();

    // separate coupons
    vector<MultiCoupon> mult;                 // T > 1
    vector<pair<long long,int>> cheap;        // T == 1  (price , index)

    for (int i = 0; i < N; ++i) {
        if (T[i] == 1) {
            cheap.emplace_back(P[i], i);
        } else {
            MultiCoupon c;
            c.P = P[i];
            c.T = T[i];
            c.idx = i;
            c.S_num = (long long)P[i] * T[i];
            c.S_den = T[i] - 1;
            mult.push_back(c);
        }
    }

    // sort cheap coupons by price and build prefix sums
    sort(cheap.begin(), cheap.end(),
         [](const pair<long long,int>& a, const pair<long long,int>& b){
             return a.first < b.first;
         });
    int C = (int)cheap.size();
    vector<long long> cheapPref(C+1, 0);
    for (int i = 0; i < C; ++i) cheapPref[i+1] = cheapPref[i] + cheap[i].first;

    auto cheapCount = [&](long long tokens)->int{
        // maximum number of cheap coupons we could afford (price-sum <= tokens)
        int pos = (int)(upper_bound(cheapPref.begin(), cheapPref.end(), tokens) - cheapPref.begin() - 1);
        return pos; // 0 .. C
    };

    // sort mult coupons by price (needed for binary search on affordability)
    sort(mult.begin(), mult.end(),
         [](const MultiCoupon& a, const MultiCoupon& b){
             return a.P < b.P;
         });
    int M = (int)mult.size();
    vector<long long> multPrice(M);
    for (int i = 0; i < M; ++i) multPrice[i] = mult[i].P;

    // segment tree over mult to retrieve coupon with minimal S among a price prefix
    SegTree seg(&mult);

    vector<int> answer;
    long long cur = A;

    while (true) {
        // find rightmost index whose price <= cur
        int r = -1;
        if (M > 0) {
            auto it = upper_bound(multPrice.begin(), multPrice.end(), cur);
            r = (int)(it - multPrice.begin()) - 1;
        }
        if (r < 0) break;                     // no mult coupon affordable

        int bestIdx = seg.query(0, r);
        if (bestIdx == -1) break;              // all affordable mult coupons already taken

        const MultiCoupon& c = mult[bestIdx];

        // compute new token amount after taking this coupon
        __int128 tmp = (__int128)(cur - c.P) * c.T;
        long long newCur = (tmp > INF) ? INF : (long long)tmp;

        // counts of cheap coupons before and after this move
        int cheapNow = cheapCount(cur);
        int cheapAfter = cheapCount(newCur);

        if (newCur >= cur) { // net‑positive (or neutral)
            answer.push_back(c.idx);
            cur = newCur;
            seg.remove(bestIdx);
            continue;
        } else {
            if (cheapAfter >= cheapNow - 1) {
                // taking it does not reduce the total possible coupons
                answer.push_back(c.idx);
                cur = newCur;
                seg.remove(bestIdx);
                continue;
            } else {
                // would lose more than one cheap coupon – stop taking mult coupons
                break;
            }
        }
    }

    // finally take cheap coupons (type 1) in order of increasing price
    int pos = 0;
    while (pos < C && cheap[pos].first <= cur) {
        cur -= cheap[pos].first;
        answer.push_back(cheap[pos].second);
        ++pos;
    }

    return answer;
}