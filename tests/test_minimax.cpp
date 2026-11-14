// Test MiniMax + Alpha-Beta: đúng luật, chọn nước đúng ở thế rõ ràng, cắt tỉa không đổi kết quả.
#include <cstdio>

#include "game_runner.h"
#include "log_format.h"
#include "minimax_agent.h"
#include "positions.h"
#include "random_agent.h"
#include "test_util.h"

using namespace caro;

namespace {
const Board& pos(const char* prefix) {
    for (const TestPosition& p : testPositions())
        if (p.name.rfind(prefix, 0) == 0) return p.board;
    static Board empty;
    return empty;
}
Move sq(const char* s) { return makeMove(s[1] - '1', s[0] - 'A'); }
}  // namespace

TEST_CASE("Thế cờ thử nghiệm hợp lệ: chưa kết thúc, số quân khớp lượt đi") {
    CHECK(testPositions().size() == 8);
    for (const TestPosition& p : testPositions()) {
        int x = 0, o = 0;
        for (Move m = 0; m < CELLS; ++m) {
            if (p.board.at(m) == Cell::X) ++x;
            if (p.board.at(m) == Cell::O) ++o;
        }
        CHECK(!p.board.isTerminal());
        // X đi trước: lượt X khi x == o, lượt O khi x == o + 1
        CHECK((p.board.toMove() == Cell::X && x == o) || (p.board.toMove() == Cell::O && x == o + 1));
    }
}

TEST_CASE("MiniMax đánh nước thắng ngay (d = 1, 2, 3)") {
    for (int d = 1; d <= 3; ++d) {
        MinimaxAgent mm({d, true, 12, 2});
        SearchStats s;
        const Move m = mm.choose(pos("P5"), s);
        CHECK(m == sq("A7") || m == sq("F7"));
        CHECK(s.value >= MinimaxAgent::WIN_SCORE);
        CHECK(s.depth == d);
    }
}

TEST_CASE("MiniMax chặn đường 4 của đối thủ (d = 2, 3)") {
    for (int d = 2; d <= 3; ++d) {
        MinimaxAgent mm({d, true, 12, 2});
        SearchStats s;
        CHECK(mm.choose(pos("P4"), s) == sq("G7"));
    }
}

TEST_CASE("MiniMax chặn 3 mở của đối thủ (d = 2)") {
    // X có D4 E4 F4 mở hai đầu: O phải đánh C4 hoặc G4
    MinimaxAgent mm({2, true, 12, 2});
    SearchStats s;
    const Move m = mm.choose(pos("P6"), s);
    CHECK(m == sq("C4") || m == sq("G4"));
}

TEST_CASE("Alpha-Beta cho cùng giá trị và cùng nước như MiniMax không cắt tỉa") {
    std::printf("    %-22s d  %10s %10s %8s  %s\n", "the co", "nut(AB)", "nut(thuong)", "giam", "nuoc");
    for (const TestPosition& p : testPositions()) {
        for (int d = 1; d <= 3; ++d) {
            for (int k : {12, 0}) {  // có và không giới hạn ứng viên
                if (k == 0 && d == 3) continue;  // d=3 không giới hạn: chậm, đã kiểm tra ở d<=2
                MinimaxAgent ab({d, true, k, 2}), plain({d, false, k, 2});
                SearchStats sa, sp;
                const Move ma = ab.choose(p.board, sa);
                const Move mp = plain.choose(p.board, sp);
                CHECK(sa.value == sp.value);
                CHECK(ma == mp);
                CHECK(sa.nodes <= sp.nodes);
                CHECK(sp.cutoffs == 0);
                CHECK(p.board.isLegal(ma));
                if (k == 12 && d >= 2)
                    std::printf("    %-22s %d  %10lld %10lld %7.1f%%  %s\n", p.name.c_str(), d, sa.nodes,
                                sp.nodes, 100.0 * (1.0 - double(sa.nodes) / double(sp.nodes)),
                                moveToString(ma).c_str());
            }
        }
    }
}

TEST_CASE("MiniMax tất định và không thay đổi bàn cờ đầu vào") {
    const Board& b = pos("P7");
    const Board copy = b;
    MinimaxAgent mm({3, true, 12, 2});
    SearchStats s1, s2;
    const Move m1 = mm.choose(b, s1);
    const Move m2 = mm.choose(b, s2);
    CHECK(m1 == m2);
    CHECK(s1.nodes == s2.nodes);
    CHECK(b.history() == copy.history());
    for (Move m = 0; m < CELLS; ++m) CHECK(b.at(m) == copy.at(m));
    CHECK(s1.nodes > 1);
    CHECK(s1.ms >= 0);
}

