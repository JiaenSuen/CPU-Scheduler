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
    int population_size;
    int generations;
    double crossover_rate;
    double mutation_rate;
    std::string selection_method;
};

GA_Params Init_GA_Params() {
    GA_Params params;
    params.population_size = 100;
    params.generations = 200;
    params.crossover_rate = 0.7;
    params.mutation_rate = 0.4;
    params.selection_method = "roulette";
    return params;
}
 
//-------------------------------


// Individual 
class Individual : public Solution {
public:
    double fitness;  

    Individual() = default;
    Individual(const Config& cfg, bool useHeuristic=false) {
        Solution init = GenerateInitialSolution(cfg, useHeuristic);
        this->ss = std::move(init.ss);
        this->ms = std::move(init.ms);
        evaluate(cfg);
    }

    //  Mating
    Individual crossover(const Individual& other,
                         const Config& cfg,
                         double crossover_rate) const {
        int T = cfg.theTCount;
        int P = cfg.thePCount;
        std::uniform_real_distribution<double> uni01(0.0, 1.0);
        std::uniform_int_distribution<int> cutDist(0, T-1);

        Individual child = *this;  

        //  OX for ss
        if (uni01(rng) < crossover_rate) {
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
            if (uni01(rng) < 0.5)
                child.ms[i] = this->ms[i];
            else
                child.ms[i] = other.ms[i];
        }

        // Return Child
        child.evaluate(cfg);
        return child;
    }

