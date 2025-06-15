#include "include/modules.hpp"
#include "tabu_search.hpp"

#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <random>

using namespace std;
using namespace os_display;
using namespace std::chrono;




  
int main() {
    Config cfg = ReadConfigFile("../../datasets/n4_00.dag");
    
    
    double num_loop = 1;

    // ----- TS Parameters ------

    int maxIter       = 200;   // 最大迭代次數  
    int tabuTenure    = 10;    // 禁忌期限  
    int numCandidates = 60;   // 一次產生的鄰居數量  

    double Avg_Cost = 0;
    double best_cost = 100000;
    double worst_cost = 0;

    vector<double> GB,CB;
    for(int i =0;i<num_loop;i++){
        Solution best = GenerateInitialSolution(cfg,true);
        
       
        ScheduleResult sr = Solution_Function(best, cfg , true);
        cout << "Best makespan: " << best.cost << "\n";
        show_solution(best);
        cout << "Feasible: " << std::boolalpha << is_feasible(sr, cfg) << "\n";
        cout << "Cost : " << sr.makespan;
        Avg_Cost+=best.cost;
        cout<<"\n";

        if (best_cost > best.cost) best_cost = best.cost;
        if (worst_cost <  best.cost) worst_cost = best.cost;
    }
    printf("\n\n\nAvg Cost = %lf\n",Avg_Cost/num_loop);
    printf("Best Cost = %lf\n",best_cost);
    printf("Worst Cost = %lf\n",worst_cost);
     
    writeTwoVectorsToFile(GB,CB,"data.txt");
    Call_Py_Visual();
    return 0;

}