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


// Avg Cost = 488.900000 , 20 , 100
// Avg Cost =  444.700000 , 20 , 200
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


// 444.200000
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