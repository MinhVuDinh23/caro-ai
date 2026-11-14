// Chạy một ván máy-đấu-máy (dùng chung cho benchmark và chế độ Máy-Máy).
#pragma once
#include <functional>

#include "agent.h"
#include "board.h"

namespace caro {

struct GameResult {
    Cell winner = Cell::Empty;  // Empty = hòa
    int plies = 0;              // tổng số nước đã đánh
    double totalMs[2] = {0, 0}; // tổng thời gian suy luận: [0] = X, [1] = O
    int moves[2] = {0, 0};      // số nước mỗi bên đã đánh
    double avgMs(Cell side) const {
        const int i = side == Cell::X ? 0 : 1;
        return moves[i] ? totalMs[i] / moves[i] : 0.0;
    }
};

using MoveCallback = std::function<void(const Board&, Move, const SearchStats&)>;

// X luôn đi trước. Muốn hoán đổi người đi trước thì đổi vai trò agent X/O.
// Thời gian được đo ở đây (đồng hồ steady_clock) để mọi tác nhân được đo như nhau.
// Ném std::logic_error nếu tác nhân trả về nước không hợp lệ.
GameResult playGame(Agent& xAgent, Agent& oAgent, Board start = Board(),
                    const MoveCallback& onMove = {});

}  // namespace caro
