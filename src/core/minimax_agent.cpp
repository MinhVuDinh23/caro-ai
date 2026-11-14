#include "minimax_agent.h"

#include <algorithm>
#include <chrono>

#include "evaluator.h"
#include "movegen.h"

namespace caro {

std::string MinimaxAgent::name() const {
    std::string s = "MiniMax(d=" + std::to_string(cfg_.depth);
    s += cfg_.alphaBeta ? ",AB" : ",noAB";
    if (cfg_.topK > 0) s += ",k=" + std::to_string(cfg_.topK);
    return s + ")";
}

std::vector<Move> MinimaxAgent::orderedMoves(const Board& b) const {
    std::vector<Move> moves = candidates(b, cfg_.radius);
    const Cell side = b.toMove();
    std::vector<std::pair<int, Move>> scored;
    scored.reserve(moves.size());
    for (Move m : moves) scored.push_back({scoreMove(b, m, side), m});
    // Sắp xếp ổn định: điểm cao trước; cùng điểm giữ thứ tự chỉ số ô -> kết quả tất định.
    std::stable_sort(scored.begin(), scored.end(),
                     [](const auto& a, const auto& c) { return a.first > c.first; });
    if (cfg_.topK > 0 && static_cast<int>(scored.size()) > cfg_.topK) scored.resize(cfg_.topK);
    moves.clear();
    for (const auto& p : scored) moves.push_back(p.second);
    return moves;
}

// Trả về giá trị của trạng thái b theo góc nhìn me_ (bên MAX).
int MinimaxAgent::search(Board& b, int depth, int alpha, int beta, bool maximizing) {
    ++nodes_;

    // ---- Điều kiện kết thúc ----
    // Thắng sớm tốt hơn thắng muộn (+depth), thua muộn đỡ tệ hơn thua sớm (-depth).
    if (b.winner() == me_) return WIN_SCORE + depth;
    if (b.winner() == opponent(me_)) return -WIN_SCORE - depth;
    if (b.isFull()) return 0;                     // hòa
    if (depth == 0) return evaluate(b, me_);      // hết độ sâu -> hàm đánh giá

    const std::vector<Move> moves = orderedMoves(b);

    if (maximizing) {  // ---- Nút MAX ----
        int best = -INF;
        for (Move m : moves) {
            b.play(m);
            const int v = search(b, depth - 1, alpha, beta, false);
            b.undo();
            best = std::max(best, v);
            if (cfg_.alphaBeta) {
                alpha = std::max(alpha, best);
                if (alpha >= beta) {  // MIN ở trên sẽ không bao giờ chọn nhánh này
                    ++cutoffs_;
                    break;
                }
            }
        }
        return best;
    } else {  // ---- Nút MIN ----
        int best = INF;
        for (Move m : moves) {
            b.play(m);
            const int v = search(b, depth - 1, alpha, beta, true);
            b.undo();
            best = std::min(best, v);
            if (cfg_.alphaBeta) {
                beta = std::min(beta, best);
                if (alpha >= beta) {  // MAX ở trên đã có lựa chọn tốt hơn
                    ++cutoffs_;
                    break;
                }
            }
        }
        return best;
    }
}

Move MinimaxAgent::choose(const Board& start, SearchStats& stats) {
    const auto t0 = std::chrono::steady_clock::now();
    stats = SearchStats{};
    if (start.isTerminal()) return NO_MOVE;

    Board b = start;  // bản sao để play/undo
    me_ = b.toMove();
    nodes_ = 1;  // nút gốc
    cutoffs_ = 0;
    const int depth = std::max(1, cfg_.depth);

    // Gốc là nút MAX: thử từng nước, giữ nước có giá trị lớn nhất (bằng nhau: giữ nước đầu tiên).
    Move bestMove = NO_MOVE;
    int bestVal = -INF;
    int alpha = -INF;
    for (Move m : orderedMoves(b)) {
        b.play(m);
        const int v = search(b, depth - 1, alpha, INF, false);
        b.undo();
        if (v > bestVal) {
            bestVal = v;
            bestMove = m;
        }
        if (cfg_.alphaBeta) alpha = std::max(alpha, bestVal);
    }

    stats.move = bestMove;
    stats.depth = depth;
    stats.nodes = nodes_;
    stats.cutoffs = cutoffs_;
    stats.value = bestVal;
    stats.ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    return bestMove;
}

}  // namespace caro
