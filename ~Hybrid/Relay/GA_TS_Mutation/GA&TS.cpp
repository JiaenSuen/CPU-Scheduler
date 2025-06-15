#include "include/modules.hpp"
#include "GA.hpp"
 

#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <random>

using namespace std;
using namespace os_display;
using namespace std::chrono;



 
  
int main() {
    Config cfg = ReadConfigFile("../../../datasets/n4_06.dag");
    

 

    double Avg_Makespan =0;
    double times = 5;


    GA_Params params_ga;
    params_ga.population_size = 20;
    params_ga.generations = 200;

    vector<double> GB_Recorder;
    vector<double> CB_Recorder;

    for (int i ;i<times;i++){
        
        
        Solution GA_Result = Genetic_Algorithm(cfg , params_ga ,  &GB_Recorder);
        Solution& best =  GA_Result;

        cout << "Best makespan: " << best.cost << "\n";
        ScheduleResult sr = Solution_Function(best, cfg , true);
        show_solution(best);
        cout << "Feasible: " << std::boolalpha << is_feasible(sr, cfg) << "\n";
        cout << "Cost : " << sr.makespan;
        cout<<"\n\n";
        Avg_Makespan+=best.cost;
    }
    Avg_Makespan /= times;
    //system("cls");
    cout<<"\n\n";
    cout<<"Avg Makespan : "<<Avg_Makespan;

    /*
    writeTwoVectorsToFile(GB_Recorder,CB_Recorder,"data.txt");
    Call_Py_Visual();*/
    return 0;
}