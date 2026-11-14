// Test MCTS + UCT: công thức UCT, phần thưởng, thống kê cây, chọn nước theo N, chất lượng nước đi.
#include <cmath>
#include <cstdio>
#include <limits>

#include "evaluator.h"
#include "game_runner.h"
#include "log_format.h"
#include "mcts_agent.h"
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

MctsConfig cfg(int it, double c, RolloutPolicy r, uint32_t seed, int top = 5, bool forced = false) {
    MctsConfig m;
    m.iterations = it;
    m.c = c;
    m.rollout = r;
    m.seed = seed;
    m.reportTop = top;
    m.forcedMoves = forced;
    return m;
}
const char* rname(RolloutPolicy r) { return r == RolloutPolicy::Random ? "rand" : "heur"; }
}  // namespace

TEST_CASE("UCT: nút chưa thăm = +vô cùng; công thức đúng") {
    CHECK(std::isinf(MctsAgent::uct(0, 0, 10, 1.4)));
    CHECK(MctsAgent::uct(0, 0, 0, 1.4) > 1e300);
    const double v = MctsAgent::uct(3, 4, 20, 1.4);
    CHECK(std::fabs(v - (0.75 + 1.4 * std::sqrt(std::log(20.0) / 4))) < 1e-12);
    CHECK(MctsAgent::uct(2, 4, 20, 2.0) > MctsAgent::uct(2, 4, 20, 0.5));  // c lớn -> khám phá nhiều
}

TEST_CASE("Phần thưởng: thắng 1, hòa 0.5, thua 0") {
    CHECK(MctsAgent::reward(Cell::X, Cell::X) == 1.0);
    CHECK(MctsAgent::reward(Cell::Empty, Cell::X) == 0.5);
    CHECK(MctsAgent::reward(Cell::O, Cell::X) == 0.0);
}

TEST_CASE("Tổng N các con của gốc = số iteration; nước chọn có N lớn nhất") {
    for (RolloutPolicy r : {RolloutPolicy::Random, RolloutPolicy::Heuristic}) {
        MctsAgent a(cfg(400, 1.4, r, 7, 1000));
        SearchStats s;
        const Move m = a.choose(pos("P2"), s);
        int sum = 0, maxN = 0;
        for (const ChildInfo& c : s.topChildren) {
            sum += c.visits;
            maxN = std::max(maxN, c.visits);
            CHECK(c.meanReward >= 0.0 && c.meanReward <= 1.0);
            CHECK(pos("P2").isLegal(c.move));
        }
        CHECK(sum == 400);
        CHECK(s.iterations == 400);
        CHECK(!s.topChildren.empty() && s.topChildren[0].move == m);
        CHECK(s.topChildren[0].visits == maxN);
        CHECK(s.nodes == 401);  // mỗi iteration mở rộng đúng 1 nút (+ gốc), khi chưa gặp nút kết thúc
    }
}

TEST_CASE("immediateWins / forcedMoves tìm đúng ô thắng và ô phải chặn") {
    std::vector<Move> wins, threats;
    immediateWins(pos("P5"), Cell::X, wins, threats);
    CHECK(wins == std::vector<Move>({sq("A7"), sq("F7")}));
    CHECK(threats.empty());
    CHECK(forcedMoves(pos("P5")) == std::vector<Move>({sq("A7"), sq("F7")}));
    CHECK(forcedMoves(pos("P4")) == std::vector<Move>({sq("G7")}));  // O phải chặn
    CHECK(forcedMoves(pos("P2")).empty());                           // không có nước bắt buộc
    CHECK(forcedMoves(pos("P6")).empty());                           // 3 mở chưa phải "thắng ngay"
}

