// Test cho phần thực nghiệm: đọc chuỗi tác nhân, CSV, runMatch (tất định, đổi người đi trước).
#include <cstdio>
#include <stdexcept>
#include <string>

#include "agent_spec.h"
#include "csv.h"
#include "match.h"
#include "test_util.h"

using namespace bench;

TEST_CASE("parseSpec: đọc đúng tham số và nhãn") {
    const AgentSpec r = parseSpec("random");
    CHECK(r.kind == AgentSpec::Random && r.label == "Random");

    const AgentSpec m = parseSpec("mm:d=3,k=8,ab=0,r=1");
    CHECK(m.kind == AgentSpec::Minimax);
    CHECK(m.mm.depth == 3 && m.mm.topK == 8 && !m.mm.alphaBeta && m.mm.radius == 1);

    const AgentSpec c = parseSpec("mcts:it=250,c=0.5,roll=heur,fm=1");
    CHECK(c.kind == AgentSpec::Mcts);
    CHECK(c.mc.iterations == 250 && c.mc.c == 0.5 && c.mc.forcedMoves);
    CHECK(c.mc.rollout == caro::RolloutPolicy::Heuristic);
    CHECK(c.label == "MCTS(it=250,c=0.50,heur,fm)");
}

TEST_CASE("parseSpec: chuỗi sai -> ném invalid_argument") {
    for (const char* bad : {"foo", "mm:d=0", "mm:x=1", "mcts:it=0", "mcts:c=-1", "mcts:roll=abc", "mm:d=2x", "mm:d"}) {
        bool threw = false;
        try {
            parseSpec(bad);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        CHECK(threw);
    }
}

TEST_CASE("CSV: ghi rồi đọc lại trường có dấu phẩy và ngoặc kép") {
    const std::string path = "test_bench_tmp.csv";
    FILE* f = std::fopen(path.c_str(), "w");
    CHECK(f != nullptr);
    if (!f) return;
    writeCsvLine(f, {"a", "b", "c"});
    writeCsvLine(f, {"MCTS(it=300,c=1.40,rand)", "nói \"xin chào\"", "3"});
    std::fclose(f);
    const std::vector<CsvRow> rows = readCsv(path);
    CHECK(rows.size() == 1);
    if (rows.size() == 1) {
        CHECK(rows[0].at("a") == "MCTS(it=300,c=1.40,rand)");
        CHECK(rows[0].at("b") == "nói \"xin chào\"");
        CHECK(rows[0].at("c") == "3");
    }
    std::remove(path.c_str());
    CHECK(readCsv("khong_ton_tai.csv").empty());
}

TEST_CASE("runMatch: tổng ván đúng, tất định, không phụ thuộc số luồng") {
    const AgentSpec a = parseSpec("mcts:it=100,c=1.4"), b = parseSpec("random");
    const MatchStats m1 = runMatch(a, b, 10, 7, 1);
    const MatchStats m4 = runMatch(a, b, 10, 7, 4);
    CHECK(m1.win + m1.draw + m1.loss == 10);
    CHECK(m1.win == m4.win && m1.draw == m4.draw && m1.loss == m4.loss);
    CHECK(m1.plies == m4.plies && m1.movesA == m4.movesA && m1.movesB == m4.movesB);
    CHECK(m1.win > m1.loss);  // MCTS 100 mạnh hơn Random (seed 7: 8-0-2)
    // A đánh ở cả hai vai X và O: tổng số nước của A ~ một nửa số nước
    CHECK(m1.movesA + m1.movesB == m1.plies);
}

TEST_CASE("runMatch: đổi người đi trước theo cặp (Random vs Random cân bằng)") {
    // Hai bên cùng là Random, A cầm X và O xen kẽ -> điểm của A phải gần 50%.
    const MatchStats m = runMatch(parseSpec("random"), parseSpec("random"), 200, 1, 4);
    CHECK(m.win + m.draw + m.loss == 200);
    CHECK(m.score() > 0.4 && m.score() < 0.6);
}
