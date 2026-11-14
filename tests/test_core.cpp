// Test tầng core: luật chơi, sinh nước, hàm đánh giá, tác nhân Random, chạy ván.
#include <algorithm>
#include <chrono>
#include <set>
#include <stdexcept>

#include "board.h"
#include "evaluator.h"
#include "game_runner.h"
#include "movegen.h"
#include "random_agent.h"
#include "test_util.h"

using namespace caro;

namespace {
// Đánh lần lượt các ô (X, O, X, ...) theo danh sách "E5", "A1"...
Move sq(const char* s) { return makeMove(s[1] - '1', s[0] - 'A'); }

Board playSeq(std::initializer_list<const char*> moves) {
    Board b;
    for (const char* s : moves) b.play(sq(s));
    return b;
}

// Mẫu bàn đầy không có 5 quân liên tiếp (dùng để test hòa).
std::vector<std::string> drawPattern() {
    std::vector<std::string> rows(N, std::string(N, '.'));
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c) rows[r][c] = ((c / 2) + r) % 2 == 0 ? 'X' : 'O';
    return rows;
}

// Kiểm tra lại người thắng bằng cách quét thô (độc lập với Board::makesFive).
Cell bruteWinner(const Board& b) {
    const int DR[4] = {0, 1, 1, 1}, DC[4] = {1, 0, 1, -1};
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            for (int d = 0; d < 4; ++d) {
                const Cell who = b.at(r, c);
                if (who == Cell::Empty) continue;
                int k = 0;
                while (k < K && inBounds(r + k * DR[d], c + k * DC[d]) &&
                       b.at(r + k * DR[d], c + k * DC[d]) == who)
                    ++k;
                if (k == K) return who;
            }
    return Cell::Empty;
}
}  // namespace

// ---------------- Luật chơi ----------------

TEST_CASE("Bàn mới: trống, X đi trước, chưa kết thúc") {
    Board b;
    CHECK(b.toMove() == Cell::X);
    CHECK(b.stoneCount() == 0);
    CHECK(!b.isTerminal());
    CHECK(b.lastMove() == NO_MOVE);
    CHECK(moveToString(makeMove(4, 4)) == "E5");
    CHECK(moveToString(makeMove(8, 0)) == "A9");
}

TEST_CASE("Không cho đánh lên ô đã có quân / ngoài bàn") {
    Board b;
    CHECK(b.play(sq("E5")));
    CHECK(!b.play(sq("E5")));  // ô đã có quân
    CHECK(b.toMove() == Cell::O);  // lượt không đổi khi nước sai
    CHECK(!b.play(-1));
    CHECK(!b.play(CELLS));
    CHECK(b.stoneCount() == 1);
}

TEST_CASE("Thắng hàng ngang (giữa bàn và sát mép phải)") {
    // X: A1 B1 C1 D1 E1 ; O: A2 B2 C2 D2
    Board b = playSeq({"A1", "A2", "B1", "B2", "C1", "C2", "D1", "D2"});
    CHECK(b.winner() == Cell::Empty);
    CHECK(b.play(sq("E1")));
    CHECK(b.winner() == Cell::X);
    CHECK(b.isTerminal());
    CHECK(!b.isLegal(sq("I9")));  // ván đã kết thúc -> không được đánh tiếp

    Board e = playSeq({"E9", "A1", "F9", "A2", "G9", "A3", "H9", "A5", "I9"});  // X: E9..I9 sát mép
    CHECK(e.winner() == Cell::X);
}

TEST_CASE("Thắng hàng dọc (O thắng, sát mép dưới)") {
    // X: A1 B1 C1 E1 G1 (không liền 5) ; O: I5 I6 I7 I8 rồi I9 -> 5 quân dọc cột I
    Board b = playSeq({"A1", "I5", "B1", "I6", "C1", "I7", "E1", "I8", "G1"});
    CHECK(b.winner() == Cell::Empty);
    CHECK(b.play(sq("I9")));
    CHECK(b.winner() == Cell::O);
    CHECK(bruteWinner(b) == Cell::O);
    CHECK(b.winningLine().size() == 5);
}

TEST_CASE("Thắng hai đường chéo") {
    Board d1 = playSeq({"A1", "A9", "B2", "B9", "C3", "C9", "D4", "D9", "E5"});  // chéo chính
    CHECK(d1.winner() == Cell::X);
    Board d2 = playSeq({"I1", "A1", "H2", "B1", "G3", "C1", "F4", "D2", "E5"});  // chéo phụ
    CHECK(d2.winner() == Cell::X);
    Board d3 = playSeq({"E1", "A1", "F2", "A3", "G3", "A5", "H4", "A7", "I5"});  // chéo ngắn sát góc
    CHECK(d3.winner() == Cell::X);
}

