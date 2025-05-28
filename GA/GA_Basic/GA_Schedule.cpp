#include "include/modules.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <random>

using namespace std;
using namespace os_display;
using namespace std::chrono;


 

struct SA_Params {
    double T;                   // 初始溫度
    double T_min;              // 最低溫度
    double alpha;             // 冷卻係數 (0.9–0.99)
    int iterPerTemp;         // 每個溫度迴圈次數
    int max_Iter;           // 最大總迭代次數
    int max_NoImprove;     // 連續無改善上限
    bool use_Heuristic;   // 是否啟用啟發式初解
    int noImproveCount;  // 內部計數，初始化為 0
};


struct GA_Params {

};


SA_Params& set_SA_param() {
    static SA_Params params;
    params.T              = 100.0;             // 例如：根據問題規模可自己調整
    params.T_min          = 1e-3;             // 例如：當溫度低於此值時停止 
    params.alpha          = 0.85;            // 每次迴圈後溫度乘上 alpha
    params.iterPerTemp    = 5;              // 每個溫度底下做多少次鄰域搜尋
    params.max_Iter       = 200;           // 最多總迭代次數
    params.max_NoImprove  = 6000;         // 連續多少次沒有改善就停止
    params.use_Heuristic  = false;       // 預設用隨機初始解
    params.noImproveCount = 0;          // 計數器歸零
    return params;
}

 
Solution Genetic_Algorithm () {
    
}




// Simulated Annealing
Solution Simulated_Annealing(const Config& config){
    
    
    SA_Params& params = set_SA_param();

    // initialize 
    Solution current_S = GenerateInitialSolution(config, params.use_Heuristic);
    ScheduleResult result = Solution_Function(current_S,config);
    double currentCost = result.makespan;

    // Best Init
    Solution Best_Solution = current_S;
    double best_Cost = currentCost;

    int Iter = 0;
    // SA Main Loop
    while(params.T > params.T_min &&  Iter < params.max_Iter){
        for (int i = 0; i < params.iterPerTemp; ++i) {
            Solution  Neighbor_Solution = GenerateNeighbor(current_S, config);
            ScheduleResult nei_result = Solution_Function(Neighbor_Solution, config);
            double newCost = nei_result.makespan;


            double delta = newCost - currentCost;
            // Make decision for Accept Neighbor Solution or Not
            bool accept = false;
            if (delta <= 0) {
                accept = true;
            } else {
                double prob = std::exp(-delta / params.T);
                if (((double)rand() / RAND_MAX) < prob) {
                    accept = true;
                }
            }

            if (accept) {
                current_S = Neighbor_Solution;
                currentCost = newCost;
                if (newCost < best_Cost) {
                    Best_Solution = Neighbor_Solution;
                    best_Cost = newCost;
                    params.noImproveCount = 0; 
                } else {
                    params.noImproveCount++;
                }
            }


            cout<<"Iter ["<<Iter<<"] : "<<currentCost<<std::endl; 




            Iter++;
            if (Iter >= params.max_Iter || params.noImproveCount >= params.max_NoImprove) break;
        }

        params.T *= params.alpha;
        if (params.noImproveCount >= params.max_NoImprove) break;
    }

    return Best_Solution;
}





int main()
{
    Config config = ReadConfigFile("n4_00.dag");
    Solution sol = Simulated_Annealing(config);
    cout<<"Best Solution : \n";
    show_solution(sol);
    ScheduleResult SR =  Solution_Function(sol,config,true);
    cout<<boolalpha<<is_feasible(SR,config)<<endl;
    cout<<"Best Cost : \n";
    cout<<SR.makespan<<endl;
    cout<<is_feasible(SR,config);

    return 0;
}

