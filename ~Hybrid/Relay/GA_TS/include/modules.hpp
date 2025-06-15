#ifndef MODULES_HPP
#define MODULES_HPP


#include "config.hpp"
#include "evaluation.hpp"
#include "utils.hpp"

#include <deque>

#include <numeric>
#include <random>
using namespace std;
std::mt19937 rng(std::random_device{}());




Solution GenerateInitialSolution(const Config& cfg, bool useHeuristic=false){
    int T = cfg.theTCount;
    int P = cfg.thePCount;
    Solution sol;

    sol.ss.clear();
    sol.ms.assign(T, 0);

    if (!useHeuristic) {
        // 隨機：先隨機順序，再隨機匹配
        sol.ss.resize(T);
        std::iota(sol.ss.begin(), sol.ss.end(), 0);
        std::shuffle(sol.ss.begin(), sol.ss.end(), rng);
        for (int t : sol.ss) {
            sol.ms[t] = rng() % P;
        }
        return sol;
    }

    // ========== Heuristic: List Scheduling with Topological Order ==========

    // 1. 建 succMap，以便從任務到後繼任務
    std::vector<std::vector<int>> succ(T);
    for (auto &e : cfg.theTransDataVol) {
        int from = (int)e[0], to = (int)e[1];
        succ[from].push_back(to);
    }

    // 2. 計算初始 indegree
    std::vector<int> indegree(T, 0);
    for (auto &e : cfg.theTransDataVol) {
        indegree[(int)e[1]]++;
    }

    // 3. 處理器可用時間 & 任務啟動／結束時間
    std::vector<double> procFree(P, 0.0);
    std::vector<double> startTime(T, 0.0), endTime(T, 0.0);

    // 4. 初始 ready 清單：所有 indegree==0 的任務
    std::deque<int> ready;
    for (int t = 0; t < T; ++t) {
        if (indegree[t] == 0)
            ready.push_back(t);
    }

    // 5. 迴圈：直到所有任務都被排
    while (!ready.empty()) {
        // 這裡可以改成優先隊列，或隨機挑一個。這裡示範 FIFO。
        int t = ready.front();
        ready.pop_front();

        // ── 在所有處理器上試算完成時間，選最佳 ──
        double bestFinish = std::numeric_limits<double>::infinity();
        int    bestProc  = 0;

        // 計算該任務的「就緒時間」（考前驅通訊延遲）
        double readyTime = 0.0;
        auto it = cfg.predMap.find(t);
        if (it != cfg.predMap.end()) {
            for (auto &pr : it->second) {
                int from = pr.first;
                double vol = pr.second;
                int pf = sol.ms[from];
                double comm = (pf != bestProc)
                    ? vol * cfg.theCommRate[pf][bestProc]
                    : 0.0;
                readyTime = std::max(readyTime, endTime[from] + comm);
            }
        }

        // 嘗試每個 processor
        for (int p = 0; p < P; ++p) {
            double commReady = 0.0;
            if (it != cfg.predMap.end()) {
                // 重新計算通訊延遲對應到 p
                for (auto &pr : it->second) {
                    int from = pr.first;
                    double vol = pr.second;
                    int pf = sol.ms[from];
                    double comm = (pf != p)
                        ? vol * cfg.theCommRate[pf][p]
                        : 0.0;
                    commReady = std::max(commReady, endTime[from] + comm);
                }
            }
            double estStart  = std::max(procFree[p], commReady);
            double estFinish = estStart + cfg.theCompCost[t][p];
            if (estFinish < bestFinish) {
                bestFinish = estFinish;
                bestProc   = p;
                readyTime  = commReady;
            }
        }

        // 指派 t → bestProc
        sol.ss.push_back(t);
        sol.ms[t]   = bestProc;
        startTime[t] = std::max(procFree[bestProc], readyTime);
        endTime[t]   = bestFinish;
        procFree[bestProc] = bestFinish;

        // 更新後繼任務 indegree，若變零就加入 ready
        for (int u : succ[t]) {
            if (--indegree[u] == 0)
                ready.push_back(u);
        }
    }

    return sol;
}


#endif