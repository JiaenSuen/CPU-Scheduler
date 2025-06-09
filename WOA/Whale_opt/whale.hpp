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

#endif // WHALE_HPP
