#ifndef WOA_HPP
#define WOA_HPP




#include "include/modules.hpp"
#include "tabu_search.hpp"
#include <algorithm>
#include <random>
#include <numeric>

extern std::mt19937 rng;

typedef std::vector<int> Vec;
class Whale : public Solution {
private:
    const Config* cfg_;

    // --- Discrete operators for ss 排序 ---
    // 1. Encircle (圍捕): SwapTowardBest — bring ss closer to best solution
    static void swapTowardBestSS(Vec &ss, const Vec &best, double A) {
        int n = ss.size();
        int m = std::ceil(std::abs(A) * n / 2.0);
        std::uniform_int_distribution<int> dist(0, n - 1);
        while (m-- > 0) {
            int i = dist(rng), j = dist(rng);
            auto pos_i = std::find(best.begin(), best.end(), ss[i]) - best.begin();
            auto pos_j = std::find(best.begin(), best.end(), ss[j]) - best.begin();
            if ((i < j && pos_i > pos_j) || (i > j && pos_i < pos_j)) {
                std::swap(ss[i], ss[j]);
            }
        }
    }

    // 2. Spiral (螺旋): TwoOptReverse — local reversal (2-Opt)
    static void twoOptReverseSS(Vec &ss) {
        int n = ss.size(); if (n < 2) return;
        std::uniform_int_distribution<int> dist(0, n - 2);
        int i = dist(rng);
        std::uniform_int_distribution<int> dist2(i + 1, n - 1);
        int j = dist2(rng);
        std::reverse(ss.begin() + i, ss.begin() + j + 1);
    }

    // 3. Exploration (搜索): BlockShuffle — cut and insert
    static void blockShuffleSS(Vec &ss) {
        int n = ss.size(); if (n < 2) return;
        std::uniform_int_distribution<int> dist(0, n - 1);
        int i = dist(rng), j = dist(rng);
        if (i > j) std::swap(i, j);
        Vec segment(ss.begin() + i, ss.begin() + j + 1);
        ss.erase(ss.begin() + i, ss.begin() + j + 1);
        std::uniform_int_distribution<int> distPos(0, ss.size());
        ss.insert(ss.begin() + distPos(rng), segment.begin(), segment.end());
    }

    // --- Discrete operators for ms 匹配 ---
    // 1. Encircle (圍捕): GreedyAdopt — adopt best processor with probability |A|
    static void greedyAdoptMS(Vec &ms, const Vec &best, double A, int P) {
        int n = ms.size();
        std::uniform_real_distribution<double> prob(0.0, 1.0);
        std::uniform_int_distribution<int> distP(0, P - 1);
        for (int k = 0; k < n; ++k) {
            if (prob(rng) < std::abs(A)) ms[k] = best[k];
            else ms[k] = distP(rng);
        }
    }

    // 2. Spiral (螺旋): SingleSwap — swap two assignments
    static void singleSwapMS(Vec &ms) {
        int n = ms.size(); if (n < 2) return;
        std::uniform_int_distribution<int> dist(0, n - 1);
        std::swap(ms[dist(rng)], ms[dist(rng)]);
    }

    // 3. Exploration (搜索): RandomReset — reset k assignments
    static void randomResetMS(Vec &ms, int P) {
        int n = ms.size();
        int k = std::ceil(0.2 * n);
        std::uniform_int_distribution<int> distIdx(0, n - 1);
        std::uniform_int_distribution<int> distP(0, P - 1);
        for (int t = 0; t < k; ++t) ms[distIdx(rng)] = distP(rng);
    }

public:
    explicit Whale(const Config& cfg) : cfg_(&cfg) {
        Solution sol = GenerateInitialSolution(cfg);
        ss = std::move(sol.ss);
        ms = std::move(sol.ms);
        ScheduleResult res = Solution_Function(*this, cfg);
        cost = res.makespan;
    }

    Whale update(const Whale &best, const Whale &/*randWhale*/, double a, double p) const {
        Whale offspring(*cfg_);
        offspring.ss = ss;
        offspring.ms = ms;

        std::uniform_real_distribution<double> distR(0.0,1.0);
        double r = distR(rng);
        double A = 2 * a * r - a;

        if (p < 0.5) {
            if (std::abs(A) < 1.0) {
                // Encircling prey (圍捕)
                swapTowardBestSS(offspring.ss, best.ss, A);
                greedyAdoptMS(offspring.ms, best.ms, A, cfg_->thePCount);
            } else {
                // Exploration (搜索)
                blockShuffleSS(offspring.ss);
                randomResetMS(offspring.ms, cfg_->thePCount);
            }
        } else {
            // Spiral updating position (螺旋)
            twoOptReverseSS(offspring.ss);
            singleSwapMS(offspring.ms);
        }

        std::uniform_real_distribution<double> leap_dist(0.0, 1.0);
        if (leap_dist(rng) < 0.08) {
            Tabu_Search( *cfg_ ,  &offspring);
        }
        
        ScheduleResult res = Solution_Function(offspring, *cfg_);
        offspring.cost = res.makespan;
        return offspring;
    }
};


Solution Whale_Optimize(const Config& cfg,
                        int num_whales = 20,
                        int max_iter   = 200) 
{
    // 1. 初始化種群
    std::vector<Whale> pop;
    pop.reserve(num_whales);
    for (int i = 0; i < num_whales; ++i) {
        pop.emplace_back(cfg);
    }

    // 2. 找到初始最優
    Whale best = pop[0];
    for (auto& w : pop) {
        if (w.cost < best.cost) best = w;
    }

    // 3. 迭代演化
    for (int iter = 1; iter <= max_iter; ++iter) {
        // 收斂因子 a 隨迭代線性下降從 2 → 0
        double a = 2.0 * (1.0 - double(iter) / max_iter);

        for (int i = 0; i < num_whales; ++i) {
            // 選一隻鯨魚 ( i)
            int rand_idx;
            do { rand_idx = rng() % num_whales; }
            while (rand_idx == i);

            Whale& cur = pop[i];
            Whale& randWhale = pop[rand_idx];

            // 隨機機率 p ∈ [0,1]
            double p = std::generate_canonical<double, 10>(rng);

            // 更新
            Whale offspring = cur.update(best, randWhale, a, p);
            // 若後代更優，替換當前
            if (offspring.cost < cur.cost) {
                pop[i] = std::move(offspring);
            }
        }

        // 更新全局最優
        for (auto& w : pop) {
            if (w.cost < best.cost) best = w;
        }

        
        // std::cout << "Iter " << iter << ", best makespan = " << best.cost << "\n";
    }

   
    return static_cast<Solution>(best);
}








#endif // WHALE_HPP
