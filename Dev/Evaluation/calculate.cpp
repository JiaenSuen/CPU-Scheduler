#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <unordered_map>
using namespace std;
using namespace std::chrono;


// Data Structue
// key is Curren Task t，value is previous task from and commuication cost : vol
struct Config {
    unsigned int thePCount, theTCount , theECount;
    vector<vector<double>> theCommRate;
    vector<vector<double>> theCompCost;
    vector<vector<double>> theTransDataVol;

    unordered_map<int, vector<pair<int,double>>> predMap;
};

struct Solution{
    vector<int> ss;
    vector<int> ms;
};

struct ScheduleResult {
    vector<double> startTime;
    vector<double> endTime;
    double makespan;
};

// Display Tools
void show_2d_vector(vector<vector<double>>& vec2d){
    for(auto& vec:vec2d){
        for (auto& data:vec)cout<<setw(4)<<left<<data<<" ";
        cout<<endl;
    }
    cout<<"\n\n";
}

template<typename T>
void show_vector_int(vector<T>& vec){
    for (auto& data:vec)cout<<setw(5)<<left<<data<<" ";
    cout<<"\n";
}


void show_solution(Solution& solution){
    cout<<"ss : "; show_vector_int(solution.ss);
    cout<<"ms : "; show_vector_int(solution.ms);
    cout<<endl;
}

void show_solution_list(vector<Solution>& solution_list){
    for (auto& solution:solution_list) 
        show_solution(solution);
}


void show_Config(Config config_data){
    cout<<"The Num of Processor : "<<config_data.thePCount<<endl;
    cout<<"The Num of Tasks     : "<<config_data.theTCount<<endl;
    cout<<"The Num of Edges     : "<<config_data.theECount<<endl;
    cout<<"\n\n";

    cout<<"The Communication Rate : \n";
    show_2d_vector(config_data.theCommRate);

    cout<<"The Communication Cost : \n";
    show_2d_vector(config_data.theCompCost);

    cout<<"The Transmission Data Volume : \n";
    show_2d_vector(config_data.theTransDataVol);
}



// Convert Format
namespace Converter{

    // Float Convert To Int Index SS
    vector<int> FloatArrayToRankIndex(const vector<double>& arr) {
        int n = arr.size();
        vector<pair<double,int>> tmp;
        tmp.reserve(n);
        for (int i = 0; i < n; ++i) {
            tmp.emplace_back(arr[i], i);
        }
         
        sort(tmp.begin(), tmp.end(),
             [](auto &a, auto &b){ return a.first < b.first; });
         
        vector<int> rank_idx(n);
        for (int rank = 0; rank < n; ++rank) {
            rank_idx[tmp[rank].second] = rank;
        }
        return rank_idx;
    }
    // Float Convert To Int Index MS
    vector<int> FloatToDiscreteClass(const vector<double>& values, int pCount) {
        int n = values.size();
        vector<pair<double, int>> sorted;
        for (int i = 0; i < n; ++i) {
            sorted.emplace_back(values[i], i);
        }
        sort(sorted.begin(), sorted.end());
    
        vector<int> class_index(n);
        for (int i = 0; i < n; ++i) {
            int label = (i * pCount) / n; // 均分的方式
            class_index[sorted[i].second] = label;
        }
    
        return class_index;
    }
    
}




// namespace Read_File
namespace Read_File
{   
    void locate_to_section(ifstream& infile,string& line){
        while (getline(infile, line)) {
            if (line.find("*/") != string::npos) {
                break;  
            }
        }
    }


    // Build for Section 1
    struct  Parallel_Parameters{
        unsigned int num_processor,num_tasks,num_edges;

        Parallel_Parameters() = default;
        Parallel_Parameters(unsigned int p,unsigned int t,unsigned int e)
            : num_processor(p),num_tasks(t),num_edges(e) {}
    };


