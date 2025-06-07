// whale.hpp
#ifndef WHALE_HPP
#define WHALE_HPP


#include "include/modules.hpp"
#include <algorithm>
#include <random>

extern std::mt19937 rng;

class Whale : public Solution {
private:
    const Config* cfg_;    

public:
    // 建構子：使用 Config 指標初始化
    explicit Whale(const Config& cfg)
    : cfg_(&cfg)
    {
        Solution base = GenerateInitialSolution(*cfg_);
        ss = std::move(base.ss);
        ms = std::move(base.ms);
        evaluate();
    }

    Whale update(const Whale& best, const Whale& randW, double a) const {
        Whale nxt = *this;
        int n = ss.size();
        std::uniform_real_distribution<double> uni(0.0,1.0);
        double r1 = uni(rng), r2 = uni(rng), p = uni(rng);
        double A = 2.0*a*r1 - a;           // 控制探索/開採
        double C = 2.0*r2;                // 加權因子

        if (p < 0.5) {
            if (std::abs(A) < 1.0) {
                // Encircle Best：改用帶 C 加權的 swap
                nxt = encircle_best(best, C);
            } else {
                // Encircle Random Whale：向隨機個體靠攏
                nxt = encircle_best(randW, C);
            }
        } else {
            // Discrete Spiral：子段多次反轉
            nxt = spiral_discrete();
        }
        // 評估一次
        nxt.evaluate();
        return nxt;
    }


    // encircle_best 改動：帶上 C，加權決定 swap 距離
    Whale encircle_best(const Whale& other, double C) const {
        Whale nxt = *this;
        int n = ss.size();
        std::uniform_int_distribution<int> dist(0, n - 1);
        // 根據 C 決定交換次數 k（C 越大，交換範圍越廣）
        int k = std::max(1, int(std::round(C * n / 2)));
        for (int t = 0; t < k; ++t) {
            int i = dist(rng);
            // 找 other 在這位置的任務
            int gene = other.ss[i];
            int j = std::find(nxt.ss.begin(), nxt.ss.end(), gene) - nxt.ss.begin();
            std::swap(nxt.ss[i], nxt.ss[j]);
            nxt.ms[nxt.ss[i]] = rng() % cfg_->thePCount;
            nxt.ms[nxt.ss[j]] = rng() % cfg_->thePCount;
        }
        return nxt;
    }

    // 離散螺旋：在同一區段做多次反轉，模擬螺旋路徑
    Whale spiral_discrete() const {
        Whale nxt = *this;
        int n = ss.size();
        std::uniform_int_distribution<int> dist(0, n - 1);
        int a = dist(rng), b = dist(rng);
        if (a > b) std::swap(a, b);
        // 做 2~4 次子段反轉
        int times = 2 + (rng() % 3);
        while (times--) {
            std::reverse(nxt.ss.begin() + a, nxt.ss.begin() + b + 1);
        }
        for (int k = a; k <= b; ++k)
            nxt.ms[nxt.ss[k]] = rng() % cfg_->thePCount;
        return nxt;
    }

    // Order Crossover：固定用 global best 做交叉，提升收斂
    Whale order_crossover_best(const Whale& best) const {
        int n = ss.size();
        std::uniform_int_distribution<int> dist(0, n - 1);
        int a = dist(rng), b = dist(rng);
        if (a > b) std::swap(a, b);

        Whale child(*this);
        child.ss.assign(n, -1);
        child.ms.assign(n, -1);

        // 複製 best 的 ss[a..b]
        for (int i = a; i <= b; ++i) {
            child.ss[i] = best.ss[i];
            child.ms[i] = ms[best.ss[i]];
        }
        // 填剩餘
        int idx = (b + 1) % n;
        for (int g = 1; g <= n; ++g) {
            int gene = ss[(b + g) % n];
            if (std::find(child.ss.begin(), child.ss.end(), gene) == child.ss.end()) {
                child.ss[idx] = gene;
                child.ms[idx] = rng() % cfg_->thePCount;
                idx = (idx + 1) % n;
            }
        }
        return child;
    }

    double getCost() const { return cost; }


    


    void evaluate() {
        auto res = Solution_Function(*this, *cfg_);
        cost = res.makespan;
    }
};

#endif // WHALE_HPP
