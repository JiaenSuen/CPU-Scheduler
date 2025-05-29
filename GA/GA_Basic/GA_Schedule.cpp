#include "include/modules.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <random>

using namespace std;
using namespace os_display;
using namespace std::chrono;

// ----- GA Parameters ------
struct GA_Params {
    int map_size;
    int population_size;
    int generations;
    double crossover_rate;
    double mutation_rate;
    std::string selection_method;
};

GA_Params Init_GA_Params() {
    GA_Params params;
    params.map_size = 20;
    params.population_size = 1000;
    params.generations = 1000;
    params.crossover_rate = 0.7;
    params.mutation_rate = 0.4;
    params.selection_method = "roulette";
    return params;
}
//-------------------------------


class Individual : public Solution {
    public:



};



/*
Individual Run_Genetic_Algorithm(
    const std::vector<g_Node>& city_map,
    const GA_Params& params
) {
    // 1. 初始化族群
    std::vector<Individual> population;
    population.reserve(params.population_size);
    for (int i = 0; i < params.population_size; i++)
        population.emplace_back(city_map);

    // 2. 依 cost 排序（方便取精英）
    std::sort(population.begin(), population.end(), cmp_individual);

    // 3. 演化主迴圈
    for (int gen = 1; gen <= params.generations; gen++) {
        std::vector<Individual> new_pop;
        // 精英保留
        const int ELITE = 3;
        for (int i = 0; i < ELITE; i++)
            new_pop.push_back(population[i]);

        // 產生剩餘新族群
        while ((int)new_pop.size() < params.population_size) {
            // 選擇父母
            Individual parent1 = 
                (params.selection_method == "roulette") 
                ? roulette_select(population)
                : tournament_select(population, 5);

            Individual parent2 = 
                (params.selection_method == "roulette") 
                ? roulette_select(population)
                : tournament_select(population, 5);
            
            // 交配或直接複製
            std::uniform_real_distribution<double> prob(0.0,1.0);
            Individual child = (prob(RNG::generator) < params.crossover_rate)
                               ? parent1.crossover(parent2)
                               : parent1;
            // 突變
            child.mutate(params.mutation_rate);
            new_pop.push_back(child);
        }

        // 替換族群並排序
        population = std::move(new_pop);
        std::sort(population.begin(), population.end(), cmp_individual);

        // 每隔一段印出進度
        if (gen % 50 == 0) {
            std::cout << "Gen " <<std::setw(4)<< gen
                      << " Best Cost = " << population.front().cost
                      << "\n";
        }
    }

    // 回傳最優解
    return population.front();
}
*/

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