    // Build for Section 1
    Parallel_Parameters parseSection_config(ifstream& infile) {
        string line;
        locate_to_section(infile,line);
        int pCount, tCount, eCount;
        infile >> pCount >> tCount >> eCount;
        infile.ignore(numeric_limits<streamsize>::max(), '\n');
        //cout << "[Section 0] Processor : " << pCount << ", Num of Tasks : " << tCount << ", Edges : " << eCount << endl;
        Read_File::Parallel_Parameters config_data(pCount,tCount,eCount);
        return config_data;
    }
    


    // Build for Section 3 , 5 , 7 Sections
    vector<vector<double>> Parse_Table(ifstream& infile,unsigned int x, unsigned int y) {
        string line;
        vector<vector<double>> theCommRate;
        locate_to_section(infile,line);
        for (int i = 0; i < y; ++i) {
            vector<double> row;
            double value;
            for (int j = 0; j < x; ++j) {
                infile >> value;
                row.push_back(value);
            }
            theCommRate.push_back(row);
        }
        infile.ignore(numeric_limits<streamsize>::max(), '\n');
        return theCommRate;
    }
    



    // Read Config of CPU 
    template<typename T>
    Config Read_Config_File(T&& filename) {
        Config Config_Info;
        ifstream infile(filename);
        string line;
        
        if (!infile) {cout << "[Error] Unable to open the file :";return Config_Info;}


        
        Read_File::Parallel_Parameters config_data;
        vector<vector<double>> theCommRate,theCompCost,theTransDataVol;


        while (getline(infile, line)) {
            if (line.find("ID==1") != string::npos)  {
                config_data = Read_File::parseSection_config(infile);

                Config_Info.thePCount = config_data.num_processor;
                Config_Info.theTCount = config_data.num_tasks;
                Config_Info.theECount = config_data.num_edges;

            } else if (line.find("ID==3") != string::npos) {
                theCommRate = Read_File::Parse_Table(
                    infile,
                    config_data.num_processor,
                    config_data.num_processor
                );
                //show_2d_vector(theCommRate);
                Config_Info.theCommRate = theCommRate;

            } else if (line.find("ID==5") != string::npos) {
                theCompCost = Read_File::Parse_Table(
                    infile,
                    config_data.num_processor,
                    config_data.num_tasks
                );
                //show_2d_vector(theCompCost);
                Config_Info.theCompCost = theCompCost;

            } else if (line.find("ID==7") != string::npos) {
                theTransDataVol = Read_File::Parse_Table(
                    infile,
                    3,
                    config_data.num_edges
                );
                //show_2d_vector(theTransDataVol);
                Config_Info.theTransDataVol = theTransDataVol;

            }
        }

        Config_Info.predMap.reserve(Config_Info.theTCount);

        for (auto &edge : Config_Info.theTransDataVol) {
            int from = static_cast<int>(edge[0]);
            int to   = static_cast<int>(edge[1]);
            double vol = edge[2];

            auto &vec = Config_Info.predMap[to];
            if (vec.empty()) {
                vec.reserve(3);
            }
            vec.emplace_back(from, vol);
        }
        

        infile.close();


        return Config_Info;
    }

    // Read Solutions
    template<typename T>
    vector<Solution>  Read_Solutions(T&& filename){
        ifstream infile(filename);
        string line;
        vector<Solution> solution_list;
        Solution solution;

        while (getline(infile, line)) {
            if (line.find("ss =") != string::npos) {
                vector<int> ss, ms;
                // Processing ss 
                line = line.substr(line.find('{') + 1);
                line = line.substr(0, line.find('}'));
                stringstream ss_stream(line);
                string num;
                while (getline(ss_stream, num, ',')) {
                    ss.push_back(stoi(num));
                }

                // Processing ms : match string
                getline(infile, line);
                line = line.substr(line.find('{') + 1);
                line = line.substr(0, line.find('}'));
                stringstream ms_stream(line);
                while (getline(ms_stream, num, ',')) {
                    ms.push_back(stoi(num));
                }

                solution.ss = ss;
                solution.ms = ms;
                solution_list.push_back(solution);
            }
            
        }

        return solution_list;
    }


