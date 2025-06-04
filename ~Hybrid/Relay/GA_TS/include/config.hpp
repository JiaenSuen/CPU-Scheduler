#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <limits>
#include <stdexcept>

// Data Structure
struct Config {
    unsigned int thePCount = 0;
    unsigned int theTCount = 0;
    unsigned int theECount = 0;
    std::vector<std::vector<double>> theCommRate;
    std::vector<std::vector<double>> theCompCost;
    std::vector<std::vector<double>> theTransDataVol;
    // task -> list of (previous task , volume of data)
    std::unordered_map<int, std::vector<std::pair<int,double>>> predMap;
};

class Solution {
    public :
    std::vector<int> ss;  // Schedule of tasks
    std::vector<int> ms; // tasks[index] -> machine ID
    double cost;
};

struct ScheduleResult {
    std::vector<double> startTime;
    std::vector<double> endTime;
    double makespan;
};

// locate label
inline void locate_to_section(std::ifstream& infile, std::string& line) {
    while (std::getline(infile, line)) {
        if (line.find("*/") != std::string::npos) break;
    }
}

// Read Config
inline Config ReadConfigFile(const std::string& filename) {
    Config cfg;
    std::ifstream infile(filename);
    if (!infile) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string line;
    unsigned int pCount = 0, tCount = 0, eCount = 0;
    std::vector<std::vector<double>> commRate, compCost, transData;

    while (std::getline(infile, line)) {
        if (line.find("ID==1") != std::string::npos) {
            locate_to_section(infile, line);
            infile >> pCount >> tCount >> eCount;
            infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cfg.thePCount = pCount;
            cfg.theTCount = tCount;
            cfg.theECount = eCount;

        } else if (line.find("ID==3") != std::string::npos) {
            // 通訊率：PCount × PCount
            locate_to_section(infile, line);
            commRate.assign(pCount, std::vector<double>(pCount));
            for (unsigned i = 0; i < pCount; ++i) {
                for (unsigned j = 0; j < pCount; ++j) {
                    infile >> commRate[i][j];
                }
            }
            infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cfg.theCommRate = std::move(commRate);

        } else if (line.find("ID==5") != std::string::npos) {
            // 計算成本：TCount × PCount
            locate_to_section(infile, line);
            compCost.assign(tCount, std::vector<double>(pCount));
            for (unsigned i = 0; i < tCount; ++i) {
                for (unsigned j = 0; j < pCount; ++j) {
                    infile >> compCost[i][j];
                }
            }
            infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cfg.theCompCost = std::move(compCost);

        } else if (line.find("ID==7") != std::string::npos) {
            // 傳輸資料量：ECount × 3
            locate_to_section(infile, line);
            transData.assign(eCount, std::vector<double>(3));
            for (unsigned i = 0; i < eCount; ++i) {
                for (int j = 0; j < 3; ++j) {
                    infile >> transData[i][j];
                }
            }
            infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cfg.theTransDataVol = std::move(transData);
        }
    }

    // Construct predMap
    cfg.predMap.reserve(cfg.theTCount);
    for (auto &edge : cfg.theTransDataVol) {
        int from = static_cast<int>(edge[0]);
        int to   = static_cast<int>(edge[1]);
        double vol = edge[2];
        auto &vec = cfg.predMap[to];
        if (vec.empty()) vec.reserve(3);
        vec.emplace_back(from, vol);
    }

    return cfg;
}

#endif 