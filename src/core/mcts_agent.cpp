#include "mcts_agent.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <limits>

#include "evaluator.h"
#include "movegen.h"

namespace caro {

std::string MctsAgent::name() const {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "MCTS(it=%d,c=%.2f,%s%s)", cfg_.iterations, cfg_.c,
                  cfg_.rollout == RolloutPolicy::Random ? "rand" : "heur", cfg_.forcedMoves ? ",fm" : "");
    return buf;
}

void MctsAgent::setConfig(const MctsConfig& cfg) {
    if (cfg.seed != cfg_.seed) rng_.seed(cfg.seed);
    cfg_ = cfg;
}

double MctsAgent::uct(double W, int visits, int parentN, double c) {
    if (visits == 0) return std::numeric_limits<double>::infinity();  // nút chưa thăm: thăm trước
    return W / visits + c * std::sqrt(std::log(static_cast<double>(parentN)) / visits);
}

double MctsAgent::reward(Cell winner, Cell player) {
    if (winner == Cell::Empty) return 0.5;  // hòa
    return winner == player ? 1.0 : 0.0;
}

int MctsAgent::newNode(const Board& b, Move move, int parent) {
    Node n;
    n.move = move;
    n.justMoved = opponent(b.toMove());  // b đã ở trạng thái SAU khi đánh `move`
    n.parent = parent;
    n.terminal = b.isTerminal();
    if (!n.terminal) {
        if (cfg_.forcedMoves) n.untried = forcedMoves(b);      // có nước bắt buộc -> chỉ xét chúng
        if (n.untried.empty()) n.untried = candidates(b, cfg_.radius);
        std::shuffle(n.untried.begin(), n.untried.end(), rng_);
    }
    pool_.push_back(std::move(n));
    return static_cast<int>(pool_.size()) - 1;
}

// 1. SELECTION: đi xuống khi nút đã mở rộng hết và chưa kết thúc, mỗi bước chọn con có UCT lớn nhất.
int MctsAgent::select(Board& b) {
    int node = 0;
    while (!pool_[node].terminal && pool_[node].untried.empty() && !pool_[node].children.empty()) {
        const Node& p = pool_[node];
        int best = -1;
        double bestScore = -std::numeric_limits<double>::infinity();
        for (int ch : p.children) {
            const double s = uct(pool_[ch].W, pool_[ch].N, p.N, cfg_.c);
            if (s > bestScore) {
                bestScore = s;
                best = ch;
            }
        }
        node = best;
        b.play(pool_[node].move);
    }
    return node;
}

// 2. EXPANSION: lấy một nước chưa thử, đánh nước đó và thêm nút con mới vào cây.
int MctsAgent::expand(int node, Board& b) {
    if (pool_[node].terminal || pool_[node].untried.empty()) return node;  // nút lá kết thúc
    const Move m = pool_[node].untried.back();
    pool_[node].untried.pop_back();
    b.play(m);
    const int child = newNode(b, m, node);  // chú ý: newNode có thể cấp phát lại pool_
    pool_[node].children.push_back(child);
    return child;
}

Move MctsAgent::randomRolloutMove(const Board&, const std::vector<Move>& pool) {
    std::uniform_int_distribution<size_t> pick(0, pool.size() - 1);
    return pool[pick(rng_)];
}

// Rollout heuristic (Bài 2 - Hướng A):
//   (1) có nước thắng ngay -> đánh; (2) đối thủ có nước thắng ngay -> chặn;
//   (3) còn lại: chọn ngẫu nhiên có trọng số theo scoreMove (ưu tiên tạo/chặn chuỗi).
Move MctsAgent::heuristicRolloutMove(const Board& b, const std::vector<Move>& pool) {
    const Cell me = b.toMove();
    std::vector<Move> wins, threats;
    immediateWins(b, me, wins, threats);
    if (!wins.empty()) return wins.front();      // (1)
    if (!threats.empty()) return threats.front();  // (2)

    std::vector<double> weights(pool.size());
    for (size_t i = 0; i < pool.size(); ++i) weights[i] = 1.0 + scoreMove(b, pool[i], me);
    std::discrete_distribution<size_t> pick(weights.begin(), weights.end());
    return pool[pick(rng_)];
}

