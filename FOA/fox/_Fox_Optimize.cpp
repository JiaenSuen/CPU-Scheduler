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

 

Solution FOX_Algorithm(const Config& cfg, FOX_Parameters& pars, vector<double>* Recorder = nullptr) {
    int T = cfg.theTCount;
    int D = 2 * T;
    std::uniform_real_distribution<double> uni(0.0, 1.0);

    // 1. 初始化 all foxes
    std::vector<Fox_Agent> foxes;
    foxes.reserve(pars.n);
    for (int i = 0; i < pars.n; ++i) {
        foxes.emplace_back(i, T, cfg, pars);
        foxes.back().initialize_position();
    }

    // 2. 找出初始全域最佳
    std::vector<double> globalBestX = foxes[0].get_best_continuous_X();
    double globalBestFit = foxes[0].get_best_fitness();
    Solution globalBestSol = foxes[0];
    for (int i = 1; i < pars.n; ++i) {
        if (foxes[i].get_best_fitness() > globalBestFit) {
            globalBestFit = foxes[i].get_best_fitness();
            globalBestX   = foxes[i].get_best_continuous_X();
            globalBestSol = foxes[i];
        }
    }
    if (Recorder) Recorder->push_back(globalBestSol.cost); // 紀錄第0代

    // 3. 迴圈主流程
    for (int it = 1; it <= pars.MaxIt; ++it) {
        // (1) 以上代 globalBestX 協同更新所有狐狸
        for (int i = 0; i < pars.n; ++i) {
            double r = uni(pars.rng);
            if (r >= 0.5) {
                double Time_ST = uni(pars.rng);
                double Sp_S     = foxes[i].calculate_sp_s();
                double Dist_ST  = foxes[i].calculate_distance_ST(Sp_S, Time_ST);
                double Dist_FP  = foxes[i].calculate_dist_fox_prey(Dist_ST);
                double p_rand   = uni(pars.rng);
                foxes[i].update_position_exploitation(p_rand, Dist_FP);
            } else {
                foxes[i].update_position_exploration(globalBestX, it);
            }
        }

        // (2) 更新本代全域最佳
        for (int i = 0; i < pars.n; ++i) {
            if (foxes[i].get_best_fitness() > globalBestFit) {
                globalBestFit = foxes[i].get_best_fitness();
                globalBestX   = foxes[i].get_best_continuous_X();
                globalBestSol = foxes[i];
            }
        }

        // (3) 紀錄本代最佳 makespan
        if (Recorder) Recorder->push_back(globalBestSol.cost);
    }

    // 4. 回傳最終解
    return globalBestSol;
}









int main( ) {
        
    string config_file = "../../datasets/n4_06.dag";
    string filename = "data.txt";
    // 1. 讀入 Config
    Config cfg;
    cfg = ReadConfigFile(config_file);
        
 

    int T = cfg.theTCount;
    int D = 2 * T;
    // 2. 初始化 FOX_Parameters
    std::vector<double> lb(D, 0.0), ub(D, 1.0);

     
    FOX_Parameters pars(
        50,      // n = 50
        200,     // MaxIt = 100
        0.8,     // alpha
        0.6,     // beta
        0.18,    // c1
        0.82,    // c2
        0.18,    // p_threshold
        1.0,      
        lb,
        ub
    );

    double Avg = 0;
    vector<double> Recorder ;
    int count = 5;
    for (size_t i = 0; i < count; i++)
    {
        Recorder.clear();
        // 3. 執行 FOX Algorithm
        Solution best = FOX_Algorithm(cfg, pars , &Recorder);

        // 4. 輸出最終結果
        std::cout << "\n=== Best Solution ===\n";
        std::cout << "ss: ";
        os_display::show_vector(best.ss);
        std::cout << "ms: ";
        os_display::show_vector(best.ms);
        std::cout << "makespan = " << best.cost << "\n\n";
        
        Avg += best.cost;
    }

    cout<<Avg/count;
    /*writeVectorToFile(Recorder, filename);
    Call_Py_Visual();*/
    return 0;
}