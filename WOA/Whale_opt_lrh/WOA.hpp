#ifndef WOA_HPP
#define WOA_HPP




#include "include/modules.hpp"
#include <algorithm>
#include <random>
#include <numeric>

extern std::mt19937 rng;

typedef std::vector<int> Vec;

class Whale : public Solution {
private:
    const Config* cfg_;

    // --- Simple discrete operators ---
    // Swap two positions in schedule sequence ss
    static void swapSS(Vec &seq) {
        int n = seq.size();
        std::uniform_int_distribution<int> dist(0, n-1);
        int i = dist(rng), j = dist(rng);
        std::swap(seq[i], seq[j]);
    }

    // Random reassign one machine in ms
    static void mutateMS(Vec &ms, int P) {
        int n = ms.size();
        std::uniform_int_distribution<int> distIdx(0, n-1);
        std::uniform_int_distribution<int> distP(0, P-1);
        int idx = distIdx(rng);
        ms[idx] = distP(rng);
    }

    // Crossover: take prefix from parentA then fill rest by order from parentB
    static Vec prefixCrossover(const Vec &parentA, const Vec &parentB) {
        int n = parentA.size();
        Vec child(n, -1);
        std::uniform_int_distribution<int> dist(1, n-1);
        int cut = dist(rng);
        // prefix
        for (int i = 0; i < cut; ++i) child[i] = parentA[i];
        // fill remaining
        int idx = cut;
        for (int x : parentB) {
            if (std::find(child.begin(), child.begin()+cut, x) == child.begin()+cut) {
                child[idx++] = x;
            }
        }
        return child;
    }

public:
    explicit Whale(const Config& cfg) : cfg_(&cfg)
    {
        Solution sol = GenerateInitialSolution(cfg);
        ss = std::move(sol.ss);
        ms = std::move(sol.ms);
        ScheduleResult res = Solution_Function(*this, cfg);
    }

    Whale update(const Whale &best, const Whale &randWhale, double a, double p) const {
        Whale offspring(*cfg_);
        // Exploration vs Exploitation
        if (p < 0.5) {
            // Exploration: small random mutations
            offspring.ss = ss;
            offspring.ms = ms;
            swapSS(offspring.ss);
            mutateMS(offspring.ms, cfg_->thePCount);
        } else {
            // Exploitation: combine with best
            offspring.ss = prefixCrossover(best.ss, ss);
            offspring.ms = ms;
            if (std::abs(a) < 1.0) {
                // Encircle best: one mutation on ms
                mutateMS(offspring.ms, cfg_->thePCount);
            } else {
                // Spiral: two mutations on ms for finer adjustment
                mutateMS(offspring.ms, cfg_->thePCount);
                mutateMS(offspring.ms, cfg_->thePCount);
            }
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
