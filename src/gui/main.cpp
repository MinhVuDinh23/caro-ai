// Chương trình giao diện Cờ Caro 9x9: Người - Máy, Máy - Máy (MiniMax / MCTS / Random).
//
// Tùy chọn dòng lệnh (đều không bắt buộc):
//   --x <human|random|minimax|mcts>   người chơi bên X (mặc định human)
//   --o <human|random|minimax|mcts>   người chơi bên O (mặc định minimax)
//   --auto                            Máy - Máy tự chạy ngay khi mở
//   --delay <ms>                      trễ giữa các nước khi tự chạy (mặc định 400)
//   --seed <n>                        seed gốc cho Random / MCTS (mặc định 2026)
//   --log <file>                      ghi thêm nhật ký ra file
//   --shot <file.png> [--frames n]    chụp màn hình sau n khung hình rồi thoát (kiểm thử)
#include <cstdlib>
#include <cstring>
#include <string>

#include "gui_app.h"
#include "raygui.h"
#include "raylib.h"
#include "rlgl.h"
#include "vn_font.h"

#if defined(_WIN32)
// Khai báo trực tiếp để không phải include <windows.h> (trùng tên với raylib).
extern "C" __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int);
#endif

namespace {
int parseType(const char* s) {
    if (!std::strcmp(s, "random")) return PT_RANDOM;
    if (!std::strcmp(s, "minimax")) return PT_MINIMAX;
    if (!std::strcmp(s, "mcts")) return PT_MCTS;
    return PT_HUMAN;
}

// TakeScreenshot of raylib 5.5 multiplies by the DPI scale a second time when HIGHDPI is on -> read the framebuffer directly.
void saveScreenshot(const char* file) {
    const int w = GetRenderWidth(), h = GetRenderHeight();
    unsigned char* px = rlReadScreenPixels(w, h);
    Image img = {px, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    ExportImage(img, file);
    RL_FREE(px);
}
}  // namespace

int main(int argc, char** argv) {
#if defined(_WIN32)
    SetConsoleOutputCP(65001);  // console hiển thị nhật ký tiếng Việt (UTF-8)
#endif
    AppOptions opt;
    std::string shot;
    int frames = 30;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        const bool hasNext = i + 1 < argc;
        if (a == "--x" && hasNext) opt.xType = parseType(argv[++i]);
        else if (a == "--o" && hasNext) opt.oType = parseType(argv[++i]);
        else if (a == "--auto") opt.autoRun = true;
        else if (a == "--delay" && hasNext) opt.delayMs = std::atoi(argv[++i]);
        else if (a == "--seed" && hasNext) opt.seed = static_cast<uint32_t>(std::strtoul(argv[++i], nullptr, 10));
        else if (a == "--log" && hasNext) opt.logFile = argv[++i];
        else if (a == "--shot" && hasNext) shot = argv[++i];
        else if (a == "--frames" && hasNext) frames = std::atoi(argv[++i]);
    }

    // HIGHDPI: Windows phóng to 150-200% vẫn vẽ đúng kích thước, chữ sắc nét.
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(GuiApp::WIDTH, GuiApp::HEIGHT, "Cờ Caro 9x9 - MiniMax & MCTS");
    SetExitKey(KEY_NULL);  // không thoát nhầm khi bấm Esc lúc demo
    SetTargetFPS(60);

    Font font = loadVietnameseFont("assets/fonts/NotoSans-Regular.ttf", 48);
    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 17);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 0);

    {
        GuiApp app(font, opt);
        int frame = 0;
        while (!WindowShouldClose()) {
            app.update();
            BeginDrawing();
            app.draw();
            const bool takeShot = !shot.empty() && ++frame >= frames;
            if (takeShot) saveScreenshot(shot.c_str());  // read before swapping buffers
            EndDrawing();
            if (takeShot) break;
        }
    }  // app hủy trước: đợi luồng AI (nếu đang chạy) rồi mới đóng cửa sổ

    UnloadFont(font);
    CloseWindow();
    return 0;
}
