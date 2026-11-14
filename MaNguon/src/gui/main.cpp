// Phase 1: test window - checks raylib + Vietnamese font.
#include "raylib.h"

int main(int argc, char** argv) {
    const bool shot = argc > 1;  // test mode: take a screenshot and exit
    InitWindow(1400, 450, "Cờ Caro 9x9");
    SetTargetFPS(60);

    const char* sample = "Cờ Caro 9×9 — Người đấu Máy: ắ ằ ẳ ẵ ặ ế ề ể ễ ệ ố ồ ổ ỗ ộ ớ ờ ở ỡ ợ ứ ừ ử ữ ự đ Đ";
    int count = 0;
    int* cps = LoadCodepoints(sample, &count);
    Font font = LoadFontEx("assets/fonts/NotoSans-Regular.ttf", 32, cps, count);
    UnloadCodepoints(cps);

    int frame = 0;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTextEx(font, sample, {20, 200}, 32, 1, DARKGRAY);
        EndDrawing();
        if (shot && ++frame == 5) { TakeScreenshot(argv[1]); break; }
    }
    UnloadFont(font);
    CloseWindow();
    return 0;
}
