#include "include/modules.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <random>
#include <functional>


#include "Fox_Agent.hpp"
#include "FOX_Parameters.hpp"


using namespace std;
using namespace os_display;
using namespace std::chrono;

 

Solution FOX_Algorithm(const Config& cfg, FOX_Parameters& pars) {
    int T = cfg.theTCount;
    int D = 2 * T;
    std::uniform_real_distribution<double> uni(0.0, 1.0);

    // 1. 初始化 n 隻 Fox_Agent
    std::vector<Fox_Agent> foxes;
    foxes.reserve(pars.n);
    for (int i = 0; i < pars.n; ++i) {
        foxes.emplace_back(i, T, cfg, pars);
        foxes.back().initialize_position();
    }

    // 2. 迴圈主流程
    for (int it = 1; it <= pars.MaxIt; ++it) {
        // 每次都持有當前全域最優的連續向量
        std::vector<double> globalBestX = foxes[0].get_best_continuous_X();
        double globalBestFit = foxes[0].get_best_fitness();
        Solution globalBestSol = foxes[0]; // 紀錄對應的離散解

        // 找出迴圈開始時的全域最優
        for (int i = 1; i < pars.n; ++i) {
            if (foxes[i].get_best_fitness() > globalBestFit) {
                globalBestFit = foxes[i].get_best_fitness();
                globalBestX = foxes[i].get_best_continuous_X();
                globalBestSol = foxes[i];
            }
        }

        // 對每隻狐狸做利用或探索更新
        for (int i = 0; i < pars.n; ++i) {
            double r = uni(pars.rng);
            if (r >= 0.5) {
                // 利用階段
                double Time_ST = uni(pars.rng);
                double Sp_S = foxes[i].calculate_sp_s();
                double Dist_ST = foxes[i].calculate_distance_ST(Sp_S, Time_ST);
                double Dist_FP = foxes[i].calculate_dist_fox_prey(Dist_ST);
                double p = uni(pars.rng);
                foxes[i].update_position_exploitation(p, Dist_FP);
            } else {
                // 探索階段
                foxes[i].update_position_exploration(globalBestX, it);
            }
        }
    }

    // 3. 結束後，再次選出最優解
    Solution bestSol = foxes[0];
    double bestFit = foxes[0].get_best_fitness();
    for (int i = 1; i < pars.n; ++i) {
        if (foxes[i].get_best_fitness() > bestFit) {
            bestFit = foxes[i].get_best_fitness();
            bestSol = foxes[i];
        }
    }
    return bestSol;
}


int main( ) {
        
    std::string config_file = "../../datasets/n4_00.dag";

    // 1. 讀入 Config
    Config cfg;
    cfg = ReadConfigFile(config_file);
        
 

    int T = cfg.theTCount;
    int D = 2 * T;
    // 2. 初始化 FOX_Parameters
    std::vector<double> lb(D, 0.0), ub(D, 1.0);
    FOX_Parameters pars(
        200,      
        200,    // MaxIt = 100
        0.6,    // alpha
        0.4,    // beta
        0.18,   // c1
        0.82,   // c2
        0.18,   // p_threshold
        std::numeric_limits<double>::infinity(),
        lb, ub
    );
    int count = 5;
    for (size_t i = 0; i < count; i++)
    {
         
        // 3. 執行 FOX Algorithm
        Solution best = FOX_Algorithm(cfg, pars);

        // 4. 輸出最終結果
        std::cout << "\n=== Best Solution ===\n";
        std::cout << "ss: ";
        os_display::show_vector(best.ss);
        std::cout << "ms: ";
        os_display::show_vector(best.ms);
        std::cout << "makespan = " << best.cost << "\n\n";
        
    }
    return 0;
}