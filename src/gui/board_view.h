// Vẽ bàn cờ 9x9 và đổi tọa độ chuột -> ô cờ.
#pragma once
#include "board.h"
#include "raylib.h"

struct BoardView {
    float x = 0, y = 0;  // góc trên trái của lưới
    float cell = 70;     // cạnh một ô (px)

    Rectangle rect() const { return {x, y, cell * caro::N, cell * caro::N}; }
    Rectangle cellRect(caro::Move m) const;
    Vector2 center(caro::Move m) const;
    // Ô dưới điểm p, NO_MOVE nếu p nằm ngoài bàn.
    caro::Move cellAt(Vector2 p) const;
};

// hover: ô đang trỏ chuột (NO_MOVE nếu không có). Tô ô vừa đi và chuỗi thắng.
void drawBoard(const BoardView& v, const caro::Board& b, const Font& font, caro::Move hover);
