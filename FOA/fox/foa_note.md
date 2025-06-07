## 類別與函式總覽

1. **FOX_Parameters：儲存參數**
2. **FOX_Agent：單隻狐狸（Search Agent）**
   * initialize_position()
   * calculate_fitness()
   * calculate_distance_sound_travels() （Eq.1）
   * calculate_sp_s() （Eq.2）
   * calculate_dist_fox_prey() （Eq.3）
   * calculate_jump() （Eq.4）
   * update_position_exploitation() （Eq.5、Eq.6）
   * calculate_MinT() （Eq.7）
   * explore_position() （Eq.9）
   * clamp_position()
3. **FOX_Algorithm：演算法主體**
   1. initialize_population()
   2. evaluate_population()
   3. select_best()
   4. iterate()

| 變數名稱                                          | 物理/數學意義                                                                                            |
| ------------------------------------------------- | -------------------------------------------------------------------------------------------------------- |
| n                                                 | 總狐狸數量（Population size）                                                                            |
| Max _It                                           | 總迭代上限（Iteration 次數上限）                                                                         |
| D                                                 | 搜索空間維度（Dimension）；本例若用於排程+機器指派，則D = 2T                                            |
| Xi                                                | 第ii隻狐狸在連續空間的當前位置向量 (維度D)                                                               |
| Vi                                                | 第ii隻狐狸在連續空間的當前速度向量 (維度D)                                                               |
| Fitness_i                                         | 第ii隻狐狸當前「適應值」，也就是映射後的問題目標值（makespan）                                           |
| BestX                                             | 當前整個族群中最優的連續向量 (全域最優位置)                                                              |
| BestFitness                                       | 對應BestX\text{BestX}的最優適應值（最小化問題，就是最小 makespan）                                       |
| **r**∼**U**(**0**,**1**) | 每隻狐狸每代用以決定「利用 (Exploitation)」或「探索 (Exploration)」的隨機數；若r≥0.5r\ge 0.5則進入利用  |
| **p**∼**U**(**0**,**1**) | 僅在利用階段生效，用以區分「高機率跳 (Eq.5)」或「低機率跳 (Eq.6)」；若p>0.18p>0.18則用 Eq.5，否則用 Eq.6 |
| **Time_ST**i                                | 隨機產生的聲波往返時間，服從U(0,1)；                                                                     |
| **S**p_p                                    | 聲速 (Speed of sound)，可直接取 343 (m/s)，或 Eq.(2) 依最優位置估算                                      |
| Dist_ST_i                                         | 聲波行進距離                                                                                             |
| Dist_Fox_Prey_i                                   | 狐狸到獵物的實際距離                                                                                     |
| Jump_i                                            | 跳躍高度                                                                                                 |
| tt_i                                              | 第 ii**i** 隻狐狸所有 Time_ST (i,j) 的平均                                                         |
| MinT                                              | tt_i 中的最小值， (Eq.(7))                                                                               |
| t                                                 | 平均時間， t = tt / 2 (Eq.(7))                                                                           |
| a                                                 | 控制探索游走的線性係數， a=2×(it−1/MaxIt)\; a = 2 \times (it - 1/\text{MaxIt})(Eq.(8))                 |
| C1                                                | 當  p>0.18 時採用的跳躍方向係數，論文建議c2 = 0.18                                                      |
| C2                                                | 當  p≤0.18 時採用的跳躍方向係數，論文建議c2 = 0.82                                                     |
| rand(1,D)                                         | 長度為DD的U(0,1)U(0,1)隨機向量 (Eq.(9))                                                                  |
| BestXi(t)                                         | 當前迭代的全域最優位置 (可視為「獵物」的位置)                                                            |
| BestFitness_i                                     | 對應於BestXi的最優適應值 (makespan)                                                                      |







| Meta‐heuristic 概念                       | FOX 演算法 對應                                    | 主要公式位置    | 說明                                                                           |
| ------------------------------------------ | -------------------------------------------------- | --------------- | ------------------------------------------------------------------------------ |
| 開發階段（Exploitation / Intensification） | `update_position_exploitation(p, Dist_Fox_Prey)` | Eq.(5) / Eq.(6) | ✔ 只依靠自身感知到的距離 Dist_Fox_Prey  ✔ 若新解更差就回滾→ 強調局部精煉。  |
| 探索階段（Exploration / Diversification）  | `update_position_exploration(BestX_current, it)` | Eq.(9)          | ✔ 以全域最佳 BestX 為基底 ✔ 乘上隨機向量、MinT、參數 a → 強調區域擴散探索。 |