TEST_CASE("6 quân liên tiếp vẫn thắng; 4 quân thì chưa") {
    // X: A1 B1 C1 E1 F1 (thiếu D1) rồi đánh D1 -> chuỗi 6
    Board b = playSeq({"A1", "A3", "B1", "B3", "C1", "C3", "E1", "E3", "F1", "G3"});
    CHECK(b.winner() == Cell::Empty);
    CHECK(b.play(sq("D1")));
    CHECK(b.winner() == Cell::X);
    CHECK(b.winningLine().size() == 6);

    Board four = playSeq({"A1", "A3", "B1", "B3", "C1", "C3", "D1"});
    CHECK(four.winner() == Cell::Empty);
}

TEST_CASE("Chuỗi bị chặn hai đầu vẫn thắng (không áp dụng luật chặn hai đầu)") {
    // O chặn A1 và G1, X đánh B1..F1
    Board b = playSeq({"B1", "A1", "C1", "G1", "D1", "A5", "E1", "A7", "F1"});
    CHECK(b.winner() == Cell::X);
}

TEST_CASE("Hòa khi bàn đầy không ai thắng") {
    std::vector<std::string> rows = drawPattern();
    Board full = Board::fromRows(rows, Cell::X);
    CHECK(full.isFull());
    CHECK(full.winner() == Cell::Empty);
    CHECK(bruteWinner(full) == Cell::Empty);
    CHECK(full.isDraw());

    // Bỏ trống ô cuối rồi đánh nước cuối cùng -> hòa qua play()
    const char last = rows[8][8];
    rows[8][8] = '.';
    Board b = Board::fromRows(rows, last == 'X' ? Cell::X : Cell::O);
    CHECK(!b.isTerminal());
    CHECK(b.play(makeMove(8, 8)));
    CHECK(b.isDraw());
    CHECK(b.isTerminal());
    CHECK(candidates(b).empty());
    CHECK(allEmpty(b).empty());
}

TEST_CASE("undo khôi phục đầy đủ trạng thái") {
    Board b = playSeq({"A1", "A2", "B1", "B2", "C1", "C2", "D1", "D2"});
    const Board before = b;
    CHECK(b.play(sq("E1")));
    CHECK(b.winner() == Cell::X);
    b.undo();
    CHECK(b.winner() == Cell::Empty);
    CHECK(b.toMove() == Cell::X);
    CHECK(b.stoneCount() == before.stoneCount());
    CHECK(b.history() == before.history());
    for (Move m = 0; m < CELLS; ++m) CHECK(b.at(m) == before.at(m));
    Board empty;
    empty.undo();  // không có nước để lùi -> không lỗi
    CHECK(empty.stoneCount() == 0);
}

