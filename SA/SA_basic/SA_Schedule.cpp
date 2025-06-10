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

 

// MetaHerustic Interface
Solution GenerateNeighbor(const Solution& current, const Config& config) ;

 



// Simulated Annealing
Solution Simulated_Annealing( Config& config  , vector<double>* GB_Recorder = nullptr , vector<double>* CB_Recorder = nullptr){
    
    
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

            //cout<<"Iter ["<<Iter<<"] : "<<currentCost<<std::endl; 

            Iter++;
            if (Iter >= params.max_Iter || params.noImproveCount >= params.max_NoImprove) break;
        }

        GB_Recorder->push_back(Best_Solution.cost);
        CB_Recorder->push_back(current_S.cost);

        params.T *= params.alpha;
        if (params.noImproveCount >= params.max_NoImprove) break;
    }

    return Best_Solution;
}





int main()
{   
    vector<double> Global_Best_Recorder , Current_Best_Recorder;
    Config config = ReadConfigFile("../../datasets/n4_06.dag");

    
    double Avg_Cost = 0;
    int count = 100;
    for (size_t i = 0; i < count; i++)
    {
        Solution sol = Simulated_Annealing(config , &Global_Best_Recorder , &Current_Best_Recorder);

        cout<<"Best Solution : \n";
        show_solution(sol);
        ScheduleResult SR =  Solution_Function(sol,config,true);
        cout<<boolalpha<<is_feasible(SR,config)<<endl;
        cout<<"Best Cost : \n";
        cout<<SR.makespan<<endl;
        cout<<is_feasible(SR,config);
        cout<<"\n\n";

        Avg_Cost+=SR.makespan;
    }
    
    cout<<"\n\n\n"<<Avg_Cost/count;
    

    /*
    writeTwoVectorsToFile(Global_Best_Recorder, Current_Best_Recorder, "data.txt");
    Call_Py_Visual();*/
    return 0;
}

 
 

 

/*
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
