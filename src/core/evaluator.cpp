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

}  // namespace caro
