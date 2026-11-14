// Tác nhân Monte Carlo Tree Search với chỉ số UCT.
//
// Mỗi iteration gồm 4 giai đoạn (mỗi giai đoạn là một hàm riêng):
//   1. Selection     : từ gốc đi xuống theo UCT lớn nhất tới nút còn nước chưa mở rộng / nút lá.
//   2. Expansion     : tạo MỘT nút con mới từ danh sách nước chưa thử.
//   3. Simulation    : chơi tiếp tới hết ván (ngẫu nhiên hoặc heuristic) -> phần thưởng.
//   4. Backpropagation: cộng N, W ngược lên gốc, đổi góc nhìn ở mỗi tầng.
//
// Quy ước phần thưởng: thắng = 1, hòa = 0.5, thua = 0.
// Góc nhìn: W của một nút được tính cho NGƯỜI VỪA ĐÁNH nước dẫn tới nút đó (justMoved).
//   Ở nút cha, bên đến lượt chính là justMoved của các nút con -> chỉ cần chọn W/N lớn nhất.
// Chọn nước cuối cùng: nút con có số lượt thăm N lớn nhất (hòa thì xét W/N).
#pragma once
#include <cstdint>
#include <random>
#include <vector>

#include "agent.h"

namespace caro {

enum class RolloutPolicy { Random, Heuristic };

struct MctsConfig {
    int iterations = 500;   // số iteration (số lần mô phỏng)
    double c = 1.4;         // hằng số khám phá trong UCT (> 0)
    int radius = 2;         // bán kính sinh nước ứng viên (cây và rollout)
    RolloutPolicy rollout = RolloutPolicy::Random;
    // Nước bắt buộc trong cây (decisive/anti-decisive moves, Teytaud & Teytaud 2010):
    // ở mỗi nút, nếu bên đến lượt thắng ngay được -> chỉ mở rộng nước thắng; nếu đối thủ
    // có 4 quân sắp thắng -> chỉ mở rộng ô chặn. Tắt = MCTS thuần (cấu hình chính khi thực nghiệm).
    bool forcedMoves = false;
    uint32_t seed = 0;      // hạt giống ngẫu nhiên (tái lập thực nghiệm)
    int reportTop = 5;      // số nút con của gốc ghi vào SearchStats::topChildren
};

class MctsAgent : public Agent {
public:
    explicit MctsAgent(const MctsConfig& cfg = {}) : cfg_(cfg), rng_(cfg.seed) {}
    std::string name() const override;
    Move choose(const Board& b, SearchStats& stats) override;

    const MctsConfig& config() const { return cfg_; }
    void setConfig(const MctsConfig& cfg);

    // UCT(i) = W_i/N_i + c * sqrt(ln(N_p) / N_i).  N_i = 0 -> +vô cùng (ưu tiên thăm trước,
    // không bao giờ chia cho 0).
    static double uct(double W, int visits, int parentN, double c);
    // Phần thưởng của `player` khi ván kết thúc với người thắng `winner` (Empty = hòa).
    static double reward(Cell winner, Cell player);

private:
    struct Node {
        Move move = NO_MOVE;          // nước dẫn từ cha tới nút này
        Cell justMoved = Cell::Empty; // người vừa đánh `move`
        int parent = -1;
        std::vector<int> children;    // chỉ số trong pool_
        std::vector<Move> untried;    // nước chưa mở rộng (đã xáo trộn)
        bool terminal = false;
        int N = 0;                    // số lượt thăm
        double W = 0;                 // tổng phần thưởng (góc nhìn justMoved)
    };

    int newNode(const Board& b, Move move, int parent);
    int select(Board& b);                 // 1. Selection
    int expand(int node, Board& b);       // 2. Expansion
    double simulate(Board& b, Cell player);  // 3. Simulation -> phần thưởng cho `player`
    void backpropagate(int node, double r);  // 4. Backpropagation

    Move randomRolloutMove(const Board& b, const std::vector<Move>& pool);
    Move heuristicRolloutMove(const Board& b, const std::vector<Move>& pool);

    MctsConfig cfg_;
    std::mt19937 rng_;
    std::vector<Node> pool_;  // cây lưu dạng mảng, nút 0 là gốc
};

}  // namespace caro