TEST_CASE("orderedMoves: tôn trọng topK, nước tốt nhất đứng đầu") {
    MinimaxAgent mm({2, true, 5, 2});
    std::vector<Move> mv = mm.orderedMoves(pos("P5"));
    CHECK(mv.size() == 5);
    CHECK(mv[0] == sq("A7") || mv[0] == sq("F7"));
    MinimaxAgent all({2, true, 0, 2});
    CHECK(all.orderedMoves(pos("P5")).size() > 5);
}

TEST_CASE("MiniMax: ván đã kết thúc -> NO_MOVE; bàn trống -> ô giữa") {
    MinimaxAgent mm;
    SearchStats s;
    Board won = Board::fromRows({"XXXXX....", "OOOO.....", ".........", ".........", ".........",
                                 ".........", ".........", ".........", "........."},
                                Cell::O);
    CHECK(mm.choose(won, s) == NO_MOVE);
    CHECK(mm.choose(Board(), s) == makeMove(4, 4));
}

TEST_CASE("Nhật ký nước đi có đủ: nước, độ sâu, thời gian, số nút") {
    MinimaxAgent mm({2, true, 12, 2});
    SearchStats s;
    mm.choose(pos("P2"), s);
    const std::string line = formatMoveLog(mm.name(), Cell::X, s);
    std::printf("    %s\n", line.c_str());
    CHECK(line.find("MiniMax(d=2,AB,k=12)") != std::string::npos);
    CHECK(line.find(moveToString(s.move)) != std::string::npos);
    CHECK(line.find("d=2") != std::string::npos);
    CHECK(line.find("ms") != std::string::npos);
    CHECK(line.find("nút=") != std::string::npos);
}

TEST_CASE("MiniMax d=2 thắng Random >= 95% (20 ván, đổi người đi trước)") {
    int win = 0, draw = 0, loss = 0;
    double msTotal = 0;
    int mmMoves = 0;
    for (uint32_t g = 0; g < 20; ++g) {
        MinimaxAgent mm({2, true, 12, 2});
        RandomAgent rnd(1000 + g);
        const bool mmIsX = g % 2 == 0;
        GameResult r = mmIsX ? playGame(mm, rnd) : playGame(rnd, mm);
        const Cell mmSide = mmIsX ? Cell::X : Cell::O;
        if (r.winner == mmSide) ++win;
        else if (r.winner == Cell::Empty) ++draw;
        else ++loss;
        msTotal += r.totalMs[mmIsX ? 0 : 1];
        mmMoves += r.moves[mmIsX ? 0 : 1];
    }
    std::printf("    MiniMax d=2 vs Random: thang=%d hoa=%d thua=%d, %.2f ms/nuoc\n", win, draw, loss,
                msTotal / mmMoves);
    CHECK(win >= 19);
}

TEST_CASE("Hiệu năng MiniMax ở độ sâu lớn (thế P7, k=12)") {
    for (int d = 4; d <= 5; ++d) {
        for (bool ab : {true, false}) {
            if (!ab && d == 5) continue;  // không cắt tỉa ở d=5: ~270k nút, bỏ qua cho test nhanh
            MinimaxAgent mm({d, ab, 12, 2});
            SearchStats s;
            mm.choose(pos("P7"), s);
            std::printf("    d=%d %-5s nut=%-8lld %.1f ms  nuoc=%s\n", d, ab ? "AB" : "thuong", s.nodes, s.ms,
                        moveToString(s.move).c_str());
            CHECK(s.ms < 2000);
        }
    }
}

TEST_CASE("MiniMax d=3 vs MiniMax d=2: không lỗi, ván kết thúc hợp lệ") {
    MinimaxAgent d3({3, true, 12, 2}), d2({2, true, 12, 2});
    GameResult a = playGame(d3, d2);
    GameResult b = playGame(d2, d3);
    std::printf("    d3(X) vs d2(O): %c | d2(X) vs d3(O): %c | d3 %.2f ms/nuoc\n",
                cellChar(a.winner), cellChar(b.winner), a.avgMs(Cell::X));
    CHECK(a.plies > 0 && b.plies > 0);
}
