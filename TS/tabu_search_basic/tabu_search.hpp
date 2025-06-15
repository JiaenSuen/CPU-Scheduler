#include "include/modules.hpp"
#include <deque>
#include <utility>   








double Evaluate(Solution& sol, const Config& cfg) {
    ScheduleResult res = Solution_Function(sol, cfg, false);
    double cost = static_cast<double>(res.makespan);
    sol.cost = cost;   
    return cost;
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



// ----- TS Interface ------



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

    
    double c = Evaluate(neighbor, cfg);

    return NeighborInfo{ neighbor, m, c };
}

//-------------------------------











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
            it->second -= 1;  
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
Solution Tabu_Search(const Config& cfg, Solution* Initial_Solution = nullptr  , int maxIter = 10 , int tabuTenure = 5 , int numCandidates = 20 , vector<double>* GB_Recorder = nullptr ,vector<double>* CB_Recorder= nullptr) {
    // INITIAL SOLUTION
    Solution  current;
    if (Initial_Solution == nullptr)   current       = GenerateInitialSolution(cfg, false);
    else                                current      = *Initial_Solution;

    double currentCost    = Evaluate(current, cfg);
    Solution bestSolution = current;
    double bestCost       = currentCost;

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

        /// 更新 current
        current     = chosen.solution;
        currentCost = chosen.cost;

        // 更新 best
        if (currentCost < bestCost) {
            bestSolution     = current;
            bestSolution.cost = currentCost;
            bestCost         = currentCost;
        }

        // **在這裡一定要執行到兩個 recorder**  
        if (GB_Recorder) GB_Recorder->push_back(bestCost);
        if (CB_Recorder) CB_Recorder->push_back(current.cost);
        /*
        if (iter % 50 == 0) {
            std::cout << "[Tabu] Iter=" << iter
                      << " CurrentCost=" << currentCost
                      << " BestCost=" << bestCost << std::endl;
        }*/
    }

    return bestSolution;
}
