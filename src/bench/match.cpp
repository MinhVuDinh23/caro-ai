#include "match.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

#include "game_runner.h"

namespace bench {

using namespace caro;

MatchStats runMatch(const AgentSpec& a, const AgentSpec& b, int games, uint32_t seed, int threads) {
    std::vector<GameResult> results(static_cast<size_t>(games));
    std::atomic<int> next{0};

    auto worker = [&]() {
        for (int i = next++; i < games; i = next++) {
            const uint32_t base = seed + 7919u * static_cast<uint32_t>(i / 2);
            auto agentA = makeAgent(a, base + 1);
            auto agentB = makeAgent(b, base + 2);
            const bool aIsX = i % 2 == 0;
            results[static_cast<size_t>(i)] = aIsX ? playGame(*agentA, *agentB) : playGame(*agentB, *agentA);
        }
    };
    const int n = std::max(1, std::min(threads, games));
    std::vector<std::thread> pool;
    for (int t = 0; t < n; ++t) pool.emplace_back(worker);
    for (std::thread& t : pool) t.join();

    // Tổng hợp theo đúng thứ tự ván -> tất định
    MatchStats s;
    s.a = a.label;
    s.b = b.label;
    s.games = games;
    s.seed = seed;
    for (int i = 0; i < games; ++i) {
        const GameResult& r = results[static_cast<size_t>(i)];
        const bool aIsX = i % 2 == 0;
        const int ia = aIsX ? 0 : 1, ib = 1 - ia;
        const Cell aSide = aIsX ? Cell::X : Cell::O;
        if (r.winner == aSide) ++s.win;
        else if (r.winner == Cell::Empty) ++s.draw;
        else ++s.loss;
        s.totalMsA += r.totalMs[ia];
        s.totalMsB += r.totalMs[ib];
        s.movesA += r.moves[ia];
        s.movesB += r.moves[ib];
        s.plies += r.plies;
    }
    return s;
}

}  // namespace bench
