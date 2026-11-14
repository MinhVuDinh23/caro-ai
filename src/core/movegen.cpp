#include "movegen.h"

#include <array>

namespace caro {

std::vector<Move> allEmpty(const Board& b) {
    std::vector<Move> out;
    if (b.isTerminal()) return out;
    out.reserve(CELLS - b.stoneCount());
    for (Move m = 0; m < CELLS; ++m)
        if (b.at(m) == Cell::Empty) out.push_back(m);
    return out;
}

std::vector<Move> candidates(const Board& b, int radius) {
    std::vector<Move> out;
    if (b.isTerminal()) return out;
    if (b.stoneCount() == 0) {
        out.push_back(makeMove(N / 2, N / 2));
        return out;
    }
    if (radius < 1) radius = 1;

    // Đánh dấu các ô nằm trong vùng lân cận của một quân bất kỳ.
    std::array<bool, CELLS> near{};
    for (Move m = 0; m < CELLS; ++m) {
        if (b.at(m) == Cell::Empty) continue;
        const int r = rowOf(m), c = colOf(m);
        for (int dr = -radius; dr <= radius; ++dr)
            for (int dc = -radius; dc <= radius; ++dc)
                if (inBounds(r + dr, c + dc)) near[makeMove(r + dr, c + dc)] = true;
    }
    out.reserve(CELLS);
    for (Move m = 0; m < CELLS; ++m)
        if (near[m] && b.at(m) == Cell::Empty) out.push_back(m);
    return out;
}

}  // namespace caro
