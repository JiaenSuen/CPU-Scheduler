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
        selection_method = "r";
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
    // Selection :
    // 鍛造式選擇（Tournament Selection）
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
    void Selection(const std::vector<Individual>& old_pop, vector<Individual>& mating_pool, const GA_Params& params) {
        int pop_size = old_pop.size();
        int tour_size = 3;   
        mating_pool.clear();
        mating_pool.reserve(pop_size);

        for (int i = 0; i < pop_size; ++i) {
            int sel_idx = Tournament_Select(old_pop, tour_size);
            mating_pool.push_back(old_pop[sel_idx]);
        }
    }

     
}// End Selection Define





// Genetic Algorith API , Need To Give The Config And Parameter of GA
Solution Genetic_Algorithm(Config& config , const GA_Params& params , vector<double>* GB_Recorder = nullptr , vector<double>* LB_Recorder = nullptr) {
    
    // Population
    std::vector<Individual> population;
    population.reserve(params.population_size);
    for (int i = 0; i < params.population_size; ++i) {
        population.emplace_back(config);
    }

    std::vector<Individual> mating_pool, next_pop;
    mating_pool.reserve(params.population_size);
    next_pop.reserve(params.population_size);

    // Evolutaion Iteration
    for (int gen = 0; gen < params.generations; ++gen) {
        // Selection
        Selection_For_GA::Selection(population, mating_pool, params);

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
    }

 
    auto best_it = std::min_element(
        population.begin(), population.end(),
        [](auto& a, auto& b){ return a.cost < b.cost; }
    );
    Solution best = *best_it;
    

    return best;
}



#endif