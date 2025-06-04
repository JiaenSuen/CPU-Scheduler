
以下以「繁體中文」針對您現有的模組（`Config.hpp`、`evaluation.hpp`、`modules.hpp`）做第一階段的「設計稿（Blueprint）」。此設計稿將分為三個主要類別，並說明每個類別的責任、成員欄位與方法，以及它們之間的互動。待設計稿確認無誤後，再進行逐步實作。

---

# 一、整體演算法流程概覽

FOX（Fox-inspired Optimization Algorithm）是一種以狐狸覓食、領域探索與社會搬遷行為為啟發的群體最佳化方法。在本例中，我們要用它來解「排程問題」（Schedule），已經由 `Solution` 類別承載每個解（包含 `ss`、`ms`、以及成本 `cost`），並且透過 `Solution_Function(...)` 計算適應值（makespan）。大致流程如下：

1. **讀入問題設定（Config）**

   由使用者呼叫 `ReadConfigFile(...)` 取得 `Config cfg`，裡面包含任務數、處理器數、通訊費用、計算成本、前置關係等資訊。
2. **設定 FOX 參數（FOX_Parameters）**

   定義族群大小（population）、最大迭代次數（maxIter）、探索半徑、收斂因子等。
3. **初始化 FOX 族群**

   * 使用者可呼叫現有的輔助函式 `GenerateInitialSolution(cfg, false)` 產生一個隨機 `Solution`。
   * 每個 `Fox_Agent` 建構時，呼叫 `GenerateInitialSolution(...)` 取得一個初始排程解，並用 `Solution_Function(...)` 計算適應值，紀錄為「個體最佳」（個體歷史最優）。
4. **主迴圈（Iteration Loop）**

   迭代至 `maxIter` 為止，每一代執行：

   1. **本地探索（Local Search）** ：對每隻狐狸代理人，在「鄰域」產生若干候選解（透過微小改動 `ss` 或 `ms`），若有更優則更新。
   2. **社會搬遷（Relocate / Global Move）** ：每隻狐狸參考全域最佳（Global Best）的解，向其移動或做較大幅度的隨機跳躍，以平衡探索與利用。
   3. **評估並更新** ：對每隻代理人重新計算適應值，更新個體最優；再由整個族群中挑出「全域最佳」Fox_Agent。
5. **回傳最終結果**

   主程式透過 `FOX_Algorithm fox(params, cfg)` → `auto best = fox.run()` → `best` 便是「全域最佳」代理人，其內含 `ss`、`ms` 與最小 `cost`。

接下來分別詳細說明三個類別該如何設計。

---

# 二、類別一：FOX_Parameters

 **責任** ：集中管理演算法參數，提供給 Agent 層與 Algorithm 層存取。

```cpp
// 檔名：FOX_Parameters.hpp
#ifndef FOX_PARAMETERS_HPP
#define FOX_PARAMETERS_HPP

#include <vector>
#include <random>

class FOX_Parameters {
public:
    // 建構子：帶入必要參數
    FOX_Parameters(
        unsigned int population_size_,
        unsigned int max_iterations_,
        double initial_alpha_,        // 本地探索半徑初始值
        double final_alpha_,          // 本地探索半徑最小值（可讓 alpha 線性遞減）
        double beta_,                 // 社會搬遷強度參數
        double gamma_,                // 全域隨機跳躍強度參數
        unsigned int local_neighbors_, // 本地探索時要產生的鄰居數量
        unsigned int seed_ = std::random_device{}()
    );

    // Accessors / Mutators
    unsigned int getPopulationSize() const;
    unsigned int getMaxIterations() const;

    double getAlpha(unsigned int iteration) const; // 若 alpha 隨迭代遞減
    double getBeta() const;
    double getGamma() const;
    unsigned int getLocalNeighbors() const;

    std::mt19937& getRNG();

private:
    unsigned int population_size;
    unsigned int max_iterations;

    double alpha_initial;
    double alpha_final;
    double beta;
    double gamma;
    unsigned int local_neighbors; 

    unsigned int seed;
    std::mt19937 rng; 

    // 若要實作「alpha 隨代衰減」，可在線性插值
    // alpha(t) = alpha_initial - (alpha_initial - alpha_final) * (t / max_iterations)
};

#endif
```

