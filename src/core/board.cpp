#include "board.h"

#include <stdexcept>

namespace caro {

namespace {
// 4 trục: ngang, dọc, chéo chính (\), chéo phụ (/)
constexpr int DR[4] = {0, 1, 1, 1};
constexpr int DC[4] = {1, 0, 1, -1};

// Đếm số quân cùng màu liên tiếp từ (r,c) theo hướng (dr,dc), không tính ô (r,c).
int countDir(const std::array<Cell, CELLS>& cells, int r, int c, int dr, int dc, Cell who) {
    int n = 0;
    r += dr;
    c += dc;
    while (inBounds(r, c) && cells[makeMove(r, c)] == who) {
        ++n;
        r += dr;
        c += dc;
    }
    return n;
}
}  // namespace

Board Board::fromRows(const std::vector<std::string>& rows, Cell toMove) {
    if (rows.size() != N) throw std::invalid_argument("fromRows: cần đúng 9 dòng");
    Board b;
    for (int r = 0; r < N; ++r) {
        if (rows[r].size() != N) throw std::invalid_argument("fromRows: mỗi dòng cần 9 ký tự");
        for (int c = 0; c < N; ++c) {
            char ch = rows[r][c];
            Cell v = ch == 'X' ? Cell::X : (ch == 'O' ? Cell::O : Cell::Empty);
            if (v == Cell::Empty && ch != '.') throw std::invalid_argument("fromRows: ký tự lạ");
            b.cells_[makeMove(r, c)] = v;
            if (v != Cell::Empty) ++b.stones_;
        }
    }
    b.toMove_ = toMove;
    b.recomputeWinner();
    return b;
}

bool Board::makesFive(Move m) const {
    const Cell who = cells_[m];
    const int r = rowOf(m), c = colOf(m);
    for (int d = 0; d < 4; ++d) {
        int len = 1 + countDir(cells_, r, c, DR[d], DC[d], who) +
                  countDir(cells_, r, c, -DR[d], -DC[d], who);
        if (len >= K) return true;  // >= 5: không áp dụng luật chặn hai đầu / quá 5
    }
    return false;
}

bool Board::play(Move m) {
    if (!isLegal(m)) return false;
    cells_[m] = toMove_;
    ++stones_;
    history_.push_back(m);
    if (makesFive(m)) winner_ = toMove_;
    toMove_ = opponent(toMove_);
    return true;
}

void Board::undo() {
    if (history_.empty()) return;
    const Move m = history_.back();
    history_.pop_back();
    toMove_ = cells_[m];  // bên vừa đánh nước m sẽ có lại lượt
    cells_[m] = Cell::Empty;
    --stones_;
    // Ván chỉ có thể kết thúc ở nước cuối, nên sau khi lùi một nước thì chưa ai thắng.
    winner_ = Cell::Empty;
}

void Board::setCell(Move m, Cell c) {
    if (m < 0 || m >= CELLS) return;
    if (cells_[m] != Cell::Empty) --stones_;
    cells_[m] = c;
    if (c != Cell::Empty) ++stones_;
    history_.clear();
    recomputeWinner();
}

void Board::recomputeWinner() {
    winner_ = Cell::Empty;
    for (Move m = 0; m < CELLS; ++m) {
        if (cells_[m] != Cell::Empty && makesFive(m)) {
            winner_ = cells_[m];
            return;
        }
    }
}

std::vector<Move> Board::winningLine() const {
    std::vector<Move> line;
    if (winner_ == Cell::Empty) return line;
    for (Move m = 0; m < CELLS; ++m) {
        if (cells_[m] != winner_) continue;
        const int r = rowOf(m), c = colOf(m);
        for (int d = 0; d < 4; ++d) {
            // chỉ bắt đầu từ đầu chuỗi để không trùng lặp
            if (countDir(cells_, r, c, -DR[d], -DC[d], winner_) != 0) continue;
            int len = 1 + countDir(cells_, r, c, DR[d], DC[d], winner_);
            if (len >= K) {
                for (int i = 0; i < len; ++i) line.push_back(makeMove(r + i * DR[d], c + i * DC[d]));
                return line;
            }
        }
    }
    return line;
}

std::string Board::toString() const {
    std::string s = "  A B C D E F G H I\n";
    for (int r = 0; r < N; ++r) {
        s += char('1' + r);
        for (int c = 0; c < N; ++c) {
            s += ' ';
            s += cellChar(at(r, c));
        }
        s += '\n';
    }
    return s;
}

}  // namespace caro
