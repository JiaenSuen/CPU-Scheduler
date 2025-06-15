#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "config.hpp"

// Convert Format
namespace Converter{

    // Float Convert To Int Index SS
    std::vector<int> FloatArrayToRankIndex(const std::vector<double>& arr) {
        int n = arr.size();
        std::vector<std::pair<double,int>> tmp;
        tmp.reserve(n);
        for (int i = 0; i < n; ++i) {
            tmp.emplace_back(arr[i], i);
        }
         
        std::sort(tmp.begin(), tmp.end(),
            [](auto &a, auto &b){ return a.first < b.first; });
         
        std::vector<int> rank_idx(n);
        for (int rank = 0; rank < n; ++rank) {
            rank_idx[tmp[rank].second] = rank;
        }
        return rank_idx;
    }
    // Float Convert To Int Index MS
    std::vector<int> FloatToDiscreteClass(const std::vector<double>& values, int pCount) {
        int n = values.size();
        std::vector<std::pair<double, int>> sorted;
        for (int i = 0; i < n; ++i) {
            sorted.emplace_back(values[i], i);
        }
        sort(sorted.begin(), sorted.end());
    
        std::vector<int> class_index(n);
        for (int i = 0; i < n; ++i) {
            int label = (i * pCount) / n; // 均分的方式
            class_index[sorted[i].second] = label;
        }
    
        return class_index;
    }
    
}


namespace  os_display {

    // Display Tools
    void show_2d_vector(std::vector<std::vector<double>>& vec2d){
        for(auto& vec:vec2d){
            for (auto& data:vec)std::cout<<std::setw(4)<<std::left<<data<<" ";
            std::cout<<std::endl;
        }
        std::cout<<"\n\n";
    }

    template<typename T>
    void show_vector(std::vector<T>& vec){
        for (auto& data:vec)std::cout<<std::setw(5)<<std::left<<data<<" ";
        std::cout<<"\n";
    }


    void show_solution(Solution& solution){
        std::cout<<"ss : "; show_vector(solution.ss);
        std::cout<<"ms : "; show_vector(solution.ms);
        std::cout<<std::endl;
    }

    void show_solution_list(std::vector<Solution>& solution_list){
        for (auto& solution:solution_list) 
            show_solution(solution);
    }


    void show_Config(Config config_data){
        std::cout<<"The Num of Processor : "<<config_data.thePCount<<std::endl;
        std::cout<<"The Num of Tasks     : "<<config_data.theTCount<<std::endl;
        std::cout<<"The Num of Edges     : "<<config_data.theECount<<std::endl;
        std::cout<<"\n\n";

        std::cout<<"The Communication Rate : \n";
        show_2d_vector(config_data.theCommRate);

        std::cout<<"The Communication Cost : \n";
        show_2d_vector(config_data.theCompCost);

        std::cout<<"The Transmission Data Volume : \n";
        show_2d_vector(config_data.theTransDataVol);
    }





    

    void writeVectorToFile(const vector<double>& data, const string& filename) {
        ofstream outFile(filename);
        if (outFile.is_open()) {
            for (size_t i = 0; i < data.size(); ++i) {
                outFile << i << " " << data[i] << "\n";
            }
            outFile.close();
            cout << "Data written to " << filename << " successfully." << endl;
        } else {
            cerr << "Unable to open file: " << filename << endl;
        }
    }
    void writeTwoVectorsToFile(const vector<double>& data1, const vector<double>& data2, const string& filename) {
        ofstream outFile(filename);
        if (outFile.is_open()) {
            size_t maxSize = max(data1.size(), data2.size());
            for (size_t i = 0; i < maxSize; ++i) {
                outFile << i << " ";
                outFile << (i < data1.size() ? to_string(data1[i]) : "nan") << " ";
                outFile << (i < data2.size() ? to_string(data2[i]) : "nan") << "\n";
            }
            outFile.close();
            cout << "Data written to " << filename << " successfully." << endl;
        } else {
            cerr << "Unable to open file: " << filename << endl;
        }
    }

    void Call_Py_Visual (){
        std::string pythonCommand = "python include/visual.py";  
        int result = std::system(pythonCommand.c_str());
    }


}


#endif 