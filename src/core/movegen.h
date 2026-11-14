// Sinh nước đi ứng viên: chỉ xét ô trống gần các quân đã có, giảm bùng nổ không gian tìm kiếm.
#pragma once
#include <vector>

#include "board.h"

namespace caro {

// Mọi ô trống (dùng cho tác nhân Random theo đúng nghĩa "nước hợp lệ ngẫu nhiên").
std::vector<Move> allEmpty(const Board& b);

// Ô trống trong bán kính `radius` (khoảng cách Chebyshev) quanh ít nhất một quân.
// Bàn trống -> chỉ ô trung tâm. Ván đã kết thúc -> rỗng. Thứ tự: tăng dần theo chỉ số ô.
std::vector<Move> candidates(const Board& b, int radius = 2);

}  // namespace caro
