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


 

// Avg_Cost : 441.4
int main(){
    
    Config config = ReadConfigFile("../../datasets/n4_00.dag");
    double count = 1;
    double Avg_Cost = 0;

    vector<double> GB_Recorder , LB_Recorder;

    GA_Params params_ga;
    params_ga.population_size = 20;
    params_ga.generations = 200;
    params_ga.selection_method = "r";

    for (size_t i = 0; i < count; i++)
    {
        Solution GA_Result = Genetic_Algorithm_2(config,params_ga, &GB_Recorder , &LB_Recorder);
        cout << "Best makespan: " << GA_Result.cost << "\n";
        show_solution(GA_Result);
        ScheduleResult sr = Solution_Function(GA_Result, config, true);
        cout << "Feasible: " << std::boolalpha << is_feasible(sr, config) << "\n";

        Avg_Cost+= GA_Result.cost;
    }
    cout<<"\n\n\nAvg_Cost : "<<Avg_Cost/count<<endl;

    writeTwoVectorsToFile(GB_Recorder , LB_Recorder, "data.txt");
    Call_Py_Visual();

    return 0;
}