### 解釋

1. **population_size** ：族群中狐狸（代理人）的數量。
2. **max_iterations** ：允許的最大世代迭代次數。
3. **alpha_initial** 、 **alpha_final** ：本地探索的「最大擾動幅度」與「最小擾動幅度」，可讓 `alpha` 隨世代數線性衰減。
4. **beta** ：在「社會搬遷（Relocation）」中，用於控制向全域最佳靠近的強度。
5. **gamma** ：用於「全域隨機跳躍」的強度。
6. **local_neighbors** ：每隻狐狸在本地探索時，要生成的鄰居數量（隨機候選解）。
7. **rng** ：統一的隨機數生成器，以 `seed` 初始化。
8. **getAlpha(iteration)** ：傳入目前世代 `iteration`，回傳一個介於 `alpha_initial` 與 `alpha_final` 之間的當前 `alpha` 值（線性插值）。

* 例如：
  ```cpp
  double FOX_Parameters::getAlpha(unsigned int t) const {
      double ratio = static_cast<double>(t) / static_cast<double>(max_iterations);
      return alpha_initial - (alpha_initial - alpha_final) * ratio;
  }
  ```

---

# 三、類別二：Fox_Agent（繼承自 Solution）

 **責任** ：封裝單一隻狐狸代理人的「排程解」（`ss`、`ms`、`cost`）及 FOX 演算法中「行為」的操作（Local Search、Relocate）。同時保留「個體歷史最優」以便比較。

```cpp
// 檔名：Fox_Agent.hpp
#ifndef FOX_AGENT_HPP
#define FOX_AGENT_HPP

#include "config.hpp"
#include "evaluation.hpp"
#include "modules.hpp"      // 取得 GenerateInitialSolution 與外部 rng
#include "FOX_Parameters.hpp"
#include <vector>
#include <random>

class Fox_Agent : public Solution {
public:
    // 建構子：隨機產生初始解
    Fox_Agent(const Config& cfg, FOX_Parameters& params);

    // 複製與賦值預設即可
    Fox_Agent(const Fox_Agent& other) = default;
    Fox_Agent& operator=(const Fox_Agent& other) = default;

    // 取得本隻代理人的適應值（makespan）
    double getFitness() const;

    // 更新適應值，並同步更新 cost
    void updateFitness(const Config& cfg);

    // 本地探索：在當前解的鄰域中隨機產生若干鄰居 (local_neighbors)，若有更優則更新
    void localSearch(const Config& cfg, FOX_Parameters& params, unsigned int current_iteration);

    // 社會搬遷：向全域最佳靠攏或做全域跳躍
    void relocate(const Fox_Agent& globalBest, const Config& cfg, FOX_Parameters& params, unsigned int current_iteration);

    // 取得個體歷史最優解
    const std::vector<int>& getBestSS() const;
    const std::vector<int>& getBestMS() const;
    double getBestFitness() const;

private:
    // 個體歷史最優
    std::vector<int> best_ss;
    std::vector<int> best_ms;
    double best_fitness;

    // 擴充現有 Solution 類別所需儲存的臨時向量
    std::vector<int> temp_ss;
    std::vector<int> temp_ms;

    std::mt19937& rngRef;   // 來自 FOX_Parameters::getRNG()

    // 產生一個鄰居解：在當前 (ss, ms) 基礎上做微小隨機擾動
    void generateNeighbor(const Config& cfg, FOX_Parameters& params, 
                          std::vector<int>& out_ss, std::vector<int>& out_ms, 
                          double alpha);

    // 輔助：兩個整數向量隨機交換位置 (swap mutation)
    void swapTwoTasks(std::vector<int>& ss_vec);

    // 輔助：隨機變動某些任務的機器配置 (ms_vec)
    void mutateMachineAssignment(const Config& cfg, std::vector<int>& ms_vec, double alpha);

    // 初始化過程：隨機產生一個 valid Solution
    void initializeRandom(const Config& cfg, FOX_Parameters& params);

    // 更新「個體歷史最優」
    void updateBestHistory();

    // 檢查 candidate_ss, candidate_ms 是否比目前更優（依 Makespan 比較）
    bool isBetter(const std::vector<int>& candidate_ss, 
                  const std::vector<int>& candidate_ms,
                  const Config& cfg) const;
};

#endif
```

