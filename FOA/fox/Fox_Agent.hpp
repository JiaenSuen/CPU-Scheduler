// 檔名：include/Fox_Agent.hpp
#ifndef FOX_AGENT_HPP
#define FOX_AGENT_HPP

               
#include "include/modules.hpp"
#include "FOX_Parameters.hpp"    

#include <vector>
#include <random>
#include <algorithm>
#include <limits>

/**
 
 * 繼承欄位（Solution）：  
 *   - ss (std::vector<int>)：任務調度順序（Permutation），長度 = TCount  
 *   - ms (std::vector<int>)：任務對應機器編號，長度 = TCount  
 *   - cost (double)：該離線解的 makespan（透過 Solution_Function 計算）
    
 *   - D         : 連續空間維度， = 2 * TCount （前 TCount 維用於排序鍵、後 TCount 維用於機器指派鍵）  
 *   - X         : std::vector<double> (長度 = D)，現行「連續表示」向量  
 *   - V         : std::vector<double> (長度 = D)，對應速度向量  
 *   - Fitness   : double，目前以連續 X 映射離散解後的 makespan  
 *   - BestX     : std::vector<double> (長度 = D)，歷代「連續空間」最佳位置向量  
 *   - BestFitness: double，歷代最佳適應值 (makespan)  
 *
 
 */
class Fox_Agent : public Solution {
private:
    int id;                    // 狐狸編號 (i)
    int TCount;                // 任務數量 T
    int D;                     // 連續空間維度 D = 2 * TCount

    const Config* cfg_ptr;     // 指向問題設定 (Config)
    FOX_Parameters* pars;      // 指向演算法參數

    std::vector<double> X;     // 連續位置向量 (長度 = D)
    std::vector<double> V;     // 速度向量 (長度 = D)
    double Fitness;            // 當前適應值 (makespan)

    std::vector<double> BestX; // 歷代最佳連續向量
    double BestFitness;        // 歷代最佳適應值

    std::normal_distribution<double> norm_dist; // 常態分佈 N(0,1)

    
public:
 
    // 依論文 Eq.(2): 計算聲速 Sp_S，這裡簡化直接回傳 343
    double calculate_sp_s( ) {
        return 343.0;
    }

    // Eq.(1): Dist_ST = Sp_S * Time_ST
    double calculate_distance_ST(double Sp_S, double Time_ST) {
        return Sp_S * Time_ST;
    }

    // Eq.(3): Dist_Fox_Prey = 0.5 * Dist_ST
    double calculate_dist_fox_prey(double Dist_ST) {
        return 0.5 * Dist_ST;
    }

    // Eq.(4): Jump = 0.5 * 9.81 * t^2
    double calculate_jump(double t) {
        return 0.5 * 9.81 * t * t;
    }

    // 初始化位置：隨機為 X, V=0，並計算一次 Fitness, 更新 BestX, BestFitness, 及離散解 ss, ms, cost
    void initialize_position() {
        std::uniform_real_distribution<double> uni(0.0, 1.0);
        for (int i = 0; i < D; ++i) {
            double r = uni(pars->rng);
            X[i] = pars->LowerBound[i] + r * (pars->UpperBound[i] - pars->LowerBound[i]);
            V[i] = 0.0;
        }
        // 計算初始適應值
        calculate_fitness();
    }

public: // 計算適存值介面
    // 將目前 X 映射成離散解，計算並更新 Fitness, BestX, BestFitness, ss, ms, cost
// 類別成員初始值：


 
    double calculate_fitness(bool update = true) {
        // 1. 分段離散化
        std::vector<double> part1(X.begin(), X.begin() + TCount);
        std::vector<int> ss_int  = Converter::FloatArrayToRankIndex(part1);
        std::vector<double> part2(X.begin() + TCount, X.end());
        std::vector<int> ms_int  = Converter::FloatToDiscreteClass(part2, cfg_ptr->thePCount);

        // 2. 組成 Solution，計算 makespan
        Solution temp;
        temp.ss = ss_int;
        temp.ms = ms_int;
        ScheduleResult res = Solution_Function(temp, *cfg_ptr);

        // 3. 將「makespan 的倒數」當作適應值
        this->Fitness = 1.0 / res.makespan;

        // 4. 更新歷代最佳 (愈大愈好)
        if (update && this->Fitness > this->BestFitness) {
            this->BestFitness = this->Fitness;
            this->BestX = X;
            this->ss    = ss_int;
            this->ms    = ms_int;
            this->cost  = temp.cost; // res.makespan
        }
        return this->Fitness;
    }


public:
    Fox_Agent(int id_, int TCount_, const Config& cfg, FOX_Parameters& pars_)
    : id(id_),
      TCount(TCount_),
      D(2 * TCount_),
      cfg_ptr(&cfg),
      pars(&pars_),
      X(D, 0.0),
      V(D, 0.0),
      Fitness(0.0),              
      BestX(D, 0.0),
      BestFitness(0.0),           
      norm_dist(0.0, 1.0)
    {
        ss.resize(TCount_);
        ms.resize(TCount_);
    }


