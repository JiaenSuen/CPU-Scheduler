// whale.hpp
#ifndef WHALE_HPP
#define WHALE_HPP

#include "include/modules.hpp"
#include <algorithm>
#include <random>

extern std::mt19937 rng;

 
class Whale : public Solution {
public:
    // 建構子：使用隨機初始解
    explicit Whale(const Config& cfg) : cfg_(&cfg) {
        auto base = GenerateInitialSolution(*cfg_);
        ss = std::move(base.ss);
        ms = std::move(base.ms);
        evaluate();
    }

    // 取得成本
    double getCost() const { return cost; }

    // 根據動態參數 a 更新位置
    Whale update(const Whale& best, const Whale& randWhale, double a) const {
        Whale nxt = *this;
        std::uniform_real_distribution<double> u01(0.0, 1.0);
        double p = u01(rng);
        double r1 = u01(rng), r2 = u01(rng);
        double A = 2.0 * a * r1 - a;

        if (p < 0.5) {
            // 包圍行為
            if (std::abs(A) < 1.0) {
                nxt = encircle(best);
            } else {
                nxt = encircle(randWhale);
            }
        } else {
            // 螺旋或反轉搜尋
            nxt = spiral();
        }
        nxt.evaluate();
        return nxt;
    }

private:
    const Config* cfg_;

    // Encircle: 找到第一處與 target 不同，交換以靠攏
    Whale encircle(const Whale& target) const {
        Whale w(*this);
        int n = ss.size();
        for (int i = 0; i < n; ++i) {
            if (w.ss[i] != target.ss[i]) {
                // 將 w.ss[i] 與 target.ss[i] 對應位置交換
                int j = std::find(w.ss.begin(), w.ss.end(), target.ss[i]) - w.ss.begin();
                std::swap(w.ss[i], w.ss[j]);
                // 隨機機器分配
                w.ms[w.ss[i]] = rng() % cfg_->thePCount;
                w.ms[w.ss[j]] = rng() % cfg_->thePCount;
                break;
            }
        }
        return w;
    }

    // Spiral: 隨機反轉子段一次
    Whale spiral() const {
        Whale w(*this);
        int n = ss.size();
        std::uniform_int_distribution<int> d(0, n - 1);
        int a = d(rng), b = d(rng);
        if (a > b) std::swap(a, b);
        std::reverse(w.ss.begin() + a, w.ss.begin() + b + 1);
        for (int k = a; k <= b; ++k) {
            w.ms[w.ss[k]] = rng() % cfg_->thePCount;
        }
        return w;
    }

    // 評估並更新 cost
    void evaluate() {
        auto res = Solution_Function(*this, *cfg_);
        cost = res.makespan;
    }
};

#endif // WHALE_HPP
