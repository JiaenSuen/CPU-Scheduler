// 檔名：src/main.cpp

#include "Discrete_Fox_Agent.hpp"
#include "include/modules.hpp"

#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
     
    std::string config_file = "../../datasets/n4_00.dag";
    Config cfg;
    try {
        cfg = ReadConfigFile(config_file);
    } catch (const std::exception& ex) {
        std::cerr << "Error reading config: " << ex.what() << std::endl;
        return 1;
    }
    int TCount = static_cast<int>(cfg.theTCount);
    int PCount = static_cast<int>(cfg.thePCount);

    // 2. 初始化 FOX 參數 (可自行調整)
    struct FOX_Parameters {
        int MaxIt      = 100;   // 最大迭代
        int P          = 30;     // 狐狸群大小
        double alpha   = 0.7;    // 排序距離權重
        double beta    = 0.3;    // 指派距離權重
        int numExplorationSwaps   = 2;   // 探索階段對 ss 的隨機 swap/reverse 次數
        int numExplorationReassign= 2;   // 探索階段對 ms 的隨機重分配次數
        double pPerturb   = 0.15;        // c1 模式中「微擾」機率
        double pLocalSwap = 0.3;         // c2 模式中做相鄰 swap 的機率
        double pLocalReverse = 0.3;      // c2 模式中做子序列 reverse 的機率
        double pLocalReassign = 0.3;     // c2 模式中做負載再平衡的機率
        int T_noImprove = 20;            // 允許連續多少代沒改善才觸發跳躍
        double pJump    = 0.1;           // 跳躍機率
    };
    FOX_Parameters fpar;

    // 3. 生成 P 隻狐狸 (DiscreteFoxAgent)
    std::vector<DiscreteFoxAgent> foxes;
    foxes.reserve(fpar.P);
    // 共用 rng： modules.hpp 中有 global rng
    extern std::mt19937 rng;
    for (int i = 0; i < fpar.P; ++i) {
        foxes.emplace_back(i, cfg, rng);
        foxes[i].initialize();
    }

    // 4. 找到初始全局最佳
    int globalBestIdx = 0;
    double globalBestFitness = foxes[0].get_Fitness();
    for (int i = 1; i < fpar.P; ++i) {
        if (foxes[i].get_Fitness() > globalBestFitness) {
            globalBestFitness = foxes[i].get_Fitness();
            globalBestIdx = i;
        }
    }
    std::vector<int> globalBestSS = foxes[globalBestIdx].get_best_ss();
    std::vector<int> globalBestMS = foxes[globalBestIdx].get_best_ms();
    double globalBestCost = foxes[globalBestIdx].get_best_cost();

    // 5. 維護每隻狐狸的 noImproveCounter
    std::vector<int> noImproveCount(fpar.P, 0);
    int globalNoImprove = 0; // 計算全局最優若干代沒變

    // 6. 主迴圈
    for (int t = 1; t <= fpar.MaxIt; ++t) {
        // (A) 動態計算探索率 (可自行換公式，這裡線性衰減)
        double p_explore = 1.0 - static_cast<double>(t) / fpar.MaxIt;

        // (B) 對每隻狐狸做更新
        for (int i = 0; i < fpar.P; ++i) {
            // (B1) 計算與自身最佳(Per-Agent Elite)的距離 (若要用)
            double distAgentBest = foxes[i].distance_to_best(fpar.alpha, fpar.beta);

            // (B2) 探索 vs 開發
            double ru = rng() / static_cast<double>(UINT32_MAX);
            if (ru < p_explore) {
                // 探索模式
                foxes[i].update_exploration(fpar.numExplorationSwaps,
                                            fpar.numExplorationReassign);
            } else {
                // 開發模式：先決定 p1, p2 (可隨 t 線性衰減)
                double p1 = 0.9 * (1.0 - static_cast<double>(t) / fpar.MaxIt) + 0.1;
                double p2 = 0.7 * (1.0 - static_cast<double>(t) / fpar.MaxIt) + 0.1;
                foxes[i].update_exploitation(p1, p2, fpar.pPerturb,
                                             fpar.pLocalSwap, fpar.pLocalReverse, fpar.pLocalReassign);
            }

            // (B3) 跳躍機制
            foxes[i].update_jump(noImproveCount[i], fpar.T_noImprove, fpar.pJump);

            // (B4) 更新該隻狐狸的 noImproveCount
            double newFit = foxes[i].get_Fitness();
            if (newFit > foxes[i].get_best_Fitness()) {
                // 如果剛剛在 update_exploration 或 update_exploitation 中更新了 best_*，
                // 就重置 noImproveCount
                noImproveCount[i] = 0;
            } else {
                noImproveCount[i]++;
            }
        }

        // (C) 更新全局最佳
        int newGlobalBestIdx = globalBestIdx;
        double newGlobalBestFit = globalBestFitness;
        for (int i = 0; i < fpar.P; ++i) {
            double fit = foxes[i].get_Fitness();
            if (fit > newGlobalBestFit) {
                newGlobalBestFit = fit;
                newGlobalBestIdx = i;
            }
        }
        if (newGlobalBestIdx != globalBestIdx) {
            globalBestIdx = newGlobalBestIdx;
            globalBestFitness = newGlobalBestFit;
            globalBestSS = foxes[globalBestIdx].get_best_ss();
            globalBestMS = foxes[globalBestIdx].get_best_ms();
            globalBestCost = foxes[globalBestIdx].get_best_cost();
            globalNoImprove = 0;
        } else {
            globalNoImprove++;
        }

        // (D) 提前終止判斷：若全局最優連續 T_noImprove 代沒變，直接 break
        if (globalNoImprove >= fpar.T_noImprove) {
            std::cout << "[Info] Terminated early at iteration " << t << "\n";
            break;
        }
    }

    // 7. 輸出最終結果
    std::cout << "=== Discrete FOA Result ===\n";
    std::cout << "Best makespan = " << globalBestCost << "\n";
    std::cout << "Best ss (task order): ";
    os_display::show_vector(globalBestSS);
    std::cout << "Best ms (machine assign): ";
    os_display::show_vector(globalBestMS);

    return 0;
}
