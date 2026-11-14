// Giao diện chung cho mọi tác nhân chơi cờ (Random, MiniMax, MCTS).
#pragma once
#include <string>
#include <vector>

#include "board.h"

namespace caro {

// Thông tin một nút con của gốc MCTS (để hiển thị top nước đi).
struct ChildInfo {
    Move move = NO_MOVE;
    int visits = 0;        // N
    double meanReward = 0; // W / N
};

// Thống kê của một lần suy luận, dùng cho log/GUI/thực nghiệm.
struct SearchStats {
    Move move = NO_MOVE;
    int depth = 0;           // MiniMax: độ sâu tìm kiếm
    long long nodes = 0;     // số nút đã duyệt
    long long cutoffs = 0;   // MiniMax: số lần cắt tỉa alpha-beta
    int iterations = 0;      // MCTS: số iteration đã chạy
    double ms = 0;           // thời gian suy luận (mili giây)
    int value = 0;           // MiniMax: giá trị của nước được chọn
    std::vector<ChildInfo> topChildren;  // MCTS: các nút con nhiều lượt thăm nhất
};

class Agent {
public:
    virtual ~Agent() = default;
    virtual std::string name() const = 0;
    // Chọn nước đi cho bên b.toMove(). Bàn cờ chưa kết thúc. Phải trả về nước hợp lệ.
    virtual Move choose(const Board& b, SearchStats& stats) = 0;
};

}  // namespace caro