TEST_CASE("MCTS + forcedMoves: luôn thắng ngay (P5) và luôn chặn 4 (P4)") {
    for (RolloutPolicy r : {RolloutPolicy::Random, RolloutPolicy::Heuristic}) {
        for (uint32_t seed = 1; seed <= 10; ++seed) {
            MctsAgent a(cfg(500, 1.4, r, seed, 100, true));
            SearchStats s;
            const Move m5 = a.choose(pos("P5"), s);
            CHECK(m5 == sq("A7") || m5 == sq("F7"));
            const Move m4 = a.choose(pos("P4"), s);
            CHECK(m4 == sq("G7"));
            CHECK(s.topChildren.size() == 1 && s.topChildren[0].visits == 500);  // gốc chỉ có 1 con
        }
    }
}

TEST_CASE("MCTS thuần ở thế chiến thuật (chỉ ghi số liệu - điểm yếu đã biết)") {
    for (RolloutPolicy r : {RolloutPolicy::Random, RolloutPolicy::Heuristic}) {
        int ok4 = 0, ok5 = 0;
        for (uint32_t seed = 1; seed <= 20; ++seed) {
            MctsAgent a(cfg(500, 1.4, r, seed));
            SearchStats s;
            if (a.choose(pos("P4"), s) == sq("G7")) ++ok4;
            const Move m = a.choose(pos("P5"), s);
            if (m == sq("A7") || m == sq("F7")) ++ok5;
        }
        std::printf("    MCTS thuan (500, c=1.4, %s): P4 chan %d/20, P5 thang ngay %d/20\n", rname(r), ok4, ok5);
    }
}

TEST_CASE("MCTS tất định theo seed; không đổi bàn đầu vào; ván kết thúc -> NO_MOVE") {
    const Board& b = pos("P7");
    const Board copy = b;
    MctsAgent a(cfg(300, 1.4, RolloutPolicy::Random, 42)), c(cfg(300, 1.4, RolloutPolicy::Random, 42));
    SearchStats s1, s2;
    CHECK(a.choose(b, s1) == c.choose(b, s2));
    CHECK(s1.topChildren.size() == s2.topChildren.size());
    for (size_t i = 0; i < s1.topChildren.size() && i < s2.topChildren.size(); ++i)
        CHECK(s1.topChildren[i].visits == s2.topChildren[i].visits);
    for (Move m = 0; m < CELLS; ++m) CHECK(b.at(m) == copy.at(m));

    Board won = Board::fromRows({"XXXXX....", "OOOO.....", ".........", ".........", ".........",
                                 ".........", ".........", ".........", "........."},
                                Cell::O);
    SearchStats s;
    CHECK(a.choose(won, s) == NO_MOVE);
    CHECK(a.choose(Board(), s) == makeMove(4, 4));  // bàn trống: ứng viên duy nhất là ô giữa
}

TEST_CASE("MCTS gần cuối ván (ít ô trống, có nút kết thúc trong cây) không lỗi") {
    // Bàn hòa gần đầy, chỉ còn 3 ô trống
    std::vector<std::string> rows(N, std::string(N, '.'));
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c) rows[r][c] = ((c / 2) + r) % 2 == 0 ? 'X' : 'O';
    rows[8][8] = rows[8][7] = rows[7][8] = '.';
    int x = 0, o = 0;
    for (const auto& s : rows)
        for (char ch : s) x += ch == 'X', o += ch == 'O';
    Board b = Board::fromRows(rows, x == o ? Cell::X : Cell::O);
    CHECK(!b.isTerminal());
    for (RolloutPolicy r : {RolloutPolicy::Random, RolloutPolicy::Heuristic}) {
        MctsAgent a(cfg(200, 1.4, r, 3, 100));
        SearchStats s;
        const Move m = a.choose(b, s);
        CHECK(b.isLegal(m));
        int sum = 0;
        for (const ChildInfo& c : s.topChildren) sum += c.visits;
        CHECK(sum == 200);
    }
}

TEST_CASE("Nhật ký MCTS có iteration và top nước") {
    MctsAgent a(cfg(300, 1.4, RolloutPolicy::Random, 1));
    SearchStats s;
    a.choose(pos("P2"), s);
    const std::string line = formatMoveLog(a.name(), Cell::X, s);
    std::printf("    %s\n", line.c_str());
    CHECK(line.find("iter=300") != std::string::npos);
    CHECK(line.find("top:") != std::string::npos);
    CHECK(s.topChildren.size() == 5);
}

