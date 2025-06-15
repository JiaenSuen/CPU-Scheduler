#ifndef IDVIDUAL_HPP
#define IDEVIUAL_HPP
#include "include/modules.hpp"



// ----- GA Parameters ------
struct GA_Params {
    int population_size;
    int generations;
    double crossover_rate;
    double mutation_rate;
    std::string selection_method;

    GA_Params(){
        population_size = 50;
        generations = 200;
        crossover_rate = 0.7;
        mutation_rate = 0.4;
        selection_method = "t"; //  鍛造式選擇 t 、 輪盤式 r
    }
};





// Individual 
class Individual : public Solution {
public:
    double fitness;  

    Individual() = default;
    Individual(const Config& cfg) {
        Solution init = GenerateInitialSolution(cfg);
        this->ss = std::move(init.ss);
        this->ms = std::move(init.ms);
        evaluate(cfg);
    }

    //  Mating
    Individual crossover( Individual& other, Config& cfg , double crossover_rate) const {
        int T = cfg.theTCount;
        int P = cfg.thePCount;
        std::uniform_real_distribution<double> uni_rnd(0.0, 1.0);
        std::uniform_int_distribution<int> cutDist(0, T-1);

        Individual child = *this;  

        //  OX for ss
        if (uni_rnd(rng) < crossover_rate) {
            int c1 = cutDist(rng), c2 = cutDist(rng);
            if (c1 > c2) std::swap(c1, c2);

            // Child Generate Schedule String 
            std::vector<int> new_ss(T, -1);
            std::vector<bool> used(T, false);

             
            for (int i = c1; i <= c2; ++i) {
                new_ss[i] = ss[i];
                used[ss[i]] = true;
            }
            //  other.ss  Fill empty Gene
            int idx = (c2 + 1) % T;
            for (int k = 0; k < T; ++k) {
                int gene = other.ss[(c2 + 1 + k) % T];
                if (!used[gene]) {
                    new_ss[idx]    = gene;
                    used[gene]     = true;
                    idx            = (idx + 1) % T;
                }
            }
            child.ss = std::move(new_ss);
        }
     

        // Uniform crossover for ms
        for (int i = 0; i < T; ++i) {
            if (uni_rnd(rng) < 0.5)
                child.ms[i] = this->ms[i];
            else
                child.ms[i] = other.ms[i];
        }

        // Return Child
        child.evaluate(cfg);
        return child;
    }

    // Mutation
    void mutate(const Config& cfg, double mutation_rate) {
        int T = cfg.theTCount;
        int P = cfg.thePCount;
        std::uniform_real_distribution<double> uni_rnd(0.0, 1.0);
        std::uniform_int_distribution<int> swap_selector(0, T-1);
        std::uniform_int_distribution<int> mutation_point(0, P-1);

        bool changed = false;
        //  ss 交換突變
        if (uni_rnd(rng) < mutation_rate) {
            int i = swap_selector(rng), j = swap_selector(rng);
            std::swap(ss[i], ss[j]);
            changed = true;
        }
        //  ms 隨機重設
        if (uni_rnd(rng) < mutation_rate) {
            int k = swap_selector(rng);
            ms[k] = mutation_point(rng);
            changed = true;
        }
        if (changed) evaluate(cfg);
    }

     
    void evaluate(const Config& cfg, bool show_adjust=false) {
        ScheduleResult res = Solution_Function(*this, cfg, show_adjust);
        this->cost    = res.makespan;
        this->fitness = 1.0 / (this->cost + 1e-9);  
    }
};
// End To Define Individual 





namespace Selection_For_GA {
    // Selection_Tournament :
    // 競賽選擇（Tournament Selection）
    int Tournament_Select(const std::vector<Individual>& pop, int tournament_size) {
        std::uniform_int_distribution<int> dist(0, pop.size() - 1);
        int best_idx = dist(rng);
        double best_fit = pop[best_idx].fitness;

        for (int i = 1; i < tournament_size; ++i) {
            int idx = dist(rng);
            if (pop[idx].fitness > best_fit) {
                best_fit = pop[idx].fitness;
                best_idx = idx;
            }
        }
        return best_idx;
    }