## Fox_Agent 成員說明

1. **欄位**
   * `best_ss`、`best_ms`：紀錄該代理人「歷史以來最好的排程順序 (ss) 與機器配置 (ms)」。
   * `best_fitness`：對應上述 `(best_ss, best_ms)` 的最低 makespan。
   * `temp_ss`、`temp_ms`：在本地探索或搬遷時，暫存候選解。
   * `rngRef`：引用同一顆隨機數引擎，來自 `FOX_Parameters::getRNG()`。
2. **建構子 `Fox_Agent(const Config& cfg, FOX_Parameters& params)`**
   * 功能：
     1. 呼叫 `initializeRandom(cfg, params)`：使用現有的 `GenerateInitialSolution(cfg,false)` 產生隨機 `Solution sol0`，並將 `sol0.ss`、`sol0.ms` 複製到 `this->ss`、`this->ms`。
     2. 透過 `Solution_Function(*this, cfg)` 計算 `this->cost`，並設 `best_ss = ss`、`best_ms = ms`、`best_fitness = cost`。
     3. 將 `rngRef = params.getRNG()`。
3. **`void updateFitness(const Config& cfg)`**
   * 功能：
     * 直接呼叫 `Solution_Function(*this, cfg)` → 取得 `ScheduleResult` → 更新 `this->cost`。
     * 若 `this->cost < best_fitness`，則更新 `best_ss = ss`、`best_ms = ms`、`best_fitness = cost`。
4. **`bool isBetter(candidate_ss, candidate_ms, cfg) const`**
   * 功能：
     * 建立一個暫時 `Solution temp; temp.ss = candidate_ss; temp.ms = candidate_ms;`。
     * 呼叫 `auto res = Solution_Function(temp, cfg)`，比較 `res.makespan` 與 `best_fitness`（或與 `this->cost`）。
     * 若更低則回傳 `true`。
5. **`void generateNeighbor(const Config& cfg, FOX_Parameters& params, out_ss, out_ms, double alpha)`**
   * 功能：
     * 先令 `out_ss = this->ss`，`out_ms = this->ms`。
     * **對 `ss` 做 swap mutation** ：以機率或次數為基準隨機交換兩個任務的位置，交換幅度可參考 `alpha`（例如，若 `alpha` 大，可允許多次交換）。
     * 比方說：根據 `alpha` 決定 swap 次數 `k = ⌈alpha * ss.size()⌉`，隨機做 `k` 次兩個 index 的交換。
     * **對 `ms` 做局部變動** ：針對任務 i，依機率 `p = alpha` 隨機將其指派到不同處理器（`rngRef() % cfg.thePCount`）。
     * 產生完後，再呼叫 `Solution_Function(temp, cfg)` 驗證可行性（雖然此問題若只是隨機指派、交換並不會違反「任務序」規則），然後回傳 `(out_ss, out_ms)`。
6. **`void localSearch(const Config& cfg, FOX_Parameters& params, unsigned int current_iteration)`**
   * 功能：
     1. 先計算當前 `alpha = params.getAlpha(current_iteration)`。
     2. 重複 `k = params.getLocalNeighbors()` 次：
        * 呼叫 `generateNeighbor(...)` 得到 `candidate_ss`、`candidate_ms`。
        * 若 `isBetter(candidate_ss, candidate_ms, cfg)`，則：
          * `this->ss = candidate_ss; this->ms = candidate_ms; this->updateFitness(cfg);`
          * `updateBestHistory()` 更新個體歷史最優，並且可以在這裡決定「是否繼續在此新位置找更好」或跳出。
     3. 完成本地探索後，最終將此輪的新 `ss, ms, cost` 與 `best_ss, best_ms, best_fitness` 同步。
