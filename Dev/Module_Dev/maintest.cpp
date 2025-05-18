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


        vector<Solution> tests = { 
            {
                {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19},
                {1, 0, 2, 3, 1, 2, 0, 1, 2, 3,2, 0, 1, 2, 0, 2, 1, 2, 0, 3}
            },
            {
                {0, 1, 2, 5, 4, 3, 6, 7, 9, 8, 11, 10, 12, 14, 15, 13, 16, 17, 18, 19},
                {2, 0, 1, 2, 1, 3, 0, 2, 1, 0, 2, 3, 0, 2, 3, 1, 2, 3, 0, 2},
            },
            {
                {0, 1, 2, 6, 5, 4, 3, 7, 11, 8, 9, 10, 12, 13, 14, 15, 16, 18, 17, 19},
                {3, 1, 2, 0, 2, 1, 3, 0, 2, 0, 3, 1, 2, 0, 1, 2, 3, 0, 0, 1}
            },
            {
                {0, 1, 6, 3, 2, 5, 4, 7, 10, 9, 11, 8, 12, 15, 14, 13, 16, 18, 17, 19},
                {2, 3, 2, 0, 2, 1, 3, 2, 0, 3, 2, 3, 2, 2, 1, 2, 3, 0, 2, 0}
            },
            {
                {0, 1, 4, 5, 6, 3, 2, 7, 8, 9, 10, 11, 12, 14, 15, 13, 16, 18, 17, 19},
                {2, 2, 1, 0, 3, 2, 2, 3, 3, 0, 1, 0, 2, 3, 0, 3, 3, 1, 0, 2}
            },
            {
                {0, 1, 3, 2, 5, 4, 12, 13, 7, 10, 9, 11, 16, 8, 15, 14, 18, 17, 6, 19},
                {0, 1, 0, 2, 0, 3, 1, 0, 2, 1, 0, 1, 0, 0, 3, 0, 1, 2, 0, 2}
            },
        };
        // Convert Test :
        vector<double> ss_d = {0.0, 0.18, 0.33, 0.27, 0.54, 0.41, 0.12, 0.13, 0.77, 0.17, 0.93, 0.11, 0.16, 0.85, 0.15, 0.41, 0.88, 0.74, 0.63, 0.19};
        vector<double> ms_d = {0.64, 0.52, 0.28, 0.11, 0.89, 0.69, 0.72, 0.91, 0.78, 0.16, 0.38, 0.22, 0.67, 0.88, 0.02, 0.94, 0.87, 0.49, 0.14, 0.53};
        Solution S_d;
        S_d.ss = Converter::FloatArrayToRankIndex(ss_d);
        S_d.ms = Converter::FloatToDiscreteClass(ms_d,config.thePCount);
        tests.push_back(S_d);
        // End - Convert Test 


        // End Test Case 

        
        show_Config(config);
        for (size_t i = 0; i < tests.size(); ++i) {
            cout << "\n=== Test #" << i+1 << " ===\n";
            const Solution& s = tests[i];

            // Calculate_schedule
            auto t0 = high_resolution_clock::now();
            ScheduleResult res0 = Calculate_schedule(s.ss, s.ms, config);
            auto t1 = high_resolution_clock::now();
            auto dur0 = duration_cast<microseconds>(t1 - t0);

            cout << "[No Adjust Function] Makespan = " << res0.makespan
                 << ", Time = " << dur0.count() << " us\n";
            cout << "Start times: "; show_vector(res0.startTime);
            cout << "End times:   "; show_vector(res0.endTime);

            // 經 Solution_Function 
            Solution s_adj = s; 
            auto t2 = high_resolution_clock::now();
            ScheduleResult res1 = Solution_Function(s_adj, config);
            auto t3 = high_resolution_clock::now();
            auto dur1 = duration_cast<microseconds>(t3 - t2);

            cout << "[Use AdjustFunction] Makespan = " << res1.makespan
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
    cout<<"\n\n\n";
    system("pause");
    return 0;
}
