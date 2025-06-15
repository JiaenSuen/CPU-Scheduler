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
    

    int Num_of_whale = 20;


    double Avg_Cost = 0;
    double Avg_Time = 0;
    double num_loop = 1;

    double best_cost = 100000;
    double worst_cost = 0;

    vector<double> cost_list;

    vector<double> GB_Recorder,PB_Recorder;
    
    for(int i =0;i<num_loop;i++){
        auto start = std::chrono::high_resolution_clock::now();
        Solution best = Whale_Optimize(cfg , Num_of_whale,200 , &GB_Recorder,&PB_Recorder);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        cout << "Time Usage : " << duration.count() << " ms" << std::endl;

        cout << "Best makespan: " << best.cost << "\n";
        ScheduleResult sr = Solution_Function(best, cfg , true);

        show_solution(best);
        cout << "Feasible: " << std::boolalpha << is_feasible(sr, cfg) << "\n";
        cout << "Cost : " << sr.makespan;
        Avg_Cost+=best.cost;
        Avg_Time+=duration.count();
        if (best_cost > best.cost) best_cost = best.cost;
        if (worst_cost <  best.cost) worst_cost = best.cost;
        
        cout<<"\n\n";

        cost_list.push_back(best.cost);
        
    }



    double mean = Avg_Cost / num_loop;
    double variance = 0;
    for (double cost : cost_list) {
        variance += (cost - mean) * (cost - mean);
    }
    variance /= num_loop;
    double std_dev = sqrt(variance);



    printf("\n\n\nAvg Cost = %lf\n",Avg_Cost/num_loop);
    printf("Best Cost = %lf\n",best_cost);
    printf("Worst Cost = %lf\n",worst_cost);
    printf("\n\n\nAvg Time = %lf\n",Avg_Time/num_loop);
    printf("Standard Deviation of Cost = %lf\n", std_dev);

    writeTwoVectorsToFile(GB_Recorder,PB_Recorder , "data.txt");
    Call_Py_Visual(); 
    return 0;

}