7. **`void relocate(const Fox_Agent& globalBest, const Config& cfg, FOX_Parameters& params, unsigned int current_iteration)`**
   * 功能：
     1. 先計算當前 `beta = params.getBeta()`、`gamma = params.getGamma()`。
     2. 「向全域最佳靠攏」：
        * 以機率 `p = beta` 決定是否採用「向 globalBest.best_ss, best_ms 靠攏」。
        * 若要靠攏，則對 `ss`、`ms` 同步做微幅調整：
          * 例：從目前 `this->ss` 到 `globalBest.best_ss`，取兩段之間的交集（crossover-like 操作），或隨機選擇 globalBest 屬性。
          * 或者：以「partial matching crossover (PMX)」概念，把 `this->ss` 中靠近 `globalBest.best_ss` 的片段交換到 `this->ss`。
        * 對 `ms` 同理：以某個比率把 `globalBest.best_ms` 的部分指派覆蓋到 `this->ms`。
     3. 「全域隨機跳躍」：以機率 `1 - beta`，使用 `gamma` 做更大範圍的隨機擾動：
        * 例如：直接重採樣 `ms[i] = rngRef() % cfg.thePCount` 或 `swapTwoTasks(ss)` 多次。
     4. 最後 `updateFitness(cfg)` → 更新 `cost`，並 `updateBestHistory()`。
8. **`void updateBestHistory()`**
   * 如果 `this->cost < best_fitness`，則更新 `best_ss = ss; best_ms = ms; best_fitness = cost;`。

---

# 四、類別三：FOX_Algorithm

 **責任** ：統籌 FOX 演算法流程，管理全體狐狸代理人、迭代、並回傳最終全域最佳解。

```cpp
// 檔名：FOX_Algorithm.hpp
#ifndef FOX_ALGORITHM_HPP
#define FOX_ALGORITHM_HPP

#include "config.hpp"
#include "FOX_Parameters.hpp"
#include "Fox_Agent.hpp"
#include <vector>

class FOX_Algorithm {
public:
    // 建構子：傳入配置檔與 FOX 參數
    FOX_Algorithm(const Config& cfg_, FOX_Parameters& params_);

    // 執行演算法，回傳全域最佳代理人
    Fox_Agent run();

    // (選用) 取得收斂歷史
    const std::vector<double>& getConvergenceHistory() const;

private:
    const Config& cfg;                  // 問題設定
    FOX_Parameters& params;             // 參數參考
    std::vector<Fox_Agent> population;  // 族群
    Fox_Agent globalBestAgent;          // 全域最佳代理人（複製自 population）

    std::vector<double> convergenceHistory; // 紀錄每代最佳 fitness

    // 私有函式
    void initializePopulation();
    void evaluatePopulation();          // 呼叫每隻 Agent 的 updateFitness
    void updateGlobalBest();            // 找出 population 中最佳的個體
    void iterate(unsigned int iteration_index); // 單一世代

    // 輔助：記錄當前全域最佳的 cost
    void recordConvergence();
};

#endif
```

## FOX_Algorithm 成員說明

1. **欄位**
   * `const Config& cfg;`：排程問題設定，從外部傳入。
   * `FOX_Parameters& params;`：FOX 參數，從外部傳入引用（讓 `Fox_Agent` 都共用同一個 `rng`）。
   * `population`：大小為 `params.getPopulationSize()` 的 `Fox_Agent` 向量（vector）。
   * `globalBestAgent`：目前全域最佳代理人，會在每一世代結束時更新。
   * `convergenceHistory`：記錄每一代的 `globalBestAgent.getBestFitness()`，供後續分析或作圖。
2. **建構子 `FOX_Algorithm(const Config& cfg_, FOX_Parameters& params_)`**
   * 功能：
     1. 初始化 `cfg`、`params` 參考。
     2. 呼叫 `initializePopulation()`：產生 `population` 中的每一個 `Fox_Agent(cfg, params)`。
     3. 呼叫 `evaluatePopulation()`：讓所有 `Fox_Agent` 計算初始適應值（建構子裡已經做了一次，但可確保過程）。
     4. 呼叫 `updateGlobalBest()`：挑出初始全域最佳。
     5. 將初始最佳 `best_fitness` 加入 `convergenceHistory`。