    // 開發階段更新位置 (Eq.(5)/(6))：此處先示範只更新 X, V, Fitness，不回滾
    void update_position_exploitation(double p, double Dist_Fox_Prey) {
        // 1. 計算平均時間 t
        std::uniform_real_distribution<double> uni(0.0, 1.0);
        double sum_time = 0.0;
        for (int j = 0; j < D; ++j) {
            double ts = uni(pars->rng);
            sum_time += ts;
        }
        double tt = sum_time / D;
        double t = tt / 2.0;
        // 2. 計算跳躍高度
        double Jump = calculate_jump(t);

        // 3. 暫存舊 X, V, Fitness
        std::vector<double> X_old = X;
        std::vector<double> V_old = V;
        double oldFitness = Fitness;

        std::vector<double> X_new(D), V_new(D);
        if (p > pars->p_threshold) {
            // Eq.(5): X_new = X + Dist_Fox_Prey * Jump * c1
            for (int i = 0; i < D; ++i) {
                double delta = Dist_Fox_Prey * Jump * pars->c1;
                X_new[i] = X[i] + delta;
                // clamp
                if (X_new[i] < pars->LowerBound[i]) X_new[i] = pars->LowerBound[i];
                if (X_new[i] > pars->UpperBound[i]) X_new[i] = pars->UpperBound[i];
                V_new[i] = X_new[i] - X[i];
            }
        } else {
            // Eq.(6): X_new = X + Dist_Fox_Prey * Jump * c2 + beta * N(0,1)
            for (int i = 0; i < D; ++i) {
                double delta = Dist_Fox_Prey * Jump * pars->c2;
                double noise = pars->beta * norm_dist(pars->rng);
                X_new[i] = X[i] + delta + noise;
                // clamp
                if (X_new[i] < pars->LowerBound[i]) X_new[i] = pars->LowerBound[i];
                if (X_new[i] > pars->UpperBound[i]) X_new[i] = pars->UpperBound[i];
                V_new[i] = X_new[i] - X[i];
            }
        }

        // 4. 更新 X, V
        X = X_new;
        V = V_new;

        // 5. 計算新適應值
        double newFitness = calculate_fitness();

        // 6. 如果新適應值更差 (newFitness <= oldFitness)，才回滾
        if (newFitness <= oldFitness) {
            X = X_old;
            V = V_old;
            Fitness = oldFitness;
        }
    }


    // 探索階段更新位置 (Eq.(7)~Eq.(9))
    void update_position_exploration(const std::vector<double>& BestX_current, int it) {
        std::uniform_real_distribution<double> uni(0.0, 1.0);

        // 1. 計算每維 Time_ST_j，求出 tt，並更新 MinT（但設下限 0.1）
        double sum_time = 0.0;
        for (int j = 0; j < D; ++j) {
            sum_time += uni(pars->rng);
        }
        double tt = sum_time / D;
        pars->MinT = std::max(0.1, std::min(pars->MinT, tt));  // MinT 不低於 0.1

        // 2. 計算 a = 2 * (1 - it/MaxIt)，若負則設 0
        double a = 2.0 * (1.0 - static_cast<double>(it) / pars->MaxIt);
        if (a < 0.0) a = 0.0;

        // 3. 暫存舊 X 與舊 Fitness
        std::vector<double> X_old = X;
        double oldFitness = Fitness;

        // 4. 針對每個維度，同時朝 BestX_current 靠近並加隨機擾動
        for (int i = 0; i < D; ++i) {
            double r1 = uni(pars->rng);
            double r2 = uni(pars->rng);
            // w: 保留原 x 的權重 (可自行設定，例如 0.7)
            double w = 0.7;
            // c3: 控制往 BestX_current 靠近的強度 (可自行設定，例如 1.5)
            double c3 = 1.5;

            // (a) 往 BestX_current 靠近
            double towardsBest = w * X[i] + c3 * r1 * (BestX_current[i] - X[i]);
            // (b) 隨機擾動 (規模由 MinT 與 alpha 控制)
            double randomStep = pars->alpha * r2 * pars->MinT;
            double xi = towardsBest + randomStep;

            // clamp 到上下界
            if (xi < pars->LowerBound[i]) xi = pars->LowerBound[i];
            if (xi > pars->UpperBound[i]) xi = pars->UpperBound[i];
            X[i] = xi;
        }

        // 5. 計算新適應值
        double newFitness = calculate_fitness();

        // 6. 如果新適應值更差 (newFitness <= oldFitness)，則回滾
        if (newFitness <= oldFitness) {
            X = X_old;
            Fitness = oldFitness;
        }
    }


    // Clamp 連續向量到上下界
    void clamp_continuous() {
        for (int i = 0; i < D; ++i) {
            if (X[i] < pars->LowerBound[i]) X[i] = pars->LowerBound[i];
            if (X[i] > pars->UpperBound[i]) X[i] = pars->UpperBound[i];
        }
    }

    // 存取介面
    const std::vector<double>& get_continuous_X() const { return X; }
    const std::vector<double>& get_velocity() const { return V; }
    double get_fitness() const { return Fitness; }
    const std::vector<double>& get_best_continuous_X() const { return BestX; }
    double get_best_fitness() const { return BestFitness; }
};

#endif // FOX_AGENT_HPP