// 3. SIMULATION: chơi tiếp từ trạng thái b tới khi kết thúc; trả về phần thưởng cho `player`.
// Ứng viên rollout được cập nhật tăng dần (ô trống trong bán kính quanh các quân) để chạy nhanh.
double MctsAgent::simulate(Board& b, Cell player) {
    if (b.isTerminal()) return reward(b.winner(), player);

    std::vector<Move> pool = candidates(b, cfg_.radius);
    std::array<int, CELLS> pos;  // vị trí của ô trong pool, -1 nếu không có
    pos.fill(-1);
    for (size_t i = 0; i < pool.size(); ++i) pos[pool[i]] = static_cast<int>(i);

    while (!b.isTerminal()) {
        const Move m = cfg_.rollout == RolloutPolicy::Random ? randomRolloutMove(b, pool)
                                                             : heuristicRolloutMove(b, pool);
        b.play(m);
        // bỏ m khỏi pool (đổi chỗ với phần tử cuối)
        if (pos[m] >= 0) {
            const int i = pos[m];
            pos[pool.back()] = i;
            pool[i] = pool.back();
            pool.pop_back();
            pos[m] = -1;
        }
        // thêm các ô trống mới nằm trong bán kính quanh m
        const int r = rowOf(m), c = colOf(m);
        for (int dr = -cfg_.radius; dr <= cfg_.radius; ++dr)
            for (int dc = -cfg_.radius; dc <= cfg_.radius; ++dc) {
                if (!inBounds(r + dr, c + dc)) continue;
                const Move q = makeMove(r + dr, c + dc);
                if (b.at(q) == Cell::Empty && pos[q] < 0) {
                    pos[q] = static_cast<int>(pool.size());
                    pool.push_back(q);
                }
            }
        if (pool.empty() && !b.isTerminal()) {  // không còn ô gần quân (hiếm) -> xét mọi ô trống
            pool = allEmpty(b);
            for (size_t i = 0; i < pool.size(); ++i) pos[pool[i]] = static_cast<int>(i);
        }
    }
    return reward(b.winner(), player);
}

// 4. BACKPROPAGATION: r là phần thưởng cho justMoved của `node`.
// Đi lên mỗi tầng thì người đánh đổi bên -> phần thưởng đổi thành 1 - r.
void MctsAgent::backpropagate(int node, double r) {
    while (node >= 0) {
        Node& n = pool_[node];
        n.N += 1;
        n.W += r;
        r = 1.0 - r;
        node = n.parent;
    }
}

Move MctsAgent::choose(const Board& start, SearchStats& stats) {
    const auto t0 = std::chrono::steady_clock::now();
    stats = SearchStats{};
    if (start.isTerminal()) return NO_MOVE;

    pool_.clear();
    pool_.reserve(static_cast<size_t>(std::max(1, cfg_.iterations)) + 1);
    newNode(start, NO_MOVE, -1);  // gốc: justMoved = đối thủ của bên đến lượt

    const int iterations = std::max(1, cfg_.iterations);
    for (int it = 0; it < iterations; ++it) {
        Board b = start;
        int node = select(b);                                // 1
        node = expand(node, b);                              // 2
        const double r = simulate(b, pool_[node].justMoved); // 3
        backpropagate(node, r);                              // 4
    }

    // Chọn nước: N lớn nhất; bằng N thì W/N lớn hơn.
    std::vector<int> kids = pool_[0].children;
    std::sort(kids.begin(), kids.end(), [&](int a, int b) {
        const Node &na = pool_[a], &nb = pool_[b];
        if (na.N != nb.N) return na.N > nb.N;
        return na.W / std::max(1, na.N) > nb.W / std::max(1, nb.N);
    });

    stats.move = kids.empty() ? NO_MOVE : pool_[kids[0]].move;
    stats.iterations = iterations;
    stats.nodes = static_cast<long long>(pool_.size());
    for (size_t i = 0; i < kids.size() && static_cast<int>(i) < cfg_.reportTop; ++i) {
        const Node& n = pool_[kids[i]];
        stats.topChildren.push_back({n.move, n.N, n.N ? n.W / n.N : 0.0});
    }
    stats.ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    return stats.move;
}

}  // namespace caro
