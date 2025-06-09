#ifndef EVALUATION_HPP
#define EVALUATION_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include "config.hpp"

using namespace std;

inline ScheduleResult Calculate_schedule(const vector<int>& ss, const vector<int>& ms, const Config& config) {
    int T = config.theTCount;
    int P = config.thePCount;

    vector<double> startTime(T, 0.0), endTime(T, 0.0);
    vector<double> procFree(P, 0.0);

    for (int idx = 0; idx < (int)ss.size(); ++idx) {
        int t = ss[idx];
        int p = ms[t];

        double ready = 0.0;
        auto it = config.predMap.find(t);
        if (it != config.predMap.end()) {
            for (const auto& pr : it->second) {
                int from = pr.first;
                double vol = pr.second;
                int pf = ms[from];
                double commDelay = (pf != p) ? vol * config.theCommRate[pf][p] : 0.0;
                ready = max(ready, endTime[from] + commDelay);
            }
        }

        startTime[t] = max(ready, procFree[p]);
        endTime[t] = startTime[t] + config.theCompCost[t][p];
        procFree[p] = endTime[t];
    }

    double makespan = *max_element(endTime.begin(), endTime.end());
    return {startTime, endTime, makespan};
}

inline bool is_feasible(const ScheduleResult& result, const Config& config, bool show_adjust = false) {
    for (const auto& edge : config.theTransDataVol) {
        int from = static_cast<int>(edge[0]);
        int to = static_cast<int>(edge[1]);
        if (result.endTime[from] > result.startTime[to]) {
            if (show_adjust) cerr << "[Error] Dependency violated: Task " << from << " ends at " << result.endTime[from] 
                 << ", but Task " << to << " starts at " << result.startTime[to] << ".\n";
            return false;
        }
    }
    return true;
}

inline ScheduleResult Solution_Function(Solution& sol, const Config& config , bool show_adjust = false) {
    int T = config.theTCount;
    int P = config.thePCount;
    vector<int> task_check = sol.ss;
    sort(task_check.begin(), task_check.end());

    for (int i = 0; i < T; ++i) {
        if (task_check[i] != i) {
            cerr << "[Error] Invalid task order in ss.\n";
            return {{}, {}, -1.0};
        }
    }

    if ((int)sol.ms.size() != T) {
        cerr << "[Error] ms size != number of tasks.\n";
        return {{}, {}, -1.0};
    }
    for (int i = 0; i < T; ++i) {
        if (sol.ms[i] < 0 || sol.ms[i] >= P) {
            cerr << "[Error] ms[" << i << "] is out of processor range.\n";
            return {{}, {}, -1.0};
        }
    }

    ScheduleResult result = Calculate_schedule(sol.ss, sol.ms, config);
    bool adjusted_any = false;

    while (!is_feasible(result, config, show_adjust)) {
        adjusted_any = true;

         
        for (const auto& edge : config.theTransDataVol) {
            int from = static_cast<int>(edge[0]);
            int to   = static_cast<int>(edge[1]);

            // 發現衝突
            if (result.endTime[from] > result.startTime[to]) {
                if (show_adjust) {
                    cerr << "[Adjusting] Dependency violated: Task " << from
                         << " ends at " << result.endTime[from]
                         << ", but Task " << to
                         << " starts at " << result.startTime[to] << ".\n";
                }
                // 「to」必須搬到尾端
                auto it = find(sol.ss.begin(), sol.ss.end(), to);
                if (it != sol.ss.end()) {
                    sol.ss.erase(it);
                    sol.ss.push_back(to);
                }
                if (show_adjust) {
                    cout << "[Adjusted] Moved Task " << to << " after Task " << from << ".\n";
                }

                // 重新計算 schedule 並跳出這個 for-loop，從頭再檢查一次
                result = Calculate_schedule(sol.ss, sol.ms, config);
                break;
            }
        }
        // 回到 while 條件，若還有 violation 就繼續
    }

    if (adjusted_any && show_adjust) {
        cout << "[Info] Adjusted solution to become feasible.\n";
    }

    

    sol.cost = result.makespan;
    return result;
}

#endif   