3. **`void initializePopulation()`**
   * 功能：
     ```cpp
     population.clear();
     unsigned int N = params.getPopulationSize();
     for (unsigned int i = 0; i < N; ++i) {
         population.emplace_back(cfg, params);
     }
     ```
   * 每個 `Fox_Agent` 建構子內都會「隨機初始化排程」並 `updateFitness()`，然後 `best_ss`、`best_ms`、`best_fitness` 都已設定。
4. **`void evaluatePopulation()`**
   * 功能：
     ```cpp
     for (auto& agent : population) {
         agent.updateFitness(cfg);
     }
     ```
   * 確保所有代理人目前的 `ss`、`ms` 都被計算過一次適應值（makespan）。
5. **`void updateGlobalBest()`**
   * 功能：
     ```cpp
     // 先假設 population[0] 是最優
     unsigned int bestIdx = 0;
     double bestVal = population[0].getBestFitness();
     for (unsigned int i = 1; i < population.size(); ++i) {
         if (population[i].getBestFitness() < bestVal) {
             bestVal = population[i].getBestFitness();
             bestIdx = i;
         }
     }
     globalBestAgent = population[bestIdx]; 
     ```
   * 更新完後可呼叫 `recordConvergence()`，把 `bestVal` 存入 `convergenceHistory`。
6. **`void recordConvergence()`**
   * 功能：`convergenceHistory.push_back(globalBestAgent.getBestFitness());`
7. **`void iterate(unsigned int iteration_index)`**
   * 功能：

     1. **Local Search 階段** ：

     ```cpp
     for (auto& agent : population) {
         agent.localSearch(cfg, params, iteration_index);
     }
     ```

     1. **更新「暫時全域最佳」** （可選擇先跑一次，確保 Relocate 用到的是最先被 localSearch 更新過的全局最佳）：

     ```cpp
     updateGlobalBest();
     ```

     1. **Relocation 階段** ：

     ```cpp
     for (auto& agent : population) {
         agent.relocate(globalBestAgent, cfg, params, iteration_index);
     }
     ```

     1. **重新評估整個族群** ：

     ```cpp
     evaluatePopulation();
     updateGlobalBest();
     ```

     1. **紀錄收斂值** ：

     ```cpp
     recordConvergence();
     ```
8. **`Fox_Agent run()`**
   * 功能：
     ```cpp
     for (unsigned int t = 1; t <= params.getMaxIterations(); ++t) {
         iterate(t);
     }
     return globalBestAgent;
     ```
9. **`const std::vector<double>& getConvergenceHistory() const`**
   * 若要觀察每代的最佳 makespan（收斂曲線），可由外部取用。

---

# 五、設計稿總結

以下整理三個類別的關係與方法，方便快速對照：

```
┌─────────────────────────────┐
│       FOX_Parameters        │
├─────────────────────────────┤
│ - population_size           │
│ - max_iterations            │
│ - alpha_initial, alpha_final│
│ - beta, gamma               │
│ - local_neighbors           │
│ - seed                      │
│ - std::mt19937 rng          │
├─────────────────────────────┤
│ + getPopulationSize()       │
│ + getMaxIterations()        │
│ + getAlpha(iter)            │
│ + getBeta()                 │
│ + getGamma()                │
│ + getLocalNeighbors()       │
│ + getRNG()                  │
└─────────────────────────────┘
              ▲
              │  提供 rng（共用同一顆）
              │
┌────────────────────────────────────────────┐
│                Fox_Agent                  │
│      (繼承 Solution：ss, ms, cost)        │
├────────────────────────────────────────────┤
│ - best_ss, best_ms, best_fitness          │
│ - temp_ss, temp_ms                        │
│ - std::mt19937& rngRef                     │
├────────────────────────────────────────────┤
│ + Fox_Agent(cfg, params)      // 建構子  │
│ + double getFitness() const               │
│ + void updateFitness(cfg)                 │
│ + void localSearch(cfg,params,iter)       │
│ + void relocate(globalBest,cfg,params,iter) │
│ - void generateNeighbor(cfg,params,out_ss,out_ms, alpha)  │
│ - void swapTwoTasks(std::vector<int>& ss)  │
│ - void mutateMachineAssignment(cfg,std::vector<int>& ms, alpha) │
│ - void initializeRandom(cfg,params)       │
│ - void updateBestHistory()                │
│ - bool isBetter(candidate_ss, candidate_ms, cfg) const │
│ + getBestSS(),getBestMS(),getBestFitness()│
└────────────────────────────────────────────┘
              ▲                         ▲
              │                         │
              │                         │
┌────────────────────────────────────────────────────┐
│                 FOX_Algorithm                     │
├────────────────────────────────────────────────────┤
│ - const Config& cfg                                │
│ - FOX_Parameters& params                           │
│ - std::vector<Fox_Agent> population                │
│ - Fox_Agent globalBestAgent                        │
│ - std::vector<double> convergenceHistory           │
├────────────────────────────────────────────────────┤
│ + FOX_Algorithm(cfg, params)        // 建構子    │
│ + Fox_Agent run()                                │
│ + const std::vector<double>& getConvergenceHistory() │
│                                                │
│ - void initializePopulation()                    │
│ - void evaluatePopulation()                      │
│ - void updateGlobalBest()                        │
│ - void recordConvergence()                       │
│ - void iterate(unsigned int iteration_index)     │
└────────────────────────────────────────────────────┘
```