TEST_CASE("fromRows / setCell tính lại người thắng") {
    Board b = Board::fromRows({".........", ".OOOOO...", ".........", ".........", ".........",
                               ".........", ".........", ".........", "XXXX....."},
                              Cell::X);
    CHECK(b.winner() == Cell::O);
    b.setCell(makeMove(1, 3), Cell::Empty);
    CHECK(b.winner() == Cell::Empty);
    CHECK(b.stoneCount() == 8);
    bool threw = false;
    try {
        Board::fromRows({"..."}, Cell::X);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    CHECK(threw);
}

// ---------------- Sinh nước ----------------

TEST_CASE("candidates: bàn trống -> ô giữa; chỉ trả ô trống trong bán kính") {
    Board b;
    std::vector<Move> c0 = candidates(b);
    CHECK(c0.size() == 1 && c0[0] == makeMove(4, 4));

    b.play(sq("A1"));  // góc
    std::vector<Move> c2 = candidates(b, 2);
    CHECK(c2.size() == 8);  // ô 3x3 góc trừ chính A1
    std::vector<Move> c1 = candidates(b, 1);
    CHECK(c1.size() == 3);
    for (Move m : c2) {
        CHECK(b.at(m) == Cell::Empty);
        CHECK(std::max(std::abs(rowOf(m)), std::abs(colOf(m))) <= 2);
    }
    CHECK(allEmpty(b).size() == 80);
}

// ---------------- Hàm đánh giá ----------------

TEST_CASE("Có đúng 140 cửa sổ 5 ô; mỗi ô thuộc 1..20 cửa sổ") {
    CHECK(allWindows().size() == 140);
    CHECK(windowsOfCell(makeMove(4, 4)).size() == 20);  // giữa bàn: 5 vị trí x 4 trục
    CHECK(windowsOfCell(makeMove(0, 0)).size() == 3);   // góc: ngang, dọc, chéo chính
}

TEST_CASE("evaluate: đối xứng góc nhìn và đúng dấu") {
    Board empty;
    CHECK(evaluate(empty, Cell::X) == 0);
    // X có 3 quân liên tiếp, O rải rác
    Board b = playSeq({"D5", "A1", "E5", "I9", "F5", "A9"});
    CHECK(evaluate(b, Cell::X) > 0);
    CHECK(evaluate(b, Cell::O) == -evaluate(b, Cell::X));
}

TEST_CASE("evaluate: bên đến lượt có 4 quân -> điểm thắng chắc") {
    // X có B5..E5 (4 quân), lượt X -> sắp thắng
    Board b = playSeq({"B5", "A1", "C5", "A3", "D5", "I9", "E5", "I1"});
    CHECK(b.toMove() == Cell::X);
    CHECK(evaluate(b, Cell::X) > THREAT_WIN_SCORE / 2);
    CHECK(evaluate(b, Cell::O) < -THREAT_WIN_SCORE / 2);
}

TEST_CASE("scoreMove: ô thắng/chặn được điểm cao nhất") {
    // X có B5..E5; lượt O: ô chặn A5 hoặc F5 phải được điểm cao nhất với O
    Board b = playSeq({"B5", "A1", "C5", "A3", "D5", "I9", "E5"});
    CHECK(b.toMove() == Cell::O);
    int best = -1;
    Move bestMove = NO_MOVE;
    for (Move m : candidates(b)) {
        const int s = scoreMove(b, m, Cell::O);
        if (s > best) {
            best = s;
            bestMove = m;
        }
    }
    CHECK(bestMove == sq("A5") || bestMove == sq("F5"));
}

// ---------------- Tác nhân Random & chạy ván ----------------

TEST_CASE("Random: luôn chọn ô trống; cùng seed -> cùng nước") {
    Board b = playSeq({"E5", "E6", "D4"});
    RandomAgent a(42);
    SearchStats s, s2;
    for (int i = 0; i < 200; ++i) CHECK(b.at(a.choose(b, s)) == Cell::Empty);
    RandomAgent c(7), c2(7);
    CHECK(c.choose(b, s) == c2.choose(b, s2));
}

TEST_CASE("1000 ván Random vs Random: luôn hợp lệ, luôn kết thúc, kết quả đúng") {
    int xWin = 0, oWin = 0, draw = 0;
    for (uint32_t g = 0; g < 1000; ++g) {
        RandomAgent x(g * 2 + 1), o(g * 2 + 2);
        Board last;
        GameResult r = playGame(x, o, Board(), [&](const Board& b, Move, const SearchStats&) { last = b; });
        CHECK(r.plies == last.stoneCount());
        CHECK(r.plies >= 9 && r.plies <= CELLS);
        CHECK(last.isTerminal());
        CHECK(bruteWinner(last) == r.winner);  // đối chiếu với cách kiểm tra độc lập
        CHECK(r.moves[0] - r.moves[1] == 0 || r.moves[0] - r.moves[1] == 1);
        if (r.winner == Cell::X) ++xWin;
        else if (r.winner == Cell::O) ++oWin;
        else ++draw;
    }
    std::printf("    Random vs Random 1000 van: X=%d O=%d hoa=%d\n", xWin, oWin, draw);
    CHECK(xWin + oWin + draw == 1000);
}

TEST_CASE("playGame ném lỗi khi tác nhân trả nước sai") {
    struct BadAgent : Agent {
        std::string name() const override { return "Bad"; }
        Move choose(const Board&, SearchStats&) override { return makeMove(4, 4); }  // luôn E5
    } bad;
    RandomAgent r(1);
    bool threw = false;
    try {
        playGame(bad, r);
    } catch (const std::logic_error&) {
        threw = true;
    }
    CHECK(threw);
}

TEST_CASE("Hiệu năng: 10000 ván Random vs Random") {
    const auto t0 = std::chrono::steady_clock::now();
    long long plies = 0;
    for (uint32_t g = 0; g < 10000; ++g) {
        RandomAgent x(g + 100000), o(g + 200000);
        plies += playGame(x, o).plies;
    }
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    std::printf("    10000 van, %lld nuoc: %.0f ms\n", plies, ms);
    CHECK(ms < 3000);
}
