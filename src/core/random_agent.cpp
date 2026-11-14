#include "random_agent.h"

#include "movegen.h"

namespace caro {

Move RandomAgent::choose(const Board& b, SearchStats& stats) {
    stats = SearchStats{};
    const std::vector<Move> moves = allEmpty(b);
    if (moves.empty()) return NO_MOVE;
    std::uniform_int_distribution<size_t> pick(0, moves.size() - 1);
    stats.move = moves[pick(rng_)];
    stats.nodes = 1;
    return stats.move;
}

}  // namespace caro
