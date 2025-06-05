#include "include/modules.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <random>
#include <functional>

using namespace std;
using namespace os_display;
using namespace std::chrono;


struct FOX_Parameters {
    int population_size;   // 狐狸群體規模
    double alpha;          // 探索強度參數（探索幅度）
    double beta;           // 開發強度參數（局部搜尋幅度）
    double vmax;           // 速度上界（若要限制速度，否則可設為極大值）
    double tol;            // 收斂容忍度（可選，若不使用可設定為 0）

    FOX_Parameters( int pop_size , double alpha_ , double beta_ , double vmax_ , double tol_ = 0.0 )
        : population_size(pop_size), alpha(alpha_), beta(beta_), vmax(vmax_) , tol(tol_) {}
};






namespace FOX_Interface {

}








// Fox Agent
class Fox_Agent {
private:
    int id;                             // 狐狸編號（可選，用於除錯或記錄）
    int dimension;                      // 維度（與FOXAlgorithm給定維度相同）
    vector<double> position;            // 當前解向量（長度 = dimension）
    vector<double> velocity;            // 速度向量（長度 = dimension）
    double fitness;                     // 當前適應度
    mt19937* rng_ptr;                   // 指向外部亂數引擎
    vector<double> lower_bound;         // 同步目標空間上下界
    vector<double> upper_bound;         // 同步目標空間上下界

public:
    // 建構子：給定維度、上下界、亂數引擎指標，與 (可選)編號
    Fox_Agent(int id_,
             int dim,
             const vector<double>& lb,
             const vector<double>& ub,
             mt19937& rng);

    // 計算 fitness
    void evaluate(const function<double(const vector<double>&)>& fitness_func);

    
    vector<double> explore(double alpha);

    
    vector<double> exploit(const vector<double>& best_pos, double beta);

    // 更新位置與速度（若 new_fitness 更佳，則接受 new_pos）
    void update_position(const vector<double>& new_pos, double new_fitness, double vmax);

    // 取得目前資訊（存取器）
    const vector<double>& get_position() ;
    double get_fitness() ;
};
