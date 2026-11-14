// Trạng thái và logic của giao diện: bàn cờ, cấu hình hai bên, luồng AI, nhật ký.
//
// - AI chạy trên luồng phụ (std::async) với BẢN SAO bàn cờ và cấu hình -> giao diện không bị đứng,
//   luồng phụ không bao giờ chạm vào trạng thái của GUI.
// - Người - Máy: máy tự đi ngay sau người. Máy - Máy: bấm "Bước" (1 nước) hoặc "Tự chạy".
// - Đổi tham số có hiệu lực từ nước tiếp theo (không ngắt lần tìm kiếm đang chạy).
#pragma once
#include <cstdint>
#include <cstdio>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include "agent.h"
#include "board_view.h"
#include "mcts_agent.h"
#include "minimax_agent.h"
#include "raylib.h"

// Thứ tự khớp với nhóm nút chọn trên GUI: "Người;Random;MiniMax;MCTS".
enum PlayerType : int { PT_HUMAN = 0, PT_RANDOM, PT_MINIMAX, PT_MCTS };

struct SideConfig {
    int type = PT_HUMAN;
    caro::MinimaxConfig mm;  // tham số khi type = MiniMax
    caro::MctsConfig mc;     // tham số khi type = MCTS
    int iterIdx = 4;         // chỉ số trong danh sách số iteration
    int rolloutIdx = 0;      // 0 = ngẫu nhiên, 1 = heuristic
    float c = 1.4f;          // giá trị thanh trượt c
};

struct AppOptions {
    int xType = PT_HUMAN, oType = PT_MINIMAX;
    bool autoRun = false;
    int delayMs = 400;       // trễ giữa hai nước khi Máy - Máy tự chạy
    uint32_t seed = 2026;    // seed gốc cho Random/MCTS (seed thật = seed + số quân trên bàn)
    std::string logFile;     // ghi thêm nhật ký ra file (tùy chọn)
};

class GuiApp {
public:
    static constexpr int WIDTH = 1180, HEIGHT = 740;

    GuiApp(const Font& font, const AppOptions& opt);
    ~GuiApp();
    void update();  // nhận kết quả AI, khởi động AI, xử lý click trên bàn cờ
    void draw();    // vẽ; các nút raygui xử lý thao tác ngay khi được vẽ

private:
    struct AiResult {
        caro::Move move = caro::NO_MOVE;
        caro::SearchStats stats;
        std::string name;
        caro::Cell side = caro::Cell::Empty;
    };

    SideConfig& side(caro::Cell c) { return sides_[c == caro::Cell::X ? 0 : 1]; }
    const SideConfig& side(caro::Cell c) const { return sides_[c == caro::Cell::X ? 0 : 1]; }
    bool isHuman(caro::Cell c) const { return side(c).type == PT_HUMAN; }
    bool thinking() const { return future_.valid(); }
    std::unique_ptr<caro::Agent> makeAgent(caro::Cell c) const;
    std::string playerName(caro::Cell c) const;

    // logic (gui_app.cpp)
    void startAi();
    void pollAi();
    void humanClick();
    void setupClick();
    void afterMove();
    void newGame();
    void undo();
    void toggleSetup();
    void addLog(const std::string& line, const std::string& full = "");

    // vẽ (gui_panel.cpp)
    void drawStatus();
    void drawPanel();
    void drawSideParams(float x, float y, float w);
    void drawStats(Rectangle r);
    void drawLog(Rectangle r);
    void text(const std::string& s, float x, float y, float size, Color col) const;

    Font font_;
    BoardView view_;
    caro::Board board_;
    SideConfig sides_[2];
    int editSide_ = 1;  // đang sửa tham số bên nào (0 = X, 1 = O)
    bool autoRun_ = false;
    float delayMs_ = 400;
    bool stepRequested_ = false;
    bool setup_ = false;    // chế độ xếp thế cờ
    int setupToMove_ = 0;   // lượt đi sau khi xếp (0 = X, 1 = O)
    bool resultLogged_ = false;
    double lastMoveTime_ = 0, thinkStart_ = 0;
    uint32_t seed_ = 2026;

    std::future<AiResult> future_;
    caro::SearchStats lastStats_;
    std::string lastName_;
    caro::Cell lastSide_ = caro::Cell::Empty;
    bool hasStats_ = false;

    std::vector<std::string> log_;
    int logScroll_ = 0;  // số dòng đã cuộn lên khỏi cuối
    FILE* logFile_ = nullptr;
    bool editDepth_ = false, editTopK_ = false;  // trạng thái gõ số của các ô spinner
};
