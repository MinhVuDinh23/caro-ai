// Trạng thái ván cờ Caro 9x9: ô cờ, lượt đi, lịch sử nước, người thắng.
#pragma once
#include <array>
#include <string>
#include <vector>

#include "types.h"

namespace caro {

class Board {
public:
    Board() { cells_.fill(Cell::Empty); }

    // Tạo bàn cờ từ 9 dòng ký tự 'X', 'O', '.' (dùng cho test, thế cờ thử nghiệm).
    // Người thắng được tính lại bằng cách quét toàn bàn. Lịch sử nước đi để trống.
    static Board fromRows(const std::vector<std::string>& rows, Cell toMove);

    Cell at(Move m) const { return cells_[m]; }
    Cell at(int row, int col) const { return cells_[makeMove(row, col)]; }
    Cell toMove() const { return toMove_; }
    Cell winner() const { return winner_; }
    int stoneCount() const { return stones_; }
    bool isFull() const { return stones_ == CELLS; }
    bool isDraw() const { return winner_ == Cell::Empty && isFull(); }
    // Điều kiện kết thúc: đã có người thắng hoặc bàn đầy (hòa).
    bool isTerminal() const { return winner_ != Cell::Empty || isFull(); }

    // Hợp lệ khi: trong bàn, ô trống, ván chưa kết thúc.
    bool isLegal(Move m) const {
        return m >= 0 && m < CELLS && cells_[m] == Cell::Empty && !isTerminal();
    }

    // Đánh quân của bên đang có lượt vào ô m. Trả về false nếu nước không hợp lệ.
    bool play(Move m);
    // Hoàn tác nước gần nhất (dùng trong MiniMax và nút "Đi lại").
    void undo();

    const std::vector<Move>& history() const { return history_; }
    Move lastMove() const { return history_.empty() ? NO_MOVE : history_.back(); }

    // Chế độ xếp thế cờ: đặt trực tiếp một ô, xóa lịch sử, tính lại người thắng.
    void setCell(Move m, Cell c);
    void setToMove(Cell c) { toMove_ = c; }

    // Các ô thuộc chuỗi thắng (>= 5 quân), rỗng nếu chưa ai thắng. Dùng để tô màu trên GUI.
    std::vector<Move> winningLine() const;

    std::string toString() const;  // in bàn cờ dạng text (debug, log)

private:
    // Kiểm tra nước vừa đánh tại m có tạo chuỗi >= K quân hay không (chỉ xét 4 trục qua m).
    bool makesFive(Move m) const;
    void recomputeWinner();

    std::array<Cell, CELLS> cells_;
    Cell toMove_ = Cell::X;  // X luôn đi trước
    Cell winner_ = Cell::Empty;
    int stones_ = 0;
    std::vector<Move> history_;
};

}  // namespace caro
