    // whale.hpp
    #ifndef WHALE_HPP
    #define WHALE_HPP

    
    #include "include/modules.hpp"
    #include <algorithm>
    #include <random>

    extern std::mt19937 rng;

    class Whale : public Solution {
    public:
        // 建構子：使用 Config 指標初始化
        explicit Whale(const Config& cfg)
        : cfg_(&cfg)
        {
            Solution base = GenerateInitialSolution(*cfg_);
            ss = std::move(base.ss);
            ms = std::move(base.ms);
            evaluate();
        }

        // Encircle Best: Swap first mismatch in ss，並對兩個索引的 ms 做 bit-flip 式擾動
        Whale encircle_best(const Whale& best) const {
            Whale nxt = *this;
            int n = ss.size();
            for (int i = 0; i < n; ++i) {
                if (nxt.ss[i] != best.ss[i]) {
                    int j = std::find(nxt.ss.begin(), nxt.ss.end(), best.ss[i]) - nxt.ss.begin();
                    // swap ss
                    std::swap(nxt.ss[i], nxt.ss[j]);
                    nxt.ms[nxt.ss[i]] = rng() % cfg_->thePCount;
                    nxt.ms[nxt.ss[j]] = rng() % cfg_->thePCount;
                    break;
                }
            }
            nxt.evaluate();
            return nxt;
        }

        // Spiral Reverse: 反轉 ss 上的子段，並對該子段的 ms 進行隨機重置
        Whale spiral_reverse() const {
            Whale nxt = *this;
            int n = ss.size();
            std::uniform_int_distribution<int> dist(0, n - 1);
            int a = dist(rng), b = dist(rng);
            if (a > b) std::swap(a, b);
            std::reverse(nxt.ss.begin() + a, nxt.ss.begin() + b + 1);
            
            for (int k = a; k <= b; ++k) {
                nxt.ms[nxt.ss[k]] = rng() % cfg_->thePCount;
            }
            nxt.evaluate();
            return nxt;
        }

        // Order Crossover: 對 ss 做 OX，對 ms 在非交叉區段隨機重置
        Whale order_crossover(const Whale& other) const {
            int n = ss.size();
            std::uniform_int_distribution<int> dist(0, n - 1);
            int a = dist(rng), b = dist(rng);
            if (a > b) std::swap(a, b);

            Whale child(*this);
            child.ss.assign(n, -1);
            child.ms.assign(n, -1);

            // 複製交叉區段 ss[a..b] 及其 ms
            for (int i = a; i <= b; ++i) {
                child.ss[i] = ss[i];
                child.ms[i] = ms[ss[i]];
            }
            // 填充剩餘 ss 及 ms
            int idx = (b + 1) % n;
            for (int k = 1; k <= n; ++k) {
                int gene = other.ss[(b + k) % n];
                if (std::find(child.ss.begin(), child.ss.end(), gene) == child.ss.end()) {
                    child.ss[idx] = gene;
                    // 對應的 ms 隨機重置
                    child.ms[idx] = rng() % cfg_->thePCount;
                    idx = (idx + 1) % n;
                }
            }
            child.evaluate();
            return child;
        }

        double getCost() const { return cost; }

    private:
        const Config* cfg_;    

        void evaluate() {
            auto res = Solution_Function(*this, *cfg_);
            cost = res.makespan;
        }
    };

    #endif // WHALE_HPP





    
 
Solution Whale_Optimize(const Config& cfg, int num_whales = 50,  int max_iter   = 100) {
    // init
    std::vector<Whale> pod;
    pod.reserve(num_whales);
    for (int i = 0; i < num_whales; ++i)
        pod.emplace_back(cfg);

    
    Whale best = pod.front();
    for (auto& w : pod) if (w.getCost() < best.getCost()) best = w;

    // Iters
    for (int t = 0; t < max_iter; ++t) {
        double a = 2.0 - 2.0 * t / max_iter;
        for (int i = 0; i < num_whales; ++i) {
            double A = 2.0 * a * (rng() / double(rng.max())) - a;
            double p = rng() / double(rng.max());
            Whale candidate = pod[i];

            if (p < 0.5) {
                if (std::abs(A) < 1.0) {
                    candidate = pod[i].encircle_best(best);
                } else {
                    int j = rng() % num_whales;
                    candidate = pod[i].order_crossover(pod[j]);
                }
            } else {
                candidate = pod[i].spiral_reverse();
            }

            if (candidate.getCost() < pod[i].getCost()) pod[i] = candidate;
            if (candidate.getCost() < best.getCost())    best  = candidate;
        }
    }

    return static_cast<Solution>(best);
}