    // 3) Mutation
    void mutate(const Config& cfg, double mutation_rate) {
        int T = cfg.theTCount;
        int P = cfg.thePCount;
        std::uniform_real_distribution<double> uni01(0.0, 1.0);
        std::uniform_int_distribution<int> posDist(0, T-1);
        std::uniform_int_distribution<int> procDist(0, P-1);

        bool changed = false;
        //  ss 交換突變
        if (uni01(rng) < mutation_rate) {
            int i = posDist(rng), j = posDist(rng);
            std::swap(ss[i], ss[j]);
            changed = true;
        }
        //  ms 隨機重設
        if (uni01(rng) < mutation_rate) {
            int k = posDist(rng);
            ms[k] = procDist(rng);
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


 
// Selection :
    // 鍛造式選擇（Tournament Selection）
    int Tournament_Select(const std::vector<Individual>& pop,
                     int tournament_size) {
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
    void Selection(const std::vector<Individual>& old_pop,
                std::vector<Individual>& mating_pool,
                const GA_Params& params) {
        int pop_size = old_pop.size();
        int tour_size = 3;   
        mating_pool.clear();
        mating_pool.reserve(pop_size);

        for (int i = 0; i < pop_size; ++i) {
            int sel_idx = Tournament_Select(old_pop, tour_size);
            mating_pool.push_back(old_pop[sel_idx]);
        }
    }

// End Selection 


 
 
Solution Genetic_Algorithm(Config& config) {
    // Setting
    
    GA_Params params = Init_GA_Params();

    // Population
    std::vector<Individual> population;
    population.reserve(params.population_size);
    for (int i = 0; i < params.population_size; ++i) {
        population.emplace_back(config ,  false);
    }

    std::vector<Individual> mating_pool, next_pop;
    mating_pool.reserve(params.population_size);
    next_pop.reserve(params.population_size);

    // Evolutaion Iteration
    for (int gen = 0; gen < params.generations; ++gen) {
        // Selection
        Selection(population, mating_pool, params);

        next_pop.clear();
        // Mating
        for (int i = 0; i < params.population_size; ++i) {
            const Individual& parent1 = mating_pool[i];
            
            std::uniform_int_distribution<int> dist(0, params.population_size-1);
            const Individual& parent2 = mating_pool[dist(rng)];

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




// ----- TS Interface ------

 
double Evaluate( Solution& sol, const Config& cfg) {
    ScheduleResult res = Solution_Function(sol, cfg, false);
    return static_cast<double>(res.makespan);
}


enum MoveType {
    SWAP_SS,       
    CHANGE_MS
};

struct Move {
    MoveType type;

    // for ss
    int i;   
    int j;   

    // for ms
    int t;     
    int old_P;   
    int new_P;  

    Move() : type(SWAP_SS), i(-1), j(-1), t(-1), old_P(-1), new_P(-1) {}
};


struct NeighborInfo {
    Solution solution;   
    Move move;          
    double cost;        
};

NeighborInfo Tabu_Generate_Neighbor(const Solution& current, const Config& cfg) {
    Solution neighbor = current;      
    int T = cfg.theTCount;
    int P = cfg.thePCount;

    std::uniform_int_distribution<int> moveDist(0, 1);
    int choice = moveDist(rng);

    Move m;
    if (choice == 0) {
        // Swap SS
        m.type = SWAP_SS;
        std::uniform_int_distribution<int> distT(0, T - 1);
        int i = distT(rng), j = distT(rng);
        while (j == i) j = distT(rng);
        m.i = i; m.j = j;
        std::swap(neighbor.ss[i], neighbor.ss[j]);
    } else {
        // Change MS
        m.type = CHANGE_MS;
        std::uniform_int_distribution<int> distT(0, T - 1);
        std::uniform_int_distribution<int> distP(0, P - 1);
        int t = distT(rng);
        int newP = distP(rng);
        while (newP == neighbor.ms[t] && P > 1) {
            newP = distP(rng);
        }
        m.t     = t;
        m.old_P = neighbor.ms[t];
        m.new_P = newP;
        neighbor.ms[t] = newP;
    }

    // 這裡呼叫 Evaluate 時，傳入 neighbor 的「複本」，不會修改 neighbor 本身
    double c = Evaluate(neighbor, cfg);

    return NeighborInfo{ neighbor, m, c };
}

//-------------------------------


#include <deque>
#include <utility>   

class Tabu_List {
public:
    // 建構：傳進 tabuTenure（正整數），代表每個 Move 在禁忌清單中保留多少輪
    Tabu_List(int tabuTenure) : maxTenure(tabuTenure) {}

    // (1) 把一個 Move 加入禁忌
    void add(const Move& m) {
        // 如果 Tabu List 中已有相同的 Move，只要把它的期限重設即可
        for (auto& entry : list_) {
            if (SameMove(entry.first, m)) {
                entry.second = maxTenure;
                return;
            }
        }
        // 否則就 push_back
        list_.push_back(std::make_pair(m, maxTenure));
    }

    // (2) 判斷某個 Move 是否在禁忌清單裡（仍有剩餘期限）
    bool contains(const Move& m) const {
        for (const auto& entry : list_) {
            if (SameMove(entry.first, m) && entry.second > 0) {
                return true;
            }
        }
        return false;
    }

    // (3) 每一次主迴圈結束，都要呼叫一次 decrementTenure()，
    void decrementTenure() {
        for (auto it = list_.begin(); it != list_.end();) {
            it->second -= 1;  // 期限 -1
            if (it->second <= 0) {
                it = list_.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    int maxTenure;
    // list_ 裡每一項存 pair<Move, remainingTenure>
    std::deque<std::pair<Move,int>> list_;

    // 比較兩個 Move 是否相同（若型態與參數都相同，視為相同 Move）
    bool SameMove(const Move& a, const Move& b) const {
        if (a.type != b.type) return false;
        if (a.type == SWAP_SS) {
            return ( (a.i == b.i && a.j == b.j) 
                  || (a.i == b.j && a.j == b.i) );
        }
        else if (a.type == CHANGE_MS) {
            return (a.t == b.t && a.old_P == b.old_P && a.new_P == b.new_P);
        }
        return false;
    }
};



// 主 Tabu Search 演算法
Solution Tabu_Search_with_GA(Config& cfg, int maxIter, int tabuTenure, int numCandidates) {
    // INITIAL SOLUTION
    Solution current      =  Genetic_Algorithm(cfg);
    double currentCost    =  Evaluate(current, cfg);
    Solution bestSolution =  current;
    double bestCost       =  currentCost;

    // Tabu List
    Tabu_List tabuList(tabuTenure);

    // Iteration
    for (int iter = 0; iter < maxIter; ++iter) {
        // Generate Neighbors
        std::vector<NeighborInfo> candidates;
        candidates.reserve(numCandidates);
        for (int k = 0; k < numCandidates; ++k) {
            NeighborInfo ni = Tabu_Generate_Neighbor(current, cfg);
            candidates.push_back(ni);
        }

        // 選出最佳非禁忌或符合 Aspiration 的
        bool found = false;
        NeighborInfo chosen;
        for (const auto& ni : candidates) {
            bool isTabu = tabuList.contains(ni.move);
            bool aspiration = (ni.cost < bestCost);
            if (!isTabu || aspiration) {
                if (!found || ni.cost < chosen.cost) {
                    chosen = ni;
                    found = true;
                }
            }
        }
        if (!found) {
            // 全部都是禁忌且沒比 bestCost 還好，就挑最小 cost
            chosen = *std::min_element(
                candidates.begin(), candidates.end(),
                [](auto& a, auto& b){ return a.cost < b.cost; }
            );
        }

        //  更新 Tabu List
        tabuList.add(chosen.move);

        //  更新 current
        current     = chosen.solution;
        currentCost = chosen.cost;

        //  更新最佳
        if (currentCost < bestCost) {
            current.cost = currentCost;     
            bestSolution = current;            
            bestCost     = currentCost;
        }

        //  扣減禁忌期限
        tabuList.decrementTenure();

      
        if (iter % 50 == 0) {
            std::cout << "[Tabu] Iter=" << iter
                      << " CurrentCost=" << currentCost
                      << " BestCost=" << bestCost << std::endl;
        }
    }

    return bestSolution;
}


  
int main() {
    Config cfg = ReadConfigFile("../../datasets/n4_00.dag");
    


    int maxIter       = 100;    // 最大迭代次數  
    int tabuTenure    = 10;    // 禁忌期限  
    int numCandidates = 40;   // 一次產生的鄰居數量  

    double Avg_Makespan =0;
    double times = 10;

    for (int i ;i<times;i++){
        Solution best = Tabu_Search_with_GA(cfg, maxIter, tabuTenure, numCandidates);
        cout << "Best makespan: " << best.cost << "\n";
        ScheduleResult sr = Solution_Function(best, cfg , true);
        show_solution(best);
        cout << "Feasible: " << std::boolalpha << is_feasible(sr, cfg) << "\n";
        cout << "Cost : " << sr.makespan;
        cout<<"\n\n";
        Avg_Makespan+=best.cost;
    }
    Avg_Makespan /= times;
    system("cls");
    cout<<"Avg Makespan : "<<Avg_Makespan;
    return 0;
}