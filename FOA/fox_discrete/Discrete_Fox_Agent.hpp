 
#ifndef DISCRETE_FOX_AGENT_HPP
#define DISCRETE_FOX_AGENT_HPP

#include "include/modules.hpp"
 
#include <vector>
#include <random>
#include <algorithm>
#include <limits>

class DiscreteFoxAgent {
private:
    int id;                      // 狐狸編號
    int TCount;                  // 任務數量 (從 Config.theTCount 取得)
    const Config* cfg_ptr;       // 指向全域問題設定
    // 目前解 (離散)
    std::vector<int> ss;         // 任務執行順序 (長度 = TCount)
    std::vector<int> ms;         // 每個任務對應機器 (長度 = TCount)
    double cost;                 // 目前 makespan
    double Fitness;              // 目前適應度 = 1.0 / cost
    // 這隻狐狸的歷代最佳解 (Per‐Agent Elite)
    std::vector<int> best_ss;
    std::vector<int> best_ms;
    double best_cost;
    double best_Fitness;

    // 隨機引擎 (共用外部 rng)
    std::mt19937& rng;
    std::uniform_real_distribution<double> uni01; // [0,1)

public:
    DiscreteFoxAgent(int id_, const Config& cfg, std::mt19937& rng_)
        : id(id_),
          TCount(static_cast<int>(cfg.theTCount)),
          cfg_ptr(&cfg),
          cost(std::numeric_limits<double>::infinity()),
          Fitness(0.0),
          best_cost(std::numeric_limits<double>::infinity()),
          best_Fitness(0.0),
          rng(rng_),
          uni01(0.0, 1.0)
    {
        ss.resize(TCount);
        ms.resize(TCount);
        best_ss.resize(TCount);
        best_ms.resize(TCount);
    }

    // =========================
    // 1. 隨機初始化離散解
    // =========================
    void initialize() {
        // 1.1 隨機產生一個排列 ss = {0,1,2,...,TCount-1} 的亂序
        for (int i = 0; i < TCount; ++i) ss[i] = i;
        std::shuffle(ss.begin(), ss.end(), rng);

        // 1.2 隨機產生 ms：每個任務隨機分配到 0..(PCount-1) 其中一台處理器
        std::uniform_int_distribution<int> uniMach(0, static_cast<int>(cfg_ptr->thePCount) - 1);
        for (int i = 0; i < TCount; ++i) {
            ms[i] = uniMach(rng);
        }
        // 1.3 計算一次 cost/Fitness，並更新歷代最佳
        calculate_fitness(true);
    }

    // =========================
    // 2. 計算適應值 (makespan & Fitness)
    //    若 update_best=true，則同時更新 best_* 
    // =========================
    double calculate_fitness(bool update_best = false) {
        // 2.1 用當前 ss, ms 計算 schedule
        Solution sol;
        sol.ss = ss;
        sol.ms = ms;
        ScheduleResult res = Solution_Function(sol, *cfg_ptr);
        cost = res.makespan;
        Fitness = 1.0 / (cost + 1e-9); // 避免除以 0

        // 2.2 更新歷代最佳
        if (update_best && cost < best_cost) {
            best_cost = cost;
            best_Fitness = Fitness;
            best_ss = ss;
            best_ms = ms;
        }
        return Fitness;
    }

    // =========================
    // 3. 計算「與自身最佳」的距離
    //    距離 = alpha * Hamming(ss, best_ss) + beta * Hamming(ms, best_ms)
    // =========================
    double distance_to_best(double alpha, double beta) const {
        int diff_seq = 0;
        for (int k = 0; k < TCount; ++k) {
            if (ss[k] != best_ss[k]) diff_seq++;
        }
        int diff_mach = 0;
        for (int k = 0; k < TCount; ++k) {
            if (ms[k] != best_ms[k]) diff_mach++;
        }
        return alpha * diff_seq + beta * diff_mach;
    }