    // Read Solutions of float
    template<typename T>
    vector<Solution>  Read_Float_Solutions(T&& filename){
        ifstream infile(filename);
        string line;
        vector<Solution> solution_list;
        Solution solution;

        while (getline(infile, line)) {
            if (line.find("ps =") != string::npos) {
                vector<double> ps, ms;
                vector<int> ss;
                vector<double> ps2;
                // Processing ps 
                line = line.substr(line.find('{') + 1);
                line = line.substr(0, line.find('}'));
                stringstream ss_stream(line);
                string num;
                while (getline(ss_stream, num, ',')) {
                    ps.push_back(stof(num));
                }
                // Float To Int Index 
                vector<int> ss_idx = Converter::FloatArrayToRankIndex(ps);  
                 
                

                // Processing ms : match string
                getline(infile, line);
                line = line.substr(line.find('{') + 1);
                line = line.substr(0, line.find('}'));
                stringstream ms_stream(line);
                while (getline(ms_stream, num, ',')) {
                    ms.push_back(stof(num));
                }
                // Float To Int Index                 
                vector<int> ms_idx = Converter::FloatToDiscreteClass(ms, 4);

                solution.ss = ss_idx;
                solution.ms = ms_idx;
                solution_list.push_back(solution);
            }
            
        }

        return solution_list;
    }

} // namespace Read_File

 


ScheduleResult Calculate_schedule(const vector<int>& ss,const vector<int>& ms,const Config& config) {
    int T = config.theTCount;     
    int P = config.thePCount;     
    int E = config.theECount;  

    // Tasks Start Time and End Time
    vector<double> startTime(T, 0.0), endTime(T, 0.0);
    vector<double> procFree(P, 0.0);

   
    for (int idx = 0; idx < (int)ss.size(); ++idx) {
        int t = ss[idx];         
        int p = ms[t];

        // The earliest ready time that this task can start after all predecessors are completed + communication delays are calculated
        double ready = 0.0;
        auto it = config.predMap.find(t);
        if (it != config.predMap.end()) {
            // it->second 是 vector<pair<int,double>> of (from, vol)
            for (auto &pr : it->second) {
                int from = pr.first;
                double vol = pr.second;
                int pf = ms[from];
                double commDelay = (pf != p)
                    ? vol * config.theCommRate[pf][p]
                    : 0.0;
                ready = max(ready, endTime[from] + commDelay);
            }
        }
        
        //The actual start time = max(machine idle time, ready)
        startTime[t] = max(ready, procFree[p]);
        endTime[t]   = startTime[t] + config.theCompCost[t][p];
        procFree[p]  = endTime[t];
    }
    
    // Makespan = Return the Largest Number at the List
    double makespan =  *max_element(endTime.begin(), endTime.end());
    return { startTime, endTime, makespan };
}

  


