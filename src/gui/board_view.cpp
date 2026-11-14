#include "board_view.h"

#include <string>

using namespace caro;

namespace {
const Color BOARD_BG = {236, 204, 150, 255};
const Color GRID = {120, 90, 50, 255};
const Color X_COLOR = {200, 40, 40, 255};
const Color O_COLOR = {30, 80, 190, 255};
const Color LAST_MOVE = {255, 236, 120, 255};
const Color WIN_CELL = {150, 220, 140, 255};
const Color HOVER = {255, 255, 255, 110};
}  // namespace

Rectangle BoardView::cellRect(Move m) const {
    return {x + colOf(m) * cell, y + rowOf(m) * cell, cell, cell};
}

Vector2 BoardView::center(Move m) const {
    return {x + (colOf(m) + 0.5f) * cell, y + (rowOf(m) + 0.5f) * cell};
}

Move BoardView::cellAt(Vector2 p) const {
    if (!CheckCollisionPointRec(p, rect())) return NO_MOVE;
    const int c = static_cast<int>((p.x - x) / cell);
    const int r = static_cast<int>((p.y - y) / cell);
    return inBounds(r, c) ? makeMove(r, c) : NO_MOVE;
}

void drawBoard(const BoardView& v, const Board& b, const Font& font, Move hover) {
    const Rectangle R = v.rect();
    DrawRectangleRec({R.x - 6, R.y - 6, R.width + 12, R.height + 12}, GRID);
    DrawRectangleRec(R, BOARD_BG);

    // nền ô: chuỗi thắng, nước vừa đi, ô đang trỏ
    for (Move m : b.winningLine()) DrawRectangleRec(v.cellRect(m), WIN_CELL);
    if (b.lastMove() != NO_MOVE && b.winner() == Cell::Empty) DrawRectangleRec(v.cellRect(b.lastMove()), LAST_MOVE);
    if (hover != NO_MOVE && b.at(hover) == Cell::Empty) DrawRectangleRec(v.cellRect(hover), HOVER);

    // lưới
    for (int i = 0; i <= N; ++i) {
        const float t = i * v.cell;
        DrawLineEx({R.x + t, R.y}, {R.x + t, R.y + R.height}, 1.5f, GRID);
        DrawLineEx({R.x, R.y + t}, {R.x + R.width, R.y + t}, 1.5f, GRID);
    }

    // tọa độ: cột A..I ở trên, hàng 1..9 bên trái
    const float fs = 20;
    for (int i = 0; i < N; ++i) {
        const std::string col(1, char('A' + i)), row = std::to_string(i + 1);
        Vector2 sc = MeasureTextEx(font, col.c_str(), fs, 0), sr = MeasureTextEx(font, row.c_str(), fs, 0);
        DrawTextEx(font, col.c_str(), {R.x + (i + 0.5f) * v.cell - sc.x / 2, R.y - 12 - sc.y}, fs, 0, DARKGRAY);
        DrawTextEx(font, row.c_str(), {R.x - 14 - sr.x, R.y + (i + 0.5f) * v.cell - sr.y / 2}, fs, 0, DARKGRAY);
    }

    // quân cờ: X = dấu nhân đỏ, O = vòng tròn xanh
    const float s = v.cell * 0.30f, th = v.cell * 0.11f;
    for (Move m = 0; m < CELLS; ++m) {
        const Vector2 c = v.center(m);
        if (b.at(m) == Cell::X) {
            DrawLineEx({c.x - s, c.y - s}, {c.x + s, c.y + s}, th, X_COLOR);
            DrawLineEx({c.x - s, c.y + s}, {c.x + s, c.y - s}, th, X_COLOR);
        } else if (b.at(m) == Cell::O) {
            DrawRing(c, s - th / 2, s + th / 2, 0, 360, 48, O_COLOR);
        }
    }

    // gạch chuỗi thắng
    const std::vector<Move> line = b.winningLine();
    if (!line.empty()) DrawLineEx(v.center(line.front()), v.center(line.back()), 6, Fade(DARKGREEN, 0.8f));
}