namespace {
struct Score {
    int win = 0, draw = 0, loss = 0;
    double ms = 0;
    int moves = 0;
};
// 2*pairs ván, đổi người đi trước mỗi cặp. makeA/makeB nhận seed.
template <class FA, class FB>
Score match(FA makeA, FB makeB, int pairs, uint32_t seed) {
    Score sc;
    for (int g = 0; g < 2 * pairs; ++g) {
        auto a = makeA(seed + g);
        auto b = makeB(seed + 1000 + g);
        const bool aIsX = g % 2 == 0;
        GameResult r = aIsX ? playGame(*a, *b) : playGame(*b, *a);
        const Cell aSide = aIsX ? Cell::X : Cell::O;
        if (r.winner == aSide) ++sc.win;
        else if (r.winner == Cell::Empty) ++sc.draw;
        else ++sc.loss;
        sc.ms += r.totalMs[aIsX ? 0 : 1];
        sc.moves += r.moves[aIsX ? 0 : 1];
    }
    return sc;
}
}  // namespace

TEST_CASE("MCTS(500, rand) thắng Random >= 90% (20 ván)") {
    Score s = match([](uint32_t sd) { return std::make_unique<MctsAgent>(cfg(500, 1.4, RolloutPolicy::Random, sd)); },
                    [](uint32_t sd) { return std::make_unique<RandomAgent>(sd); }, 10, 100);
    std::printf("    MCTS(500,rand) vs Random: %d-%d-%d, %.2f ms/nuoc\n", s.win, s.draw, s.loss, s.ms / s.moves);
    CHECK(s.win >= 18);
}

TEST_CASE("Rollout heuristic vs rollout ngẫu nhiên (20 ván, 300 iteration)") {
    Score s = match(
        [](uint32_t sd) { return std::make_unique<MctsAgent>(cfg(300, 1.4, RolloutPolicy::Heuristic, sd)); },
        [](uint32_t sd) { return std::make_unique<MctsAgent>(cfg(300, 1.4, RolloutPolicy::Random, sd)); }, 10, 200);
    std::printf("    MCTS heur vs MCTS rand: %d-%d-%d, heur %.2f ms/nuoc\n", s.win, s.draw, s.loss, s.ms / s.moves);
    CHECK(s.win + s.draw + s.loss == 20);
    CHECK(s.win > s.loss);
}

TEST_CASE("MCTS vs MiniMax d=2 (thông tin, 10 ván mỗi cấu hình)") {
    for (bool forced : {false, true}) {
        for (RolloutPolicy r : {RolloutPolicy::Random, RolloutPolicy::Heuristic}) {
            Score s = match(
                [r, forced](uint32_t sd) { return std::make_unique<MctsAgent>(cfg(500, 1.4, r, sd, 5, forced)); },
                [](uint32_t) { return std::make_unique<MinimaxAgent>(MinimaxConfig{2, true, 12, 2}); }, 5, 300);
            std::printf("    MCTS(500,%s%s) vs MiniMax d=2: %d-%d-%d, %.2f ms/nuoc\n", rname(r), forced ? ",fm" : "",
                        s.win, s.draw, s.loss, s.ms / s.moves);
            CHECK(s.win + s.draw + s.loss == 10);
        }
    }
}

TEST_CASE("MCTS + forcedMoves thắng MCTS thuần (20 ván, rand, 500 iteration)") {
    Score s = match(
        [](uint32_t sd) { return std::make_unique<MctsAgent>(cfg(500, 1.4, RolloutPolicy::Random, sd, 5, true)); },
        [](uint32_t sd) { return std::make_unique<MctsAgent>(cfg(500, 1.4, RolloutPolicy::Random, sd)); }, 10, 400);
    std::printf("    MCTS fm vs MCTS thuan: %d-%d-%d\n", s.win, s.draw, s.loss);
    CHECK(s.win > s.loss);
}
