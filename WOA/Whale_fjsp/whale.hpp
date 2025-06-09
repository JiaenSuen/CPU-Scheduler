#ifndef WHALE_HPP
#define WHALE_HPP

#include "include/modules.hpp"
#include <algorithm>
#include <random>
#include <numeric>

extern std::mt19937 rng;

typedef std::vector<int> Vec;

class Whale : public Solution {
private:
    const Config* cfg_;

    // --- Crossover / mutation operators ---
    // Improved Precedence Preserving Order-based Crossover (IPOX)
    static Vec IPOX(const Vec &parentA, const Vec &parentB) {
        int n = parentA.size();
        Vec child(n, -1);
        std::uniform_int_distribution<int> dist(1, n - 1);
        int cut = dist(rng);
        // 保留 parentA 前 cut 個元素
        for (int i = 0; i < cut; ++i) child[i] = parentA[i];    
        // 按 parentB 順序填入其餘元素
        int idx = cut;
        for (int x : parentB) {
            if (std::find(child.begin(), child.begin() + cut, x) == child.begin() + cut) {
                child[idx++] = x;
            }
        }
        return child;
    }

    // Multi-Point Crossover for machine assignment (MPX)
    static Vec MPX(const Vec &parentA, const Vec &parentB) {
        int n = parentA.size();
        Vec child(n);
        std::uniform_int_distribution<int> bit(0, 1);
        for (int i = 0; i < n; ++i) {
            child[i] = bit(rng) ? parentA[i] : parentB[i];
        }
        return child;
    }

    // Random Order-based Crossover (ROX)
    static Vec ROX(const Vec &parentA, const Vec &parentB) {
        int n = parentA.size();
        Vec child;
        std::uniform_int_distribution<int> dist(1, n - 1);
        int k = dist(rng);
        // 隨機選 k 個任務到前端
        Vec pool = parentA;
        std::shuffle(pool.begin(), pool.end(), rng);
        child.insert(child.end(), pool.begin(), pool.begin() + k);
        // 按原序加入剩餘任務
        for (int x : parentA) {
            if (std::find(child.begin(), child.end(), x) == child.end())
                child.push_back(x);
        }
        return child;
    }

    // Two-Point Crossover for machine assignment (TPX)
    static Vec TPX(const Vec &parentA, const Vec &parentB) {
        int n = parentA.size();
        Vec child = parentA;
        std::uniform_int_distribution<int> dist(1, n - 1);
        int i = dist(rng), j = dist(rng);
        if (i > j) std::swap(i, j);
        for (int k = i; k < j; ++k) {
            child[k] = parentB[k];
        }
        return child;
    }

public:
    // 建構子：初始化並計算成本
    explicit Whale(const Config& cfg )
        : cfg_(&cfg)
    {
        Solution sol = GenerateInitialSolution( cfg );
        ss = std::move(sol.ss);
        ms = std::move(sol.ms);
        // 計算初始解成本
        ScheduleResult res = Solution_Function(*this, cfg);
        //cost = res.makespan;
    }

    // 更新函數：根據 a, p, best, randWhale 三種行為產生新 Whale
    Whale update(const Whale &best, const Whale &randWhale, double a, double p) const {
        Whale offspring(*cfg_);
        // 選擇行為
        if (p < 0.5) {
            if (std::abs(a) < 1.0) {
                // Encircling Prey
                offspring.ss = IPOX(best.ss, ss);
                offspring.ms = MPX(best.ms, ms);
            } else {
                // Search for Prey
                offspring.ss = IPOX(randWhale.ss, ss);
                offspring.ms = MPX(randWhale.ms, ms);
            }
        } else {
            // Spiral Updating
            offspring.ss = ROX(best.ss, ss);
            offspring.ms = TPX(best.ms, ms);
        }
        // 評估 offspring 成本
        ScheduleResult res = Solution_Function(offspring, *cfg_);
        offspring.cost = res.makespan;
        return offspring;
    }
};

#endif // WHALE_HPP