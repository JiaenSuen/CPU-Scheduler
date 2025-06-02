以下為實作 FOX（“A Fox-inspired Optimization Algorithm”）的整體計畫與設計稿。我們將以物件導向的方式切分功能，先定義好各類別、介面與主要函式，再進行後續的程式撰寫。所有說明皆以繁體中文撰述。

---

## 一、演算法概述

FOX 演算法靈感來自狐狸在狩獵與領域搜尋時展現的探索與開發行為，核心要點包含：

1. **領域探索** （Exploration）：狐狸透過搜尋更廣範圍的獵物區域來尋找潛在解。
2. **局部開發** （Exploitation）：一旦發現較佳位置，即聚焦於該區域進行更精細搜尋。
3. **動態更新** （Dynamic Update）：狐狸群體根據環境回饋不斷更新自身位置與速度。

演算法大致流程：

1. **初始化** ：隨機產生多隻「狐狸」作為初始群體，每隻狐狸代表一個可行解。
2. **評估適應度** ：針對每隻狐狸計算其對應問題的適應值（Fitness）。
3. **狸穴更新** ：每隻狐狸根據某種移動規則（含領域探索與局部開發策略）產生新位置。
4. **適應度比較** ：若新位置適應度更佳，則狐狸移動至新位置；否則保留原位置。
5. **迭代終止** ：重覆步驟 2～4，直到達到迭代次數或滿足收斂條件。

---

## 二、主要類別與函式設計

我們將拆分以下幾個關鍵模組／類別，各自負責不同職責，最終由一個頂層的 `FOXAlgorithm` 類別協同完成整個演算法流程。

```
+----------------------------------+
|           FOXAlgorithm           |  <--- 演算法控制器
+----------------------------------+
| - population: vector<FoxAgent>   |  （狐狸群體）
| - best_agent: FoxAgent           |  （當前最優狐狸）
| - max_iterations: int            |  （最大迭代次數）
| - problem: shared_ptr<Problem>   |  （優化問題物件）
| - rng: mt19937                   |  （亂數引擎）
+----------------------------------+
| + FOXAlgorithm(problem, params)  |  （建構子，讀入問題與參數）
| + void initialize()              |  （初始化狐狸群體）
| + void evaluate_all()            |  （評估所有狐狸適應度）
| + void update_population()       |  （更新狐狸群體位置）
| + FoxAgent select_best() const   |  （選出當前最優狐狸）
| + void run()                     |  （執行演算法主迴圈）
| + vector<double> get_best_solution() |
+----------------------------------+

+----------------------------------+
|           FoxAgent               |  <--- 狐狸個體（解向量 + 狀態）
+----------------------------------+
| - position: vector<double>       |  （當前解向量）
| - fitness: double                |  （當前適應度）
| - velocity: vector<double>       |  （動量／速度向量，用於移動）
| - id: int                        |  （狐狸編號，可選）
+----------------------------------+
| + FoxAgent(dim, rng)             |  （依維度與亂數引擎隨機生成）
| + void evaluate(shared_ptr<Problem> problem) |
| + vector<double> explore()       |  （領域探索行為，回傳新位置）
| + vector<double> exploit(const FoxAgent& best) |  （局部開發行為，回傳新位置）
| + void update_position(const vector<double>& new_pos) |
+----------------------------------+

+----------------------------------+
|           Problem                |  <--- 抽象問題介面
+----------------------------------+
| - dimension: int                 |  （決策變數維度）
| - lower_bound: vector<double>    |  （各維度下界）
| - upper_bound: vector<double>    |  （各維度上界）
+----------------------------------+
| + Problem(dim, lb, ub)           |  （建構子）
| + virtual double fitness(const vector<double>& x) const = 0 |
|   （純虛擬函式，子類別實作具體目標函式）        |
| + bool in_bounds(const vector<double>& x) const      |
|   （檢查 x 是否在邊界內）                      |
+----------------------------------+

+----------------------------------+
|         BenchmarkProblem         |  <--- 範例問題類別
+----------------------------------+
|  (繼承自 Problem)                |
+----------------------------------+
| + BenchmarkProblem(dim, lb, ub)  |
| + double fitness(const vector<double>& x) const override |
|   （具體實作某常見測試函式，如 Rastrigin、Sphere 等）   |
+----------------------------------+

+----------------------------------+
|        FOXParameters             |  <--- 演算法參數容器
+----------------------------------+
| - population_size: int           |
| - alpha: double                  |  （領域探索速率參數）
| - beta: double                   |  （局部開發速率參數）
| - vmax: double                   |  （速度上限）
| - tol: double                    |  （收斂容忍度，可選） 
+----------------------------------+
| + FOXParameters(...)             |  （建構子，初始化各參數）      |
+----------------------------------+
```

### 類別職責說明

1. **FOXAlgorithm**
   * 讀取並保存我們要優化的 `Problem` 物件（例如 BenchmarkProblem）。
   * 根據 `FOXParameters` 來配置群體大小、參數等。
   * 管理狐狸群體（`population`）與全局最優解（`best_agent`）。
   * 包含主迴圈：`initialize() → evaluate_all() → iter( update_population(); evaluate_all(); 更新 best_agent ) → 結束`。
   * 提供 `get_best_solution()` 回傳最終最優解向量。
2. **FoxAgent**
   * 儲存單一狐狸個體的「位置向量」（即可行解）。
   * 儲存對應的 `fitness` 與移動時所需的 `velocity`。
   * 提供 `explore()`（領域探索）及 `exploit()`（局部開發）的行為函式，傳回新的候選位置。
   * `evaluate()`：呼叫 `Problem::fitness(...)` 來計算適應度。
   * `update_position(...)`：若新的候選位置符合條件，則更新自身位置與速度。
3. **Problem（抽象基底）**
   * 定義所有優化問題都需實作的介面：
     * 決策變數維度 `dimension`、上下界 `lower_bound` / `upper_bound`。
     * 純虛擬函式 `fitness(...)`：給定一個向量回傳對應目標函式值。
     * `in_bounds(...)`：判斷解是否在範圍內。
4. **BenchmarkProblem（範例實作）**
   * 繼承自 `Problem`，實作具體目標函式（如 Sphere、Rosenbrock、Rastrigin 等）。
   * 方便開發與測試時直接插入。
5. **FOXParameters**
   * 包含關鍵參數：
     * **population_size** （群體大小）
     * **alpha** （領域探索係數）
     * **beta** （局部開發係數）
     * **vmax** （速度限制）
     * **tol** （收斂容忍度，如最大迭代若解未大幅改變便停止）

---

## 三、函式介面詳細規劃

以下列出每個類別中最重要的公開（public）函式與預期行為，僅示意主要參數與回傳型態。

### 1. FOXAlgorithm

```cpp
class FOXAlgorithm {
private:
    vector<FoxAgent> population;        // 狐狸群體
    FoxAgent best_agent;                // 當前最佳狐狸
    shared_ptr<Problem> problem;        // 優化問題
    FOXParameters params;               // 演算法參數
    int max_iterations;                 // 最大迭代次數
    mt19937 rng;                        // 隨機數引擎

public:
    // 建構：接收問題指標、參數與最大迭代次數
    FOXAlgorithm(shared_ptr<Problem> problem_,
                 const FOXParameters& params_,
                 int max_iterations_);

    // 初始化狐狸群體位置與速度
    void initialize();

    // 對整個群體計算適應度
    void evaluate_all();

    // 選出群體中最優狐狸
    FoxAgent select_best() const;

    // 根據 FOX 規則更新所有狐狸位置
    void update_population();

    // 執行主迴圈
    void run();

    // 取得最終最佳解向量
    vector<double> get_best_solution() const;
};
```

* **initialize()**
  * 隨機產生 `params.population_size` 隻狐狸，使用 `FoxAgent(dim, rng)` 建構。
  * 設定 `best_agent` 為第一隻或適應度最低（或最高，視優化問題而定）。
* **evaluate_all()**
  * 對每隻 `population[i]` 呼叫 `population[i].evaluate(problem)`，計算並更新其 `fitness`。
  * 比較更新 `best_agent`。
* **select_best()**
  * 在 `population` 中找到適應度最優的那隻，回傳一份 copy。
* **update_population()**
  * 對於每隻 `fox`，
    1. 先呼叫 `fox.explore()` 取得一組新候選位置 `pos_explore`，若 `pos_explore` 在範圍內且適應度更佳，則更新該狐狸位置；
    2. 接著呼叫 `fox.exploit(best_agent)` 取得新候選位置 `pos_exploit`，再做同樣檢查與更新；
    3. 如有需要可合併 Exploration/Exploitation 產生最優子候選解。
  * 動態調整速度向量（若使用速度概念）。
* **run()**
  * 呼叫 `initialize()` → `evaluate_all()` → `for iter=1..max_iterations { update_population(); evaluate_all(); ...收斂或更新 best_agent }` → 結束。
* **get_best_solution()**
  * 回傳 `best_agent.position`。

### 2. FoxAgent

```cpp
class FoxAgent {
private:
    vector<double> position;    // 當前解向量
    vector<double> velocity;    // 速度 / 動量
    double fitness;             // 當前適應度
    int id;                     // (可選) 狐狸編號
    mt19937* rng_ptr;           // 指向外部亂數引擎

public:
    // 建構：給定維度 dim 與亂數引擎指標
    FoxAgent(int dim, mt19937& rng, int id_ = -1);

    // 計算並更新 fitness
    void evaluate(shared_ptr<Problem> problem);

    // 領域探索：以衝量 (alpha) 及隨機方向尋找新位置
    vector<double> explore(double alpha, const vector<double>& lb, const vector<double>& ub);

    // 局部開發：以當前最優狐狸位置 best_pos 做引導 (beta)
    vector<double> exploit(const FoxAgent& best, double beta, const vector<double>& lb, const vector<double>& ub);

    // 更新位置與速度（若通過 fitness 檢查）
    void update_position(const vector<double>& new_pos, double new_fitness);

    // 存取函式：取得 position、fitness、velocity
    const vector<double>& get_position() const;
    double get_fitness() const;
    const vector<double>& get_velocity() const;
};
```

* **建構子**
  * 隨機初始化 `position[i]` 為 [lb[i], ub[i]] 區間的均勻亂數。
  * `velocity[i]` 可設為 0 或小範圍亂數。
  * `fitness` 預設為極大/極小值待計算。
* **evaluate()**
  * 若 `position` 在範圍之外，先投影回最近邊界（呼叫 `Problem::in_bounds`）。
  * `fitness = problem->fitness(position)`，並儲存。
* **explore(alpha, lb, ub)**
  * 根據 FOX 演算法論文所定義的“領域探索”公式，例如：
    ```
    new_pos[i] = position[i] + alpha * rand_uniform(-1, 1) * (ub[i] - lb[i]);
    ```
  * 回傳 `new_pos`，但不改變 `this->position`；由呼叫端比較適應度之後再決定是否更新。
* **exploit(best, beta, lb, ub)**
  * 以全局最優狐狸位置 `best.get_position()` 為引導目標：
    ```
    new_pos[i] = position[i] + beta * rand_uniform(0, 1) * (best_pos[i] - position[i]);
    ```
  * 回傳 `new_pos`。
* **update_position(new_pos, new_fitness)**
  * 檢查 `new_pos` 是否在 `[lb, ub]` 之內。若超出則投影。
  * 與原本 `fitness` 比較，若 `new_fitness` 更佳，則更新 `position = new_pos`、`fitness = new_fitness`，並同步更新 `velocity = new_pos - old_pos`。

### 3. Problem（純抽象）

```cpp
class Problem {
protected:
    int dimension;
    vector<double> lower_bound;
    vector<double> upper_bound;

public:
    Problem(int dim, const vector<double>& lb, const vector<double>& ub);

    virtual ~Problem() = default;

    // 需由子類別實作：給定向量 x 回傳對應目標函式值
    virtual double fitness(const vector<double>& x) const = 0;

    // 檢查 x 是否在各維度上下界內
    bool in_bounds(const vector<double>& x) const {
        for (int i = 0; i < dimension; ++i) {
            if (x[i] < lower_bound[i] || x[i] > upper_bound[i]) return false;
        }
        return true;
    }

    // 若超出邊界，投影回上下界
    vector<double> project(const vector<double>& x) const {
        vector<double> y = x;
        for (int i = 0; i < dimension; ++i) {
            if (y[i] < lower_bound[i]) y[i] = lower_bound[i];
            if (y[i] > upper_bound[i]) y[i] = upper_bound[i];
        }
        return y;
    }

    int get_dimension() const { return dimension; }
    const vector<double>& get_lb() const { return lower_bound; }
    const vector<double>& get_ub() const { return upper_bound; }
};
```

### 4. BenchmarkProblem（示範）

```cpp
class SphereProblem : public Problem {
public:
    SphereProblem(int dim, const vector<double>& lb, const vector<double>& ub)
        : Problem(dim, lb, ub) {}

    // Sphere function: f(x) = sum_{i=1}^n x_i^2
    double fitness(const vector<double>& x) const override {
        double sum = 0.0;
        for (double xi : x) sum += xi * xi;
        return sum;
    }
};

// 若需要多個測試函式，可以再定義 RosenbrockProblem、RastriginProblem 等等。
```

### 5. FOXParameters

```cpp
struct FOXParameters {
    int population_size;     // 狐狸數量
    double alpha;            // 領域探索係數
    double beta;             // 局部開發係數
    double vmax;             // 速度上限（若採用速度機制）
    double tol;              // 收斂容忍度（Optional，可留待未來擴充）

    FOXParameters(int pop_size,
                  double alpha_,
                  double beta_,
                  double vmax_,
                  double tol_ = 1e-6)
        : population_size(pop_size),
          alpha(alpha_),
          beta(beta_),
          vmax(vmax_),
          tol(tol_) {}
};
```

---

## 四、演算法流程示意

以下以文字敘述 FOXAlgorithm::run() 中的主流程，方便後續程式撰寫時對照。

1. **建構與參數設定**

   ```cpp
   shared_ptr<Problem> prob = make_shared<SphereProblem>(dim, lb, ub);
   FOXParameters params(pop_size, alpha, beta, vmax);
   FOXAlgorithm fox_algo(prob, params, max_iters);
   ```
2. **初始化**

   ```cpp
   fox_algo.initialize();
   // 內部會：
   //   population.resize(pop_size);
   //   for each i: population[i] = FoxAgent(dim, rng, i);
   //   每個 FoxAgent 在 constructor 中隨機設定位置與速度
   ```
3. **首次評估**

   ```cpp
   fox_algo.evaluate_all();
   // 計算每隻狐狸的 fitness，並更新 best_agent
   ```
4. **反覆迴圈**

   ```cpp
   for (int iter = 0; iter < max_iters; ++iter) {
       fox_algo.update_population();
       fox_algo.evaluate_all();
       current_best = fox_algo.select_best();
       // 可選：記錄 convergence 資訊、印出當前 iter/best_fitness、或畫圖
       if (收斂條件達成) break;
   }
   ```

   * 在 `update_population()` 內部，對每隻狐狸 `f` 執行以下步驟：
     1. **探索 (Exploration)**
        * `new_pos1 = f.explore(params.alpha, lb, ub);`
        * 若 `problem->in_bounds(new_pos1)`，則計算 `fitness1 = problem->fitness(new_pos1)`，若 `fitness1` 更佳，則 `f.update_position(new_pos1, fitness1)`。
     2. **開發 (Exploitation)**
        * 在探索後或同時，取全局最佳狐狸 `best = select_best()`，執行

          `new_pos2 = f.exploit(best, params.beta, lb, ub);`
        * 同樣檢查並更新。
     3. **速度控制（若有）**
        * 若設置速度上限 `vmax`，則對 `f.velocity` 做裁切：
          ```
          for (i = 0..dim-1)
             if (|velocity[i]| > vmax) velocity[i] = sign(velocity[i]) * vmax;
          ```
        * 實際位置更新由 `update_position()` 負責。
5. **輸出結果**

   ```cpp
   auto best_sol = fox_algo.get_best_solution();
   // 印出或回傳 best_sol 以及對應 fitness
   ```

---

## 五、各函式與參數之間的關係

* **FOXAlgorithm.initialize()**
  * 透過 `FoxAgent(dim, rng, id)` 產生初始狐狸，內部呼叫 `FoxAgent` 建構子，隨機生成 `position`、`velocity`，並將 `fitness` 設為極大 / 極小，此時尚未評估。
* **FOXAlgorithm.evaluate_all()**
  * 逐一呼叫 `FoxAgent.evaluate(problem)`，內部執行 `problem->fitness(position)`，並更新該隻狐狸的 `fitness`。
  * 同時與 `best_agent` 做比較，若更優則更新 `best_agent`。
* **FOXAlgorithm.update_population()**
  * **取得** 全局最佳：`auto best = select_best()`。
  * 對每隻 `population[i]` 呼叫
    ```cpp
    // 領域探索
    auto pos1 = population[i].explore(params.alpha, lb, ub);
    double f1 = problem->fitness(pos1);
    if (f1 < population[i].get_fitness()) {
        population[i].update_position(pos1, f1);
    }
    // 局部開發
    auto pos2 = population[i].exploit(best, params.beta, lb, ub);
    double f2 = problem->fitness(pos2);
    if (f2 < population[i].get_fitness()) {
        population[i].update_position(pos2, f2);
    }
    ```
  * 每次 `update_position()` 都須確保新位置 `new_pos` 在邊界內，若不在則呼叫 `problem->project(new_pos)`，然後再計算適應度。
  * 更新後的 `velocity = new_pos - old_pos`。
* **FoxAgent.explore() / exploit()**
  * **explore(alpha, lb, ub)** ：
  * 以狐狸當前位置為基準，加上範圍比例的隨機跳動；`alpha` 決定跳動幅度。
  * 產生維度向量 `new_pos`，直接回傳。
  * **exploit(best, beta, lb, ub)** ：
  * 參考最優狐狸位置 `best.get_position()`，以比例 `beta` 往該方向靠近；相當於局部搜尋。
  * 回傳 `new_pos`。
* **FoxAgent.update_position(new_pos, new_fitness)**
  * **邊界檢查** ：呼叫 `problem->project(new_pos)`，將 `new_pos` 投影到上下界內。
  * **適應度更新** ：直接將 `position = new_pos`、`fitness = new_fitness`。
  * **速度更新** ：`velocity = new_pos - old_pos`，並對其施加 `vmax` 限制（若啟用速度）。

---

## 六、資料結構與成員變數

1. **FOXAlgorithm**
   * `vector<FoxAgent> population;`
     * 保存群體中每隻狐狸物件，透過 `population[i].get_position()`／`.get_fitness()` 存取。
   * `FoxAgent best_agent;`
     * 每次迭代結束或在 `evaluate_all()` 中更新為當前最小（或最大）適應度的狐狸。
   * `shared_ptr<Problem> problem;`
     * 以多型指向我們要優化的問題（可靈活替換不同問題）。
   * `FOXParameters params;`
     * 包含迭代參數（α、β、vmax、群體大小等）。
   * `mt19937 rng;`
     * 全域亂數引擎，用於 `FoxAgent` 建構子與探索、開發時的隨機數。
   * `int max_iterations;`
     * 演算法執行的最大迭代次數。
2. **FoxAgent**
   * `vector<double> position;`
     * 維度即問題 `problem->get_dimension()`。
   * `vector<double> velocity;`
     * 同樣為 `dimension` 長度，用於追蹤移動方向與大小。
   * `double fitness;`
     * 當前 `position` 對應的適應度。
   * `int id;`
     * （Optional）若想在除錯或視覺化時標記狐狸編號，可使用。
   * `mt19937* rng_ptr;`
     * 指向外部傳入的亂數引擎，以確保所有狐狸共用同一個引擎而非各自獨立。
3. **Problem / BenchmarkProblem**
   * `int dimension;`
   * `vector<double> lower_bound, upper_bound;`
   * 具體問題子類別須實作 `double fitness(const vector<double>& x) const`。
4. **FOXParameters**
   * `int population_size;`
   * `double alpha, beta, vmax, tol;`

---

## 七、整體程式架構與檔案結構建議

* `modules.hpp`
  * 使用者提供的公用模組（顯示介面、計時工具、通用函式）。
  * 我們假設其中包含如 `os_display` 的函式，方便印出收斂過程、繪製簡單曲線或統計數值。
* `FOXAlgorithm.hpp` / `FOXAlgorithm.cpp`
  * 宣告與實作 `FOXAlgorithm` 類別。
* `FoxAgent.hpp` / `FoxAgent.cpp`
  * 宣告與實作 `FoxAgent` 類別。
* `Problem.hpp`
  * 宣告 `Problem` 抽象基底類別與必要的通用函式（如 `project()`）。
* `BenchmarkProblem.hpp` / `BenchmarkProblem.cpp`
  * 實作範例問題，如 Sphere、Rastrigin 等。
* `FOXParameters.hpp`
  * 宣告 `FOXParameters` 結構。
* `main.cpp`
  * 撰寫測試程式：
    1. 讀入／設定問題維度、上下界、參數。
    2. 建立 `shared_ptr<Problem>`。
    3. 建立 `FOXAlgorithm` 物件，並呼叫 `run()`。
    4. 印出最終結果與運算時間等資訊。

---

## 八、操作流程與後續步驟

1. **確認需求**
   * 先決定要使用哪個測試問題（Sphere、Rastrigin、Rosenbrock……）。
   * 決定群體大小、α、β、vmax、迭代次數等超參數。
2. **撰寫頭文件（.hpp）**
   * 按照上述設計分別建立 `FOXAlgorithm.hpp`、`FoxAgent.hpp`、`Problem.hpp`、`FOXParameters.hpp`、`BenchmarkProblem.hpp`。
   * 在 `FOXAlgorithm.hpp` 中只放宣告，將細節實作放到對應的 `.cpp`。
3. **實作細節（.cpp）**
   * 先從最基礎的 `Problem`／`BenchmarkProblem` 下手，確保 `fitness()` 與 `project()` 能正常運作。
   * 接著實作 `FoxAgent`：包含亂數初始化、`explore()`、`exploit()`、`evaluate()`、`update_position()`，並撰寫單元測試（如給一隻狐狸手動呼叫 explore/exploit，檢查是否在上下界內）。
   * 再實作 `FOXAlgorithm`：包含 `initialize()`、`evaluate_all()`、`select_best()`、`update_population()`、`run()`。
   * 在 `update_population()` 中需特別留意「探索→評估→更新」與「開發→評估→更新」的順序與邏輯。
4. **整合測試**
   * 撰寫 `main.cpp`，載入某個 `BenchmarkProblem`，設定參數後執行 `FOXAlgorithm::run()`。
   * 打印每次迭代的最優適應度，以確保演算法在收斂。
   * 嘗試不同參數、不同問題，若收斂效果不佳則調整 α / β 等參數。
5. **可視化與收斂分析**
   * 利用 `os_display`（假設 modules.hpp 提供）印出收斂曲線或詳細 log。
   * 將每代最佳適應度存成 `vector<double>`，並在結束後繪製收斂趨勢圖。
6. **擴充功能（選做）**
   * 支援「收斂容忍度」：若多代最佳適應度未改善超過一定門檻，則提前跳出。
   * 增加更多探索／開發策略變形，例如動態調整 α、β。
   * 加入並行化特性（若未來需要加速）。

---

## 九、小結

* 我們先從「類別與函式規劃」著手，不急著寫程式碼。
* 核心物件：`FOXAlgorithm`、`FoxAgent`、`Problem` 及其子類別 `BenchmarkProblem`，再加上參數容器 `FOXParameters`。
* 每個類別負責單一職責，各司其職：FOXAlgorithm 管理演算法流程，FoxAgent 管理個體行為，Problem 管理問題定義。
* 下一步就是依此草稿分別撰寫各 `.hpp/.cpp`，然後編寫 `main.cpp` 進行整合測試。

待此設計稿無誤，接下來即可開始撰寫實際程式碼。祝實作順利！