* **階層關係**
  * `FOX_Algorithm` 持有一個 `FOX_Parameters&`（所有代理人共用同一個隨機引擎）。
  * `FOX_Algorithm` 會產生多個 `Fox_Agent`，每個 `Fox_Agent` 則有自己的 `best_ss`、`best_ms`、但它們的隨機種子由同一個 `rng`（來自 `params.getRNG()`）控制，以確保可重現性。
  * `Fox_Agent` 繼承自 `Solution`（擁有 `ss`、`ms`、`cost` 欄位），並新增 FOX 特有的本地探索與搬遷行為。
* **流程互動**
  1. 在 `FOX_Algorithm` 建構子中呼叫 `initializePopulation()` → 每個 `Fox_Agent` 隨機產生一組可行解，並 `updateFitness()` → 同時 `best_ss`、`best_ms`、`best_fitness` 初始化。
  2. `FOX_Algorithm` 立即呼叫 `updateGlobalBest()` → 得到初始全域最佳。
  3. 迴圈 `for t in [1..maxIter]`：
     * `for each agent in population: agent.localSearch(...)`
     * `updateGlobalBest()`
     * `for each agent in population: agent.relocate(globalBestAgent, ...)`
     * `evaluatePopulation()` → 每個代理人更新 `cost`、並在 `updateFitness()` 中同步更新各自 `best_fitness`
     * `updateGlobalBest()` → 更新本代全域最佳
     * `recordConvergence()` → 把本代全域最佳 fitness 推入 `convergenceHistory`
* **參數演進**
  * `alpha`：隨代衰減，讓本地擾動範圍逐漸縮小；
  * `beta`：保持固定，或也可隨代微調（視論文思路）；
  * `gamma`：全域跳躍強度，可讓某些代理人在低機率下做較大擾動。

---

# 六、後續實作順序建議

1. **建立檔案**
   * `FOX_Parameters.hpp`
   * `Fox_Agent.hpp` + `Fox_Agent.cpp`
   * `FOX_Algorithm.hpp` + `FOX_Algorithm.cpp`
2. **逐步實作 `FOX_Parameters`**
   * 編寫建構子，初始化所有欄位、`rng(seed) = mt19937(seed_)`。
   * 實作各種 `getXxx()`，以及 `getAlpha(iter)` 線性插值。
