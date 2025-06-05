// 檔名：include/FOX_Parameters.hpp
#ifndef FOX_PARAMETERS_HPP
#define FOX_PARAMETERS_HPP

#include <vector>
#include <random>
 

struct FOX_Parameters {
  int n;                            // 族群大小
  int MaxIt;                        // 最大迭代次數
  double alpha;                     // 探索強度 α (Eq.(4), Eq.(9) 用)
  double beta;                      // 開發強度 β (Eq.(6) 中隨機微擾)
  double c1;                        // 係數 c1 (Eq.(5) 中跳向東北)
  double c2;                        // 係數 c2 (Eq.(6) 中跳向相反方向)
  double p_threshold;               // p 隨機閾值 (x > p_threshold → Eq.(5)，否則 Eq.(6))
  double MinT;                      // 最小平均時間 MinT (Eq.(7))
  std::vector<double> LowerBound;   // 連續解空間下界 (長度 = D)
  std::vector<double> UpperBound;   // 連續解空間上界 (長度 = D)

  unsigned int seed;                // 隨機種子
  std::mt19937 rng;                 // 隨機引擎

  FOX_Parameters(int pop_size , int max_it , double alpha_ , double beta_ , double c1_ , double c2_ , double p_thresh , double MinT_ ,
                    const std::vector<double>& lb , const std::vector<double>& ub , unsigned int seed_ = std::random_device{}() )
      : n(pop_size) , MaxIt(max_it) , alpha(alpha_) , beta(beta_) , c1(c1_) , c2(c2_), p_threshold(p_thresh) , MinT(MinT_) 
        , LowerBound(lb) , UpperBound(ub) , seed(seed_) , rng(seed_) {}
};

#endif // FOX_PARAMETERS_HPP
