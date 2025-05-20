Function SimulatedAnnealing(config):
    	// --- 參數設定 ---
    	T ← T0
    	Tmin ← 最低溫度
    	alpha ← 冷卻速率（例如 0.95）
    	iterPerTemp ← 每個溫度迭代次數
   	 maxIter ← 最大總迭代數
    	iter ← 0

    // --- 初始解 ---
    	current ← 隨機產生解()
    	result ← Solution_Function(current, config)
    	currentCost ← result.makespan
    	best ← current
    	bestCost ← currentCost

    // --- 主迴圈 ---
    	while T > Tmin and iter < maxIter:
        	for i from 1 to iterPerTemp:
            		neighbor ← current 的鄰近解
            		result ← Solution_Function(neighbor, config)
            		newCost ← result.makespan

    Δ ← newCost - currentCost

    if newCost < currentCost or random(0,1) < exp(-Δ / T):
        	 current ← neighbor
               	 currentCost ← newCost

    if newCost < bestCost:
                    best ← neighbor
                    bestCost ← newCost

    iter ← iter + 1

    T ← T * alpha

    return best

| 元件                      | 是否已實作 | 說明                                                          |
| ------------------------- | ---------- | ------------------------------------------------------------- |
| 解的定義（Solution 結構） | ✅         | 使用 `ss` + `ms` 雙向量結構                               |
| 初始解產生                | ✅         | 隨機或啟發式（LPT）                                           |
| 成本函數                  | ✅         | 使用 `Solution_Function` 包含合法性檢查與 `makespan` 評估 |
| 鄰域產生                  | ✅         | 使用交換任務順序、修改處理器、複合操作                        |
| 接受準則（exp(-Δ/T)）    | ✅         | 正確根據 SA 接受機率公式                                      |
| 降溫與終止條件            | ✅         | 包含 T\_min、迭代次數、無改善次數                             |



方法 1：Topological-sorted block 移動（保證合法）

任務之間有相依性 → predMap 構成 DAG

用合法的拓撲順序產生 ss，然後：

只允許交換「沒有依賴關係的任務」

或只做「合法的插入移動」：如將子任務延後執行