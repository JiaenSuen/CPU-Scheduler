#include "include/modules.hpp"
#include "whale.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <random>

using namespace std;
using namespace os_display;
using namespace std::chrono;
 

// Avg Cost = 471.910000
Solution Whale_Optimize(const Config& cfg,
                        int num_whales = 20,
                        int max_iter   = 200) {
    // 1. 準備族群
    std::vector<Whale> pod; pod.reserve(num_whales);
    for (int i = 0; i < num_whales; ++i)
        pod.emplace_back(cfg);

    // 2. 初始全局最優
    Whale best = *std::min_element(pod.begin(), pod.end(),
                                   [](auto& a, auto& b){ return a.getCost()<b.getCost(); });

    // 3. 迭代更新
    for (int t = 1; t <= max_iter; ++t) {
        double a = 2.0 * (1.0 - double(t)/max_iter);

        for (int i = 0; i < num_whales; ++i) {
            int j;
            do { j = rng() % num_whales; } while (j == i);

            Whale cand = pod[i].update(best, pod[j], a);


            if (cand.getCost() < pod[i].getCost())
                pod[i] = std::move(cand);

            if (pod[i].getCost() < best.getCost())
                best = pod[i];
        }
        //std::cout << "Iter " << t << ": best = " << best.getCost() << "\n";
    }

    return static_cast<Solution>(best);
}
  


int main() {
    Config cfg = ReadConfigFile("../../datasets/n4_00.dag");
    
    double Avg_Cost = 0;
    double num_loop = 10;
 
    for(int i =0;i<num_loop;i++){
        Solution best = Whale_Optimize(cfg);

        cout << "Best makespan: " << best.cost << "\n";
        ScheduleResult sr = Solution_Function(best, cfg , true);
        show_solution(best);
        cout << "Feasible: " << std::boolalpha << is_feasible(sr, cfg) << "\n";
        cout << "Cost : " << sr.makespan;
        Avg_Cost+=best.cost;
        cout<<"\n\n";
    }
    printf("\n\n\nAvg Cost = %lf\n\n",Avg_Cost/num_loop);
    return 0;

}