    // 建立下一代父母配對的選擇池
    void Selection_Tournament(const std::vector<Individual>& old_pop, vector<Individual>& mating_pool, const GA_Params& params) {
        int pop_size = old_pop.size();
        int tour_size = 3;   
        mating_pool.clear();
        mating_pool.reserve(pop_size);

        for (int i = 0; i < pop_size; ++i) {
            int sel_idx = Tournament_Select(old_pop, tour_size);
            mating_pool.push_back(old_pop[sel_idx]);
        }
    }

    


    int Roulette_Select(const std::vector<Individual>& pop) {
        int n = pop.size();
        // 計算總適應度
        double sum_fitness = 0.0;
        for (const auto& ind : pop) {
            sum_fitness += ind.fitness;
        }
        if (sum_fitness <= 0) {
            // 若所有 fitness 非正，退回隨機選擇
            std::uniform_int_distribution<int> dist(0, n - 1);
            return dist(rng);
        }
        // 建立累積和
        std::vector<double> cum_fitness(n);
        double cum = 0.0;
        for (int i = 0; i < n; ++i) {
            cum += pop[i].fitness;
            cum_fitness[i] = cum;
        }
        // 隨機數 r in [0, sum_fitness)
        std::uniform_real_distribution<double> uni_rnd(0.0, 1.0);
        double r = uni_rnd(rng) * sum_fitness;
        // 二分搜尋或線性搜尋找到第一個 cum_fitness[j] >= r
        int left = 0, right = n - 1, selected = n-1;
        while (left <= right) {
            int mid = (left + right) / 2;
            if (cum_fitness[mid] >= r) {
                selected = mid;
                right = mid - 1;
            } else {
                left = mid + 1;
            }
        }
        return selected;
    }
    // 建立 Selection_Roulette 函式：
    void Selection_Roulette(const std::vector<Individual>& old_pop, vector<Individual>& mating_pool, const GA_Params& params) {
        int pop_size = old_pop.size();
        mating_pool.clear();
        mating_pool.reserve(pop_size);
        for (int i = 0; i < pop_size; ++i) {
            int sel_idx = Roulette_Select(old_pop);
            mating_pool.push_back(old_pop[sel_idx]);
        }
    }

}// End Selection  Define





// Genetic Algorith API , Need To Give The Config And Parameter of GA
Solution Genetic_Algorithm(Config& config , const GA_Params& params , vector<double>* GB_Recorder = nullptr , vector<double>* LB_Recorder = nullptr) {
    
    // Population
    std::vector<Individual> population;
    population.reserve(params.population_size);
    for (int i = 0; i < params.population_size; ++i) {
        population.emplace_back(config);
    }
    Individual best_so_far = population[0];


    std::vector<Individual> mating_pool, next_pop;
    mating_pool.reserve(params.population_size);
    next_pop.reserve(params.population_size);

    // Evolutaion Iteration
    for (int gen = 0; gen < params.generations; ++gen) {

        // Selection_Tournament
        if      (params.selection_method == "t") Selection_For_GA::Selection_Tournament(population, mating_pool, params);
        else if (params.selection_method == "r") Selection_For_GA::Selection_Roulette  (population, mating_pool, params);
        else Selection_For_GA::Selection_Tournament(population, mating_pool, params);
        next_pop.clear();
        // Mating
        for (int i = 0; i < params.population_size; ++i) {
            Individual& parent1 = mating_pool[i];
            
            std::uniform_int_distribution<int> dist(0, params.population_size-1);
            Individual& parent2 = mating_pool[dist(rng)];

            Individual child = parent1.crossover(parent2, config, params.crossover_rate);
            child.mutate(config, params.mutation_rate);

            next_pop.push_back(std::move(child));
        }

        
        population.swap(next_pop);


        if (GB_Recorder || LB_Recorder) {
            double best_cost = std::numeric_limits<double>::infinity();
            double total_cost = 0.0;

            for (const auto& ind : population) {
                total_cost += ind.cost;
            }

            double avg_cost = total_cost / population.size();

            if (GB_Recorder) GB_Recorder->push_back(best_so_far.cost);
            if (LB_Recorder) LB_Recorder->push_back(avg_cost);   
        }

        for (const auto& ind : population) {
            if (ind.cost < best_so_far.cost) {
                best_so_far = ind;
            }
        }
    }

 
    return static_cast<Solution>(best_so_far);
}



#endif