3. **實作 `Fox_Agent`**
   * 在 `Fox_Agent.hpp` 中先寫好成員宣告。
   * `Fox_Agent.cpp`：
     1. `initializeRandom(cfg, params)` → `GenerateInitialSolution(cfg,false)` → 設定 `ss, ms` → `updateFitness(cfg)` → `best_ss, best_ms, best_fitness`。
     2. `updateFitness(cfg)` → 呼叫 `Solution_Function(*this, cfg)` → 更新 `this->cost` → 如果更好則更新 `best_xxx`。
     3. `isBetter(...)` → 對候選解做暫時 `Solution temp` → `Solution_Function(temp,cfg)` → 比較 `makespan`。
     4. `generateNeighbor(...)` → 依據 `alpha` 先對 `ss` 做 `swapTwoTasks(...)` 多次、再對 `ms` 做隨機指派微擾。
     5. `localSearch(...)` → 迴圈 `params.getLocalNeighbors()` 次呼叫 `generateNeighbor` → 比較是否更好 → 若更好則更新。
     6. `relocate(...)` → 根據 `beta` 做向 `globalBest` 靠攏或 `gamma` 做全域跳躍 → 更新後呼叫 `updateFitness(cfg)`。
     7. `updateBestHistory()` → 若 `this->cost < best_fitness`，則更新 `best_xxx`。
4. **實作 `FOX_Algorithm`**
   * `FOX_Algorithm.hpp` 定義好成員。
   * `FOX_Algorithm.cpp`：
     1. 建構子 → 初始化 `population` → `evaluatePopulation()` → `updateGlobalBest()` → `recordConvergence()`。
     2. `evaluatePopulation()` → `for each agent: agent.updateFitness(cfg);`
     3. `updateGlobalBest()` → 比較 `agent.getBestFitness()` 選出最小值 → `globalBestAgent = population[bestIdx]`。
     4. `recordConvergence()` → 推入 `globalBestAgent.getBestFitness()`。
     5. `iterate(t)` → `localSearch` + `updateGlobalBest()` + `relocate` + `evaluatePopulation()` + `updateGlobalBest()` + `recordConvergence()`。
     6. `run()` → 迴圈呼叫 `iterate(t)` → 回傳 `globalBestAgent`。
5. **撰寫範例程式 `main.cpp`**
   * 讀入 `Config cfg = ReadConfigFile("xxx.config");`
   * `FOX_Parameters params(popSize, maxIter, alpha0, alphaF, beta, gamma, localNeighbors, seed);`
   * `FOX_Algorithm fox(cfg, params);`
   * `auto best = fox.run();`
   * 輸出 `best.getBestSS(), best.getBestMS(), best.getBestFitness()`。
6. **測試**
   * 在已知小規模排程範例上，逐步調整 `population_size`、`max_iterations`、`alpha`、`beta`、`gamma`、`local_neighbors`，觀察收斂曲線（可印出 `convergenceHistory` 或匯出 CSV）。
   * 驗證最終 `best_ss`、`best_ms` 的可行性（透過 `Solution_Function` 的可行性檢查），並比較與其他演算法 (e.g. GA, PSO) 的結果。

---

## 參考檔案結構建議

```
project_root/
├─ include/
│   ├─ config.hpp
│   ├─ evaluation.hpp
│   ├─ modules.hpp
│   ├─ FOX_Parameters.hpp
│   ├─ Fox_Agent.hpp
│   └─ FOX_Algorithm.hpp
├─ src/
│   ├─ FOX_Parameters.cpp
│   ├─ Fox_Agent.cpp
│   ├─ FOX_Algorithm.cpp
│   └─ main.cpp
└─ data/
    └─ example.config
```

* `include/` 資料夾放所有 `.hpp` 標頭檔，與原先您提供的 `config.hpp`、`evaluation.hpp`、`modules.hpp` 放在同一層。
* `src/` 資料夾放具體的實作檔 `.cpp`（包含各類別的實作），以及一隻簡單的 `main.cpp` 做測試。

---

### 小結

1. **FOX_Parameters** ：管理所有參數與隨機數生成器。
2. **Fox_Agent** ：繼承自 `Solution`，封裝「隨機初始化」、「本地探索 (`localSearch`)」、「社會搬遷 (`relocate`)」與「個體最優歷史 (`best_ss, best_ms, best_fitness`)」。
3. **FOX_Algorithm** ：集結整個族群，落實「初始化 → localSearch → relocate → 評估 → 更新全域最佳 → 記錄收斂」之迭代流程，最終回傳 `globalBestAgent`。

待您確認此「設計稿」無誤之後，即可進入下一步「逐步實作各個 `.cpp`」階段。希望能幫助您按照既有的模組包，清楚地分工與撰寫 FOX 最佳化演算法。祝 開發順利！
