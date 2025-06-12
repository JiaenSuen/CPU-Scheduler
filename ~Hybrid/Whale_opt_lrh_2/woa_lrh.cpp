#include "include/modules.hpp"
#include "WOA.hpp"
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
    

    int Num_of_whale = 5;


    double Avg_Cost = 0;
    double num_loop = 10;
    
    for(int i =0;i<num_loop;i++){
        auto start = std::chrono::high_resolution_clock::now();
        Solution best = Whale_Optimize(cfg , Num_of_whale);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        cout << "Time Usage : " << duration.count() << " ms" << std::endl;

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