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




/*
for SA , tabu 
| 0    | Swap in `ss`               | 交換兩個任務的順序
| 1    | Change in `ms`             | 隨機改變某個任務的處理器分配 
| 2    | Swap in `ss` + modify `ms` | 同時調整順序與處理器配置（加強探索） 
*/
Solution GenerateNeighbor(const Solution& current, const Config& config) {
    Solution neighbor = current;
    int T = config.theTCount;
    int P = config.thePCount;

    // Randomly Choose A Method Operator to Get Neighbor
    int move_type = rng() % 3;  // 0: swap ss, 1: change ms, 2: both

    if (move_type == 0 || move_type == 2) {
        int i = rng() % T;
        int j = rng() % T;
        while (j == i) j = rng() % T;
        std::swap(neighbor.ss[i], neighbor.ss[j]);
    }

    if (move_type == 1 || move_type == 2) {
         
        int t = rng() % T;
        int newP = rng() % P;
        while (newP == neighbor.ms[t] && P > 1) {
            newP = rng() % P;
        }
        neighbor.ms[t] = newP;
    }

    return neighbor;
}
