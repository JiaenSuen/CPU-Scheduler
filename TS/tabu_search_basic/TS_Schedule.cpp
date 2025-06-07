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
    
    double Avg_Cost = 0;
    double num_loop = 5;

    // ----- TS Parameters ------

    int maxIter       = 200;   // 最大迭代次數  
    int tabuTenure    = 10;    // 禁忌期限  
    int numCandidates = 60;   // 一次產生的鄰居數量  

    for(int i =0;i<num_loop;i++){
        Solution best = Tabu_Search(cfg, nullptr  ,maxIter, tabuTenure, numCandidates);

        cout << "Best makespan: " << best.cost << "\n";
        ScheduleResult sr = Solution_Function(best, cfg , true);
        show_solution(best);
        cout << "Feasible: " << std::boolalpha << is_feasible(sr, cfg) << "\n";
        cout << "Cost : " << sr.makespan;
        Avg_Cost+=best.cost;
        cout<<"\n";
    }
    printf("\n\n\nAvg Cost = %lf\n\n",Avg_Cost/num_loop);
    return 0;

}