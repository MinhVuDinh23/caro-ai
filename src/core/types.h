// Các kiểu dữ liệu cơ bản dùng chung cho toàn bộ chương trình.
#pragma once
#include <cstdint>
#include <string>

namespace caro {

constexpr int N = 9;          // kích thước bàn cờ N x N
constexpr int K = 5;          // số quân liên tiếp để thắng
constexpr int CELLS = N * N;  // 81 ô

enum class Cell : int8_t { Empty = 0, X = 1, O = 2 };

// Nước đi = chỉ số ô: idx = row * N + col. NO_MOVE = không có nước.
using Move = int;
constexpr Move NO_MOVE = -1;

inline Cell opponent(Cell c) { return c == Cell::X ? Cell::O : Cell::X; }
inline int rowOf(Move m) { return m / N; }
inline int colOf(Move m) { return m % N; }
inline Move makeMove(int row, int col) { return row * N + col; }
inline bool inBounds(int row, int col) { return row >= 0 && row < N && col >= 0 && col < N; }

inline char cellChar(Cell c) { return c == Cell::X ? 'X' : (c == Cell::O ? 'O' : '.'); }

// Ký hiệu nước đi: cột A..I, hàng 1..9 (hàng 1 ở trên cùng). Ví dụ "E5" = ô giữa.
inline std::string moveToString(Move m) {
    if (m == NO_MOVE) return "--";
    return std::string(1, char('A' + colOf(m))) + std::to_string(rowOf(m) + 1);
}

}  // namespace caro
