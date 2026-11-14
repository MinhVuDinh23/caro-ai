#include "experiments.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <stdexcept>
#include <vector>

#include "csv.h"
#include "match.h"
#include "minimax_agent.h"
#include "positions.h"

namespace bench {

using namespace caro;

namespace {
// Đối thủ cố định dùng chung cho các phép quét
const char* RANDOM = "random";
const char* BASELINE = "mcts:it=300,c=1.4,roll=rand";  // MCTS cơ sở
const char* MM1 = "mm:d=1,k=12";                      // MiniMax d=1 (tham lam 1 nước + hàm đánh giá)

FILE* openCsv(const ExpContext& ctx, const char* name) {
    const std::string path = ctx.outDir + "/" + name;
    FILE* f = std::fopen(path.c_str(), "w");
    if (!f) throw std::runtime_error("không mở được " + path);
    writeCsvLine(f, {"exp", "group", "agent_a", "agent_b", "param", "value", "games", "win", "draw", "loss",
                     "win_rate", "score", "avg_ms_a", "avg_ms_b", "avg_plies", "seed"});
    return f;
}

// Chạy một trận, in tiến độ và ghi một dòng CSV.
// group: nhóm để vẽ (thường là đối thủ), param/value: tham số đang quét.
MatchStats play(FILE* f, const ExpContext& ctx, const char* exp, const std::string& group, const std::string& a,
                const std::string& b, const std::string& param, const std::string& value, int games) {
    const auto t0 = std::chrono::steady_clock::now();
    const MatchStats m = runMatch(parseSpec(a), parseSpec(b), games, ctx.seed, ctx.threads);
    const double sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    std::printf("[%s] %-30s vs %-30s %3d-%3d-%3d  điểm=%.3f  %.2f ms/nước  (%.1f s)\n", exp, m.a.c_str(), m.b.c_str(),
                m.win, m.draw, m.loss, m.score(), m.avgMsA(), sec);
    std::fflush(stdout);
    writeCsvLine(f, {exp, group, m.a, m.b, param, value, std::to_string(m.games), std::to_string(m.win),
                     std::to_string(m.draw), std::to_string(m.loss), fmtNum(m.winRate(), 4), fmtNum(m.score(), 4),
                     fmtNum(m.avgMsA(), 3), fmtNum(m.avgMsB(), 3), fmtNum(m.avgPlies(), 2), std::to_string(m.seed)});
    return m;
}

std::string mcts(int it, const std::string& c, const char* roll, bool fm = false) {
    return "mcts:it=" + std::to_string(it) + ",c=" + c + ",roll=" + roll + (fm ? ",fm=1" : "");
}
}  // namespace

// E1: đấu vòng tròn, mỗi cặp ctx.games ván (đổi người đi trước theo cặp).
void expCross(const ExpContext& ctx) {
    const std::vector<std::string> agents = {RANDOM,
                                             "mm:d=1,k=12",
                                             "mm:d=2,k=12",
                                             "mm:d=3,k=12",
                                             mcts(500, "1.4", "rand"),
                                             mcts(500, "1.4", "heur"),
                                             mcts(3000, "1.4", "rand"),
                                             mcts(500, "1.4", "rand", true)};
    FILE* f = openCsv(ctx, "cross.csv");
    for (size_t i = 0; i < agents.size(); ++i)
        for (size_t j = i + 1; j < agents.size(); ++j)
            play(f, ctx, "cross", "cross", agents[i], agents[j], "", "", ctx.games);
    std::fclose(f);
}

// E2: quét c ở số iteration cố định 300 (theo đề) với đối thủ cố định;
//     thêm phép quét bổ sung ở 10000 iteration với MiniMax d=1 (đối thủ đủ mạnh để thấy khác biệt).
void expCsweep(const ExpContext& ctx) {
    const std::vector<std::string> cs = {"0.5", "1.0", "1.4", "2.0"};
    FILE* f = openCsv(ctx, "csweep.csv");
    for (const std::string& c : cs) play(f, ctx, "csweep", "vs Random", mcts(300, c, "rand"), RANDOM, "c", c, ctx.games);
    for (const std::string& c : cs)
        play(f, ctx, "csweep", "vs MCTS(300,c=1.4)", mcts(300, c, "rand"), BASELINE, "c", c, ctx.gamesSelf);
    for (const std::string& c : cs)
        play(f, ctx, "csweep", "vs MiniMax d=1 (it=10000)", mcts(10000, c, "rand"), MM1, "c", c, ctx.games);
    std::fclose(f);
}

// E3: quét số iteration ở c = 1.4 cố định: {100,300,500} theo đề + {1000,3000,10000} mở rộng.
void expItersweep(const ExpContext& ctx) {
    const std::vector<int> its = {100, 300, 500, 1000, 3000, 10000};
    FILE* f = openCsv(ctx, "itersweep.csv");
    for (int it : its) play(f, ctx, "itersweep", "vs Random", mcts(it, "1.4", "rand"), RANDOM, "iterations", std::to_string(it), ctx.games);
    for (int it : its)
        play(f, ctx, "itersweep", "vs MCTS(300,c=1.4)", mcts(it, "1.4", "rand"), BASELINE, "iterations", std::to_string(it),
             ctx.gamesSelf);
    for (int it : its) play(f, ctx, "itersweep", "vs MiniMax d=1", mcts(it, "1.4", "rand"), MM1, "iterations", std::to_string(it), ctx.games);
    std::fclose(f);
}

// E5 (Bài 2 - Hướng A): rollout heuristic so với rollout ngẫu nhiên.
//   (a) đối đầu trực tiếp cùng số iteration;
//   (b) "cùng thời gian": rollout ngẫu nhiên được 20x iteration (heuristic chậm hơn ~20-25 lần/iteration);
//   (c) mỗi loại rollout gặp cùng đối thủ cố định (Random, MiniMax d=1).
void expRollout(const ExpContext& ctx) {
    const std::vector<int> its = {100, 300, 500};
    FILE* f = openCsv(ctx, "rollout.csv");
    for (int it : its)
        play(f, ctx, "rollout", "heur vs rand (cùng iteration)", mcts(it, "1.4", "heur"), mcts(it, "1.4", "rand"), "iterations",
             std::to_string(it), ctx.gamesSelf);
    for (int it : its)
        play(f, ctx, "rollout", "heur vs rand 20x iteration (cùng thời gian)", mcts(it, "1.4", "heur"),
             mcts(20 * it, "1.4", "rand"), "iterations", std::to_string(it), ctx.games);
    for (const char* roll : {"rand", "heur"})
        for (int it : its)
            play(f, ctx, "rollout", std::string("vs Random/") + roll, mcts(it, "1.4", roll), RANDOM, "iterations",
                 std::to_string(it), ctx.games);
    for (const char* roll : {"rand", "heur"})
        for (int it : its)
            play(f, ctx, "rollout", std::string("vs MiniMax d=1/") + roll, mcts(it, "1.4", roll), MM1, "iterations",
                 std::to_string(it), ctx.games);
    std::fclose(f);
}

// E4: MiniMax có / không cắt tỉa trên 8 thế cờ cố định: số nút, số lần cắt, thời gian, nước chọn.
// Chạy tuần tự một luồng; thời gian lấy trung bình nhiều lần chạy (>= 50 ms tổng hoặc 20 lần).
void expAb(const ExpContext& ctx) {
    const std::string path = ctx.outDir + "/ab.csv";
    FILE* f = std::fopen(path.c_str(), "w");
    if (!f) throw std::runtime_error("không mở được " + path);
    writeCsvLine(f, {"position", "depth", "topk", "alphabeta", "move", "value", "nodes", "cutoffs", "ms"});
    struct Cfg { int d, k; };
    const std::vector<Cfg> cfgs = {{1, 12}, {2, 12}, {3, 12}, {4, 12}, {1, 0}, {2, 0}, {3, 0}};
    for (const TestPosition& p : testPositions()) {
        for (const Cfg& c : cfgs) {
            for (bool ab : {true, false}) {
                MinimaxAgent agent(MinimaxConfig{c.d, ab, c.k, 2});
                SearchStats s;
                Move m = agent.choose(p.board, s);
                double total = s.ms;
                int runs = 1;
                while (total < 50 && runs < 20) {
                    SearchStats s2;
                    agent.choose(p.board, s2);
                    total += s2.ms;
                    ++runs;
                }
                writeCsvLine(f, {p.name, std::to_string(c.d), std::to_string(c.k), ab ? "1" : "0", moveToString(m),
                                 std::to_string(s.value), std::to_string(s.nodes), std::to_string(s.cutoffs),
                                 fmtNum(total / runs, 4)});
                std::printf("[ab] %-24s d=%d k=%-2d %-5s -> %-3s nút=%-8lld cắt=%-6lld %.3f ms\n", p.name.c_str(), c.d,
                            c.k, ab ? "AB" : "thuần", moveToString(m).c_str(), s.nodes, s.cutoffs, total / runs);
            }
        }
    }
    std::fclose(f);
}

}  // namespace bench