ScheduleResult Solution_Function(Solution& sol, const Config& config) {
    int T = config.theTCount;
    int P = config.thePCount;
    vector<int> task_check = sol.ss;
    sort(task_check.begin(), task_check.end());

    // 檢查任務索引是否正確
    for (int i = 0; i < T; ++i) {
        if (task_check[i] != i) {
            cerr << "[Error] Invalid task order in ss.\n";
            return { {}, {}, -1.0 };
        }
    }

    // 檢查處理器編號範圍
    if ((int)sol.ms.size() != T) {
        cerr << "[Error] ms size != number of tasks.\n";
        return { {}, {}, -1.0 };
    }
    for (int i = 0; i < T; ++i) {
        if (sol.ms[i] < 0 || sol.ms[i] >= P) {
            cerr << "[Error] ms[" << i << "] is out of processor range.\n";
            return { {}, {}, -1.0 };
        }
    }

    // 計算初步解
    ScheduleResult result = Calculate_schedule(sol.ss, sol.ms, config);
    bool adjusted = false;

    // 修正不可行解
    for (const auto& edge : config.theTransDataVol) {
        int from = (int)edge[0];
        int to = (int)edge[1];

        // 若依賴條件不滿足，進行調整
        if (result.endTime[from] > result.startTime[to]) {
            cerr << "[Adjusting] Dependency violated: Task " << from
                 << " ends at " << result.endTime[from]
                 << ", but Task " << to
                 << " starts at " << result.startTime[to] << ".\n";

            // 重新排序：將任務 to 移動到任務 from 之後
            auto it = find(sol.ss.begin(), sol.ss.end(), to);
            if (it != sol.ss.end()) {
                sol.ss.erase(it);       // 刪除原位置
            }
            sol.ss.push_back(to);       // 加入到最後

            // 更新計算結果
            result = Calculate_schedule(sol.ss, sol.ms, config);
            adjusted = true;

            cout << "[Adjusted] Moved Task " << to << " after Task " << from << ".\n";
        }
    }

    // 若有調整，顯示調整後解
    if (adjusted) {
        cout << "[Info] Adjusted solution to become feasible.\n";
        show_solution(sol);
    }

    return result;
}



int main() {
    auto start = high_resolution_clock::now();


    Config the_config_data   = Read_File::Read_Config_File("HW01-1.txt");
    auto solution_list       = Read_File::Read_Solutions("HW01-2.txt");
    auto solution_list_float = Read_File::Read_Float_Solutions("HW01-3.txt");
    auto solution_x          = Read_File::Read_Solutions("HW01-3.txt");
    /*
    cout<<"\n\n";
    show_Config(the_config_data);

    cout<<"\n\n";
    show_solution_list(solution_list);

    cout<<"\n\n";
    show_solution_list(solution_list_float);

    cout<<"\n\n";
    show_solution_list(solution_x);
    */

    
    
    // Feasible Solution
    for (size_t i = 0; i < solution_list.size(); ++i) {
        const auto& sol = solution_list[i];
        auto result = Calculate_schedule(sol.ss, sol.ms, the_config_data);
        cout << "Solution #" << i+1 << " Makespan = " << result.makespan << "\n";
        for (int t = 0; t < the_config_data.theTCount; ++t) {
            cout << "\tTask " <<setw(3)<< t
                 << " \tstart=" << result.startTime[t]
                 << ", \tend=" << result.endTime[t] << "\n";
        }
        cout<<"\n\n";
    }

    // Feasible Solution (float)
    for (size_t i = 0; i < solution_list_float.size(); ++i) {
        const auto& sol = solution_list_float[i];
        auto result = Calculate_schedule(sol.ss, sol.ms, the_config_data);
        cout << "Float-Solution #" << i+1 << " Makespan = " << result.makespan << "\n";
        for (int t = 0; t < the_config_data.theTCount; ++t) {
            cout << "\tTask " <<setw(3)<< t
                 << " \tstart=" << result.startTime[t]
                 << ", \tend=" << result.endTime[t] << "\n";
        }
    }
    

    cout<<"\n\n"; 
    // Feasible Solution (not)
    auto result = Solution_Function(solution_x[0] , the_config_data);
    cout << "-Solution # not"  << " Makespan = " << result.makespan << "\n";
    for (int t = 0; t < the_config_data.theTCount; ++t) {
        cout << "\tTask " <<setw(3)<< t
                << " \tstart=" << result.startTime[t]
                << ", \tend=" << result.endTime[t] << "\n";
    }

    cout<<"\n\n";

    cout<<"\n\n";
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Execution time: " << duration.count() << " milliseconds\n";
    cout<<"\n\n\n";system("pause");
    return 0;
}


