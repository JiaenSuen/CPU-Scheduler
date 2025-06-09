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





// Avg_Cost : 472.2
int main(){
    
    Config config = ReadConfigFile("../../datasets/n4_00.dag");
    double count = 100;
    double Avg_Cost = 0;

    for (size_t i = 0; i < count; i++)
    {
        GA_Params params_ga;
        params_ga.population_size = 20;
        params_ga.generations = 200;
        Solution GA_Result = Genetic_Algorithm(config,params_ga);
        cout << "Best makespan: " << GA_Result.cost << "\n";
        show_solution(GA_Result);
        ScheduleResult sr = Solution_Function(GA_Result, config, true);
        cout << "Feasible: " << std::boolalpha << is_feasible(sr, config) << "\n";

        Avg_Cost+= GA_Result.cost;
    }
    cout<<"\n\n\nAvg_Cost : "<<Avg_Cost/count;

    return 0;
}