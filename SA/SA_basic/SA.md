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