    // =========================
    // 4. 探索模式 (Exploration)
    //    大幅隨機擾動 ss, ms
    //    構想：執行 numExplorationSwaps 次 swap/reverse；執行 numExplorationReassign 次隨機重分配
    // =========================
    void update_exploration(int numExplorationSwaps, int numExplorationReassign) {
        // 保存舊解
        auto old_ss = ss;
        auto old_ms = ms;
        double old_cost = cost;

        // (A) 隨機擾動 ss
        std::uniform_int_distribution<int> uniIdx(0, TCount - 1);
        for (int r = 0; r < numExplorationSwaps; ++r) {
            double z = uni01(rng);
            if (z < 0.5) {
                // 隨機 swap 兩個索引
                int i = uniIdx(rng);
                int j = uniIdx(rng);
                std::swap(ss[i], ss[j]);
            } else {
                // 隨機反轉 (reverse) 一段子序列
                int i = uniIdx(rng);
                int j = uniIdx(rng);
                if (i > j) std::swap(i, j);
                if (i < j) std::reverse(ss.begin() + i, ss.begin() + j + 1);
            }
        }

        // (B) 隨機擾動 ms
        std::uniform_int_distribution<int> uniMach(0, static_cast<int>(cfg_ptr->thePCount) - 1);
        std::uniform_int_distribution<int> uniTask(0, TCount - 1);
        for (int r = 0; r < numExplorationReassign; ++r) {
            int t = uniTask(rng);
            ms[t] = uniMach(rng);
        }

        // (C) 計算新適應，如果沒有改善就回滾
        double newFit = calculate_fitness(false);
        if (newFit <= 1.0 / (old_cost + 1e-9)) {
            ss = old_ss;
            ms = old_ms;
            cost = old_cost;
            Fitness = 1.0 / (cost + 1e-9);
        }
    }

    // =========================
    // 5. 開發模式 (Exploitation)
    //    分成 c1 (部分靠攏 + 微量擾動) 及 c2 (局部微調)
    //    參數：
    //      p1: ss 向最優靠攏的機率；
    //      p2: ms 向最優靠攏的機率；
    //      pPerturb: 未對齊時的小範圍隨機擾動機率；
    //      pLocalSwap / pLocalReverse / pLocalReassign: c2 的微調機率
    // =========================
    void update_exploitation(double p1, double p2, double pPerturb,
                             double pLocalSwap, double pLocalReverse, double pLocalReassign) 
    {
        // 保存舊解
        auto old_ss = ss;
        auto old_ms = ms;
        double old_cost = cost;

        // 決定 c1 或 c2
        double z = uni01(rng);
        if (z < 0.5) {
            // === c1: 部分靠攏最優 + 微量隨機擾動 ===
            // (A) ss 的部分靠攏
            for (int k = 0; k < TCount; ++k) {
                if (ss[k] != best_ss[k]) {
                    double z2 = uni01(rng);
                    if (z2 < p1) {
                        // 對齊到 best
                        ss[k] = best_ss[k];
                    } else {
                        double z3 = uni01(rng);
                        if (z3 < pPerturb) {
                            // 小範圍 swap：與相鄰一位交換
                            if (k < TCount - 1) std::swap(ss[k], ss[k + 1]);
                            else std::swap(ss[k], ss[k - 1]);
                        }
                    }
                }
            }
            // (B) ms 的部分靠攏
            std::uniform_int_distribution<int> uniMach(0, static_cast<int>(cfg_ptr->thePCount) - 1);
            for (int k = 0; k < TCount; ++k) {
                if (ms[k] != best_ms[k]) {
                    double z4 = uni01(rng);
                    if (z4 < p2) {
                        ms[k] = best_ms[k];
                    } else {
                        double z5 = uni01(rng);
                        if (z5 < pPerturb) {
                            ms[k] = uniMach(rng);
                        }
                    }
                }
            }
        } else {
            // === c2: 局部微調 (Local Refinement) ===
            // (A) ss 的鄰位 swap / 子序列 reverse
            double z6 = uni01(rng);
            if (z6 < pLocalSwap) {
                std::uniform_int_distribution<int> uniI(0, TCount - 2);
                int i = uniI(rng);
                std::swap(ss[i], ss[i + 1]);
            }
            double z7 = uni01(rng);
            if (z7 < pLocalReverse) {
                std::uniform_int_distribution<int> uniI(0, TCount - 2);
                int i = uniI(rng);
                std::uniform_int_distribution<int> uniLen(1, TCount - 1 - i);
                int L = uniLen(rng);
                std::reverse(ss.begin() + i, ss.begin() + i + L);
            }
            // (B) ms 的負載再平衡
            double z8 = uni01(rng);
            if (z8 < pLocalReassign) {
                // 找出機器負載最重者
                int heavy = find_heavy_machine();
                auto tasksOnHeavy = tasks_of_machine(heavy);
                if (static_cast<int>(tasksOnHeavy.size()) >= 2) {
                    std::uniform_int_distribution<int> uniIdx(0, static_cast<int>(tasksOnHeavy.size()) - 2);
                    int pos = uniIdx(rng);
                    int t1 = tasksOnHeavy[pos];
                    int t2 = tasksOnHeavy[pos + 1];
                    std::swap(ms[t1], ms[t2]);
                }
            }
        }

        // 計算新適應，若沒有改善就回滾
        double newFit = calculate_fitness(false);
        if (newFit <= 1.0 / (old_cost + 1e-9)) {
            ss = old_ss;
            ms = old_ms;
            cost = old_cost;
            Fitness = 1.0 / (cost + 1e-9);
        }
    }

    // =========================
    // 6. 跳躍機制 (Jump)
    //    若連續 noImproveCounter >= T_noImprove，就觸發全重置或部分重置
    // =========================
    void update_jump(int noImproveCounter, int T_noImprove, double pJump) {
        if (noImproveCounter >= T_noImprove) {
            double z = uni01(rng);
            if (z < pJump) {
                double z2 = uni01(rng);
                if (z2 < 0.5) {
                    // 全重置：重新隨機產生 ss, ms
                    for (int i = 0; i < TCount; ++i) ss[i] = i;
                    std::shuffle(ss.begin(), ss.end(), rng);
                    std::uniform_int_distribution<int> uniMach(0, static_cast<int>(cfg_ptr->thePCount) - 1);
                    for (int i = 0; i < TCount; ++i) {
                        ms[i] = uniMach(rng);
                    }
                } else {
                    // 部分重置：ss 打亂 50%，ms 隨機重設 30%
                    int numShuffle = TCount / 2;
                    std::uniform_int_distribution<int> uniI(0, TCount - 1);
                    for (int r = 0; r < numShuffle; ++r) {
                        int i = uniI(rng), j = uniI(rng);
                        std::swap(ss[i], ss[j]);
                    }
                    int numReassign = (TCount * 3) / 10;
                    std::uniform_int_distribution<int> uniTask(0, TCount - 1);
                    std::uniform_int_distribution<int> uniMach(0, static_cast<int>(cfg_ptr->thePCount) - 1);
                    for (int r = 0; r < numReassign; ++r) {
                        int t = uniTask(rng);
                        ms[t] = uniMach(rng);
                    }
                }
                // 跳躍後更新適應度
                calculate_fitness(false);
            }
        }
    }

    // =========================
    // 輔助函式：找到負載最重的機器
    //    先基於當前 ss, ms 計算各機器上的累積加工完成時間 (procFree)
    // =========================
    int find_heavy_machine() const {
        int P = static_cast<int>(cfg_ptr->thePCount);
        std::vector<double> procFree(P, 0.0);
        // 先模擬已有排程，只需考慮 ms, ss 的順序
        std::vector<double> startT(TCount, 0.0), endT(TCount, 0.0);
        for (int idx = 0; idx < TCount; ++idx) {
            int t = ss[idx];
            int p = ms[t];
            double ready = 0.0;
            auto it = cfg_ptr->predMap.find(t);
            if (it != cfg_ptr->predMap.end()) {
                for (auto &pr : it->second) {
                    int from = pr.first;
                    double vol = pr.second;
                    int pf = ms[from];
                    double commDelay = (pf != p) ? (vol * cfg_ptr->theCommRate[pf][p]) : 0.0;
                    ready = std::max(ready, endT[from] + commDelay);
                }
            }
            startT[t] = std::max(ready, procFree[p]);
            endT[t] = startT[t] + cfg_ptr->theCompCost[t][p];
            procFree[p] = endT[t];
        }
        // 找最大 procFree
        int heavy = 0;
        double maxLoad = procFree[0];
        for (int i = 1; i < P; ++i) {
            if (procFree[i] > maxLoad) {
                maxLoad = procFree[i];
                heavy = i;
            }
        }
        return heavy;
    }

    // =========================
    // 輔助函式：回傳當前被指派到 machine m 的任務列表
    // =========================
    std::vector<int> tasks_of_machine(int m) const {
        std::vector<int> list;
        for (int t = 0; t < TCount; ++t) {
            if (ms[t] == m) list.push_back(t);
        }
        return list;
    }

    // =========================
    // Getter / Setter
    // =========================
    const std::vector<int>& get_ss()     const { return ss; }
    const std::vector<int>& get_ms()     const { return ms; }
    double get_cost()                    const { return cost; }
    double get_Fitness()                 const { return Fitness; }
    const std::vector<int>& get_best_ss()const { return best_ss; }
    const std::vector<int>& get_best_ms()const { return best_ms; }
    double get_best_cost()               const { return best_cost; }
    double get_best_Fitness()            const { return best_Fitness; }
};

#endif // DISCRETE_FOX_AGENT_HPP
