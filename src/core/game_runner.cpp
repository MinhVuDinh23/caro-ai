#include "game_runner.h"

#include <chrono>
#include <stdexcept>

namespace caro {

GameResult playGame(Agent& xAgent, Agent& oAgent, Board board, const MoveCallback& onMove) {
    GameResult res;
    while (!board.isTerminal()) {
        const Cell side = board.toMove();
        Agent& agent = side == Cell::X ? xAgent : oAgent;
        SearchStats stats;

        const auto t0 = std::chrono::steady_clock::now();
        const Move m = agent.choose(board, stats);
        const auto t1 = std::chrono::steady_clock::now();
        const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        if (!board.play(m))
            throw std::logic_error(agent.name() + " trả về nước không hợp lệ: " + moveToString(m));
        const int i = side == Cell::X ? 0 : 1;
        res.totalMs[i] += ms;
        ++res.moves[i];
        stats.ms = ms;
        if (onMove) onMove(board, m, stats);
    }
    res.winner = board.winner();
    res.plies = res.moves[0] + res.moves[1];
    return res;
}

}  // namespace caro
