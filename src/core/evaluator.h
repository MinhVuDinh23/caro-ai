// Hàm đánh giá trạng thái dựa trên "cửa sổ 5 ô".
//
// Ý tưởng: bàn 9x9 có đúng 140 đoạn thẳng dài 5 ô (ngang 45, dọc 45, hai chéo 25 + 25).
// Một đoạn chỉ chứa quân của MỘT bên (và ô trống) là một khả năng thắng còn sống của bên đó.
// Đoạn càng nhiều quân càng nguy hiểm: trọng số tăng theo cấp số ~10 lần.
// Đoạn có quân của cả hai bên đã "chết", không tính điểm.
#pragma once
#include <array>
#include <vector>

#include "board.h"

namespace caro {

using Window = std::array<Move, K>;

// Trọng số theo số quân trong một cửa sổ còn sống: 0..5 quân.
constexpr int WINDOW_WEIGHT[K + 1] = {0, 1, 10, 100, 1000, 100000};

// Điểm lớn khi bên đến lượt đã có cửa sổ 4 quân: chắc chắn thắng ở nước kế tiếp.
constexpr int THREAT_WIN_SCORE = 500000;

const std::vector<Window>& allWindows();                // 140 cửa sổ
const std::vector<int>& windowsOfCell(Move m);          // chỉ số các cửa sổ chứa ô m

// Điểm của bàn cờ theo góc nhìn `perspective`: dương = có lợi cho perspective.
// = tổng trọng số cửa sổ của mình - tổng trọng số cửa sổ của đối thủ,
//   cộng/trừ THREAT_WIN_SCORE nếu bên đến lượt có cửa sổ 4 quân (thắng ngay nước sau).
int evaluate(const Board& b, Cell perspective);

// Giá trị của việc bên `side` đánh vào ô trống m = giá trị tấn công + giá trị phòng thủ.
//   tấn công: mức tăng trọng số các cửa sổ của mình chứa m;
//   phòng thủ: mức tăng mà đối thủ sẽ có nếu họ đánh vào m (tức là giá trị chặn).
// Dùng để sắp xếp nước đi (MiniMax) và cho rollout heuristic (MCTS).
int scoreMove(const Board& b, Move m, Cell side);

}  // namespace caro
