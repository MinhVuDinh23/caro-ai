#include "evaluator.h"

namespace caro {

namespace {
struct WindowTable {
    std::vector<Window> windows;
    std::array<std::vector<int>, CELLS> ofCell;

    WindowTable() {
        constexpr int DR[4] = {0, 1, 1, 1};
        constexpr int DC[4] = {1, 0, 1, -1};
        for (int r = 0; r < N; ++r)
            for (int c = 0; c < N; ++c)
                for (int d = 0; d < 4; ++d) {
                    const int er = r + (K - 1) * DR[d], ec = c + (K - 1) * DC[d];
                    if (!inBounds(er, ec)) continue;
                    Window w;
                    for (int i = 0; i < K; ++i) w[i] = makeMove(r + i * DR[d], c + i * DC[d]);
                    const int idx = static_cast<int>(windows.size());
                    windows.push_back(w);
                    for (Move m : w) ofCell[m].push_back(idx);
                }
    }
};

const WindowTable& table() {
    static const WindowTable t;  // khởi tạo một lần, an toàn đa luồng (C++11)
    return t;
}

// Đếm số quân X và O trong một cửa sổ.
inline void countWindow(const Board& b, const Window& w, int& x, int& o) {
    x = o = 0;
    for (Move m : w) {
        const Cell v = b.at(m);
        if (v == Cell::X) ++x;
        else if (v == Cell::O) ++o;
    }
}
}  // namespace

const std::vector<Window>& allWindows() { return table().windows; }
const std::vector<int>& windowsOfCell(Move m) { return table().ofCell[m]; }

int evaluate(const Board& b, Cell perspective) {
    long long sx = 0, so = 0;
    bool fourX = false, fourO = false;  // có cửa sổ 4 quân + 1 ô trống
    for (const Window& w : allWindows()) {
        int x, o;
        countWindow(b, w, x, o);
        if (o == 0 && x > 0) {
            sx += WINDOW_WEIGHT[x];
            if (x == K - 1) fourX = true;
        } else if (x == 0 && o > 0) {
            so += WINDOW_WEIGHT[o];
            if (o == K - 1) fourO = true;
        }
    }
    long long score = sx - so;  // góc nhìn X
    // Bên đến lượt có sẵn 4 quân trong một cửa sổ -> đánh nốt ô còn lại là thắng.
    if (!b.isTerminal()) {
        if (b.toMove() == Cell::X && fourX) score += THREAT_WIN_SCORE;
        if (b.toMove() == Cell::O && fourO) score -= THREAT_WIN_SCORE;
    }
    if (perspective == Cell::O) score = -score;
    return static_cast<int>(score);
}

int scoreMove(const Board& b, Move m, Cell side) {
    int attack = 0, defend = 0;
    for (int idx : windowsOfCell(m)) {
        int x, o;
        countWindow(b, allWindows()[idx], x, o);
        const int mine = side == Cell::X ? x : o;
        const int theirs = side == Cell::X ? o : x;
        if (theirs == 0) attack += WINDOW_WEIGHT[mine + 1] - WINDOW_WEIGHT[mine];
        if (mine == 0) defend += WINDOW_WEIGHT[theirs + 1] - WINDOW_WEIGHT[theirs];
    }
    return attack + defend;
}

void immediateWins(const Board& b, Cell side, std::vector<Move>& wins, std::vector<Move>& threats) {
    wins.clear();
    threats.clear();
    if (b.isTerminal()) return;
    std::array<bool, CELLS> isWin{}, isThreat{};
    for (const Window& w : allWindows()) {
        int mine = 0, theirs = 0;
        Move empty = NO_MOVE;
        for (Move m : w) {
            const Cell v = b.at(m);
            if (v == Cell::Empty) empty = m;
            else if (v == side) ++mine;
            else ++theirs;
        }
        if (mine == K - 1 && theirs == 0) isWin[empty] = true;     // 4 quân của mình + 1 ô trống
        if (theirs == K - 1 && mine == 0) isThreat[empty] = true;  // 4 quân đối thủ + 1 ô trống
    }
    for (Move m = 0; m < CELLS; ++m) {
        if (isWin[m]) wins.push_back(m);
        if (isThreat[m]) threats.push_back(m);
    }
}

std::vector<Move> forcedMoves(const Board& b) {
    std::vector<Move> wins, threats;
    immediateWins(b, b.toMove(), wins, threats);
    return wins.empty() ? threats : wins;
}

}  // namespace caro
