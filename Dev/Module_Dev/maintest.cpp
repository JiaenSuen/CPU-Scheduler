 #include <iostream>
#include <vector>
#include <chrono>
#include "include/config.hpp"
#include "include/utils.hpp"
#include "include/evaluation.hpp"

using namespace std;
using namespace os_display;
using namespace std::chrono;

int main() {

    try {
        // config
        Config config = ReadConfigFile("config_A01.txt");
        cout << "Loaded config: PCount=" << config.thePCount
             << ", TCount=" << config.theTCount
             << ", ECount=" << config.theECount << "\n";

        // Test Case 
        Solution sol;
        sol.ss = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                  10,11,12,13,14,15,16,17,18,19};
        sol.ms = {1, 0, 2, 3, 1, 2, 0, 1, 2, 3,
                  2, 0, 1, 2, 0, 2, 1, 2, 0, 3};

        vector<Solution> tests = { sol };
        show_Config(config);
        for (size_t i = 0; i < tests.size(); ++i) {
            cout << "\n=== Test #" << i+1 << " ===\n";
            const Solution& s = tests[i];

            // Calculate_schedule
            auto t0 = high_resolution_clock::now();
            ScheduleResult res0 = Calculate_schedule(s.ss, s.ms, config);
            auto t1 = high_resolution_clock::now();
            auto dur0 = duration_cast<microseconds>(t1 - t0);

            cout << "[No Adjust] Makespan = " << res0.makespan
                 << ", Time = " << dur0.count() << " us\n";
            cout << "Start times: "; show_vector(res0.startTime);
            cout << "End times:   "; show_vector(res0.endTime);

            // 經 Solution_Function 
            Solution s_adj = s; 
            auto t2 = high_resolution_clock::now();
            ScheduleResult res1 = Solution_Function(s_adj, config);
            auto t3 = high_resolution_clock::now();
            auto dur1 = duration_cast<microseconds>(t3 - t2);

            cout << "[Use ] Makespan = " << res1.makespan
                 << ", Time = " << dur1.count() << " us\n";
            cout << "Final ss : "; show_vector(s_adj.ss);
            cout << "Final ms : "; show_vector(s_adj.ms);
            cout << "Start times: "; show_vector(res1.startTime);
            cout << "End times:   "; show_vector(res1.endTime);
        }
    } catch (const exception& ex) {
        cerr << "Exception: " << ex.what() << endl;
        return 1;
    }
    return 0;
}
