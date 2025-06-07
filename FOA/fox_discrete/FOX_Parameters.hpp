// 檔名：include/FOX_Parameters.hpp
#ifndef FOX_PARAMETERS_HPP
#define FOX_PARAMETERS_HPP

#include <random>

struct FOX_Parameters {
    std::mt19937 rng;           // 隨機引擎

    // ---- 探索模式相關 ----
    int numExplorationSwaps;    // 每次探索時對 ss 做 swap/reverse 的次數 (建議 1~3)
    int numExplorationReassign;  // 每次探索時對 ms 重新分配的次數 (建議 1~3)

    // ---- 開發模式 c1 相關 ----
    double pPerturb;            // c1 中“未對齊部分”做小範圍隨機擾動的機率 (0.1~0.2)

    // ---- 開發模式 c2 相關 ----
    double pLocalSwap;          // 做一次鄰位 swap 的機率 (0.2~0.5)
    double pLocalReverse;       // 做一次子序列 reverse 的機率 (0.2~0.5)
    double pLocalReassign;      // 做一次跨機器負載再平衡的機率 (0.2~0.5)

    // ---- 距離加權參數 ----
    double alpha;               // 排列距離權重 (e.g. 0.7)
    double beta;                // 機器指派距離權重 (alpha + beta = 1)

    // ---- 跳躍機制 ----
    int T_noImprove;            // 允許該狐狸連續多少代沒改善，才觸發跳躍 (例如 15~30)
    double pJump;               // 一旦達到 T_noImprove，以此機率進行跳躍 (0.1~0.2)

    // ---- 演算法全域參數 ----
    int MaxIt;                  // 最大迭代次數
    int P;                      // 狐狸群族大小

    FOX_Parameters()
    : rng(std::random_device{}()),
      numExplorationSwaps(2),
      numExplorationReassign(2),
      pPerturb(0.15),
      pLocalSwap(0.3),
      pLocalReverse(0.3),
      pLocalReassign(0.3),
      alpha(0.7),
      beta(0.3),
      T_noImprove(20),
      pJump(0.1),
      MaxIt(1000),
      P(30)
    {}
};

#endif // FOX_PARAMETERS_HPP
