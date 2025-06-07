#ifndef MODULES_HPP
#define MODULES_HPP


#include "config.hpp"
#include "evaluation.hpp"
#include "utils.hpp"


#include <numeric>
#include <random>
using namespace std;
std::mt19937 rng(std::random_device{}());






Solution GenerateInitialSolution(const Config& cfg, bool useHeuristic=false){
    int T = cfg.theTCount;
    int P = cfg.thePCount;
    Solution sol;


    // Process Initial Schedule String
    sol.ss.resize(T);
    std::iota(sol.ss.begin(), sol.ss.end(), 0);
    if (!useHeuristic) {
        std::shuffle(sol.ss.begin(), sol.ss.end(), rng); 
    }else{
        // implement Herustic Solution
    }


    // Process Initial Matching String
    sol.ms.resize(T);
    for (int t = 0; t < T; ++t) {
        if (!useHeuristic) {
            sol.ms[t] = rng() % P;
        } else{
            break; // implement Herustic Solution
        }
    }

    return sol;
}


#endif