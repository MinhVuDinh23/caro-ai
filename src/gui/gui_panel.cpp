// Phần vẽ của GuiApp: thanh trạng thái, bảng điều khiển, thống kê, nhật ký.
#include <algorithm>
#include <cmath>

#include "gui_app.h"
#include "raygui.h"

using namespace caro;

namespace {
const int ITER_CHOICES[] = {100, 300, 500, 1000, 2000, 5000, 10000};
const char* ITER_TEXT = "100;300;500;1000;2000;5000;10000";
const Color X_COLOR = {200, 40, 40, 255};
const Color O_COLOR = {30, 80, 190, 255};
const Color PANEL_TEXT = {50, 50, 50, 255};
const float PX = 750, PW = 410;  // vị trí và bề rộng bảng điều khiển

std::string fmt(const char* f, double v) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), f, v);
    return buf;
}
}  // namespace

void GuiApp::text(const std::string& s, float x, float y, float size, Color col) const {
    DrawTextEx(font_, s.c_str(), {x, y}, size, 0, col);
}

void GuiApp::draw() {
    ClearBackground({245, 242, 235, 255});
    Move hover = NO_MOVE;
    const bool canClick = setup_ || (!thinking() && !board_.isTerminal() && isHuman(board_.toMove()));
    if (canClick) hover = view_.cellAt(GetMousePosition());
    drawBoard(view_, board_, font_, hover);
    drawStatus();
    drawPanel();
}

void GuiApp::drawStatus() {
    std::string s;
    Color col = PANEL_TEXT;
    const Cell me = board_.toMove();
    if (setup_) {
        s = std::string("Xếp thế cờ - lượt đi sau khi xếp: ") + (setupToMove_ == 0 ? "X" : "O");
    } else if (board_.isTerminal()) {
        if (board_.winner() == Cell::Empty) {
            s = "HÒA! Bàn cờ đã đầy";
        } else {
            s = std::string(1, cellChar(board_.winner())) + " THẮNG! (" + playerName(board_.winner()) + ")";
            col = board_.winner() == Cell::X ? X_COLOR : O_COLOR;
        }
    } else {
        col = me == Cell::X ? X_COLOR : O_COLOR;
        s = std::string("Lượt ") + cellChar(me) + ": " + playerName(me);
        if (thinking()) s += fmt(" đang suy nghĩ... %.1f s", GetTime() - thinkStart_);
        else if (isHuman(me)) s += " - click vào ô trống";
        else if (!isHuman(opponent(me)) && !autoRun_) s += " - bấm Bước hoặc Tự chạy";
    }
    // Hết ván: chữ lớn, có khung (không che bàn cờ)
    if (board_.isTerminal() && !setup_) {
        const Vector2 sz = MeasureTextEx(font_, s.c_str(), 30, 0);
        DrawRectangleRounded({14, 6, sz.x + 20, 40}, 0.3f, 8, Fade(col, 0.12f));
        text(s, 24, 9, 30, col);
    } else {
        text(s, 20, 12, 24, col);
    }
}

void GuiApp::drawPanel() {
    float y = 12;
    text("Người chơi", PX, y, 20, PANEL_TEXT);
    const float tw = (PW - 34 - 3 * 4) / 4;
    for (int i = 0; i < 2; ++i) {
        y += 34;
        const Cell c = i == 0 ? Cell::X : Cell::O;
        text(i == 0 ? "X" : "O", PX + 4, y + 2, 24, c == Cell::X ? X_COLOR : O_COLOR);
        GuiToggleGroup({PX + 34, y, tw, 30}, "Người;Random;MiniMax;MCTS", &sides_[i].type);
    }

    y += 44;
    GuiToggleGroup({PX, y, (PW - 4) / 2, 28}, "Tham số bên X;Tham số bên O", &editSide_);
    drawSideParams(PX, y + 38, PW);

    // Hàng nút 1: ván mới, đi lại, xếp thế cờ (khóa khi máy đang nghĩ)
    y = 300;
    const float bw = (PW - 16) / 3;
    if (thinking()) GuiDisable();
    if (GuiButton({PX, y, bw, 32}, "Ván mới")) newGame();
    if (setup_) GuiDisable();
    if (GuiButton({PX + bw + 8, y, bw, 32}, "Đi lại")) undo();
    if (!thinking()) GuiEnable();
    if (GuiButton({PX + 2 * (bw + 8), y, bw, 32}, setup_ ? "Xếp xong" : "Xếp thế cờ")) toggleSetup();

    // Hàng nút 2: Máy - Máy (bước / tự chạy / độ trễ) hoặc công cụ xếp thế cờ
    y += 40;
    if (setup_) {
        text("Lượt đi:", PX, y + 5, 18, PANEL_TEXT);
        GuiToggleGroup({PX + 80, y, 60, 32}, "X;O", &setupToMove_);
        if (GuiButton({PX + 2 * (bw + 8), y, bw, 32}, "Xóa bàn")) {
            board_ = Board();
            addLog("Đã xóa bàn");
        }
    } else {
        const bool aiTurn = !board_.isTerminal() && !isHuman(board_.toMove());
        if (!aiTurn || thinking()) GuiDisable();
        if (GuiButton({PX, y, bw, 32}, "Bước")) stepRequested_ = true;
        GuiEnable();
        if (!aiTurn && !autoRun_) GuiDisable();
        GuiToggle({PX + bw + 8, y, bw, 32}, autoRun_ ? "Dừng" : "Tự chạy", &autoRun_);
        GuiEnable();
        // độ trễ giữa các nước khi tự chạy
        const float sx = PX + 2 * (bw + 8);
        text(fmt("Trễ %.0f ms", delayMs_), sx, y - 2, 13, GRAY);
        GuiSliderBar({sx, y + 16, bw, 16}, nullptr, nullptr, &delayMs_, 0, 1500);
    }
    GuiEnable();

    drawStats({PX, 392, PW, 196});
    drawLog({PX, 598, PW, 130});
}

void GuiApp::drawSideParams(float x, float y, float w) {
    SideConfig& s = sides_[editSide_];
    const float lw = 190, cw = w - lw, rh = 33;  // cột nhãn, cột điều khiển, chiều cao hàng
    switch (s.type) {
        case PT_HUMAN:
            text("Người chơi: click vào ô trống để đánh.", x, y + 4, 17, PANEL_TEXT);
            break;
        case PT_RANDOM:
            text("Random: chọn đều ngẫu nhiên một ô trống.", x, y + 4, 17, PANEL_TEXT);
            text(fmt("seed = %.0f + số quân trên bàn", seed_), x, y + 4 + rh, 15, GRAY);
            break;
        case PT_MINIMAX:
            text("Độ sâu d", x, y + 4, 17, PANEL_TEXT);
            if (GuiSpinner({x + lw, y, cw, 28}, nullptr, &s.mm.depth, 1, 6, editDepth_)) editDepth_ = !editDepth_;
            text("Top-k ứng viên (0 = tất cả)", x, y + rh + 4, 17, PANEL_TEXT);
            if (GuiSpinner({x + lw, y + rh, cw, 28}, nullptr, &s.mm.topK, 0, 40, editTopK_)) editTopK_ = !editTopK_;
            GuiCheckBox({x, y + 2 * rh + 3, 22, 22}, "Cắt tỉa Alpha-Beta", &s.mm.alphaBeta);
            if (!s.mm.alphaBeta && s.mm.depth >= 4)
                text("Không cắt tỉa + d >= 4: có thể rất chậm", x, y + 3 * rh + 2, 15, RED);
            break;
        case PT_MCTS:
            text("Số iteration", x, y + 4, 17, PANEL_TEXT);
            GuiComboBox({x + lw, y, cw, 28}, ITER_TEXT, &s.iterIdx);
            s.mc.iterations = ITER_CHOICES[std::clamp(s.iterIdx, 0, 6)];
            text(fmt("Hằng số c = %.1f", s.c), x, y + rh + 4, 17, PANEL_TEXT);
            GuiSliderBar({x + lw, y + rh + 4, cw, 20}, nullptr, nullptr, &s.c, 0.1f, 3.0f);
            s.c = std::round(s.c * 10.0f) / 10.0f;  // bước 0.1 để chọn đúng 0.5 / 1.0 / 1.4 / 2.0
            s.mc.c = s.c;
            text("Rollout", x, y + 2 * rh + 4, 17, PANEL_TEXT);
            GuiToggleGroup({x + lw, y + 2 * rh, (cw - 4) / 2, 28}, "Ngẫu nhiên;Heuristic", &s.rolloutIdx);
            s.mc.rollout = s.rolloutIdx == 0 ? RolloutPolicy::Random : RolloutPolicy::Heuristic;
            GuiCheckBox({x, y + 3 * rh + 3, 22, 22}, "Nước bắt buộc (thắng ngay / chặn 4)", &s.mc.forcedMoves);
            break;
        default:
            break;
    }
}

void GuiApp::drawStats(Rectangle r) {
    GuiGroupBox(r, "Thống kê nước máy vừa đi");
    if (!hasStats_) {
        text("(chưa có)", r.x + 10, r.y + 14, 16, GRAY);
        return;
    }
    const SearchStats& s = lastStats_;
    const float x = r.x + 10, fs = 16, lh = 19;
    float y = r.y + 12;
    auto line = [&](const std::string& t, Color c = PANEL_TEXT) {
        text(t, x, y, fs, c);
        y += lh;
    };
    line(std::string(1, cellChar(lastSide_)) + ": " + lastName_, lastSide_ == Cell::X ? X_COLOR : O_COLOR);
    line("Nước đi: " + moveToString(s.move) + fmt("     Thời gian: %.2f ms", s.ms));
    if (s.iterations > 0) {  // MCTS
        line("Iteration: " + std::to_string(s.iterations) + "     Số nút trong cây: " + std::to_string(s.nodes));
        line("Top nước (chọn theo N lớn nhất):", GRAY);
        for (const ChildInfo& c : s.topChildren) {
            char buf[96];
            std::snprintf(buf, sizeof(buf), "  %-4s  N = %-6d  W/N = %.3f", moveToString(c.move).c_str(), c.visits, c.meanReward);
            line(buf);
        }
    } else if (s.depth > 0) {  // MiniMax
        line("Độ sâu: " + std::to_string(s.depth) + "     Số nút đã duyệt: " + std::to_string(s.nodes));
        line("Số lần cắt tỉa: " + std::to_string(s.cutoffs));
        line("Giá trị nước chọn: " + std::to_string(s.value));
        if (s.value >= MinimaxAgent::WIN_SCORE) line("=> tìm thấy đường thắng chắc chắn", DARKGREEN);
        else if (s.value <= -MinimaxAgent::WIN_SCORE) line("=> đối thủ có đường thắng chắc chắn", RED);
    }
}

void GuiApp::drawLog(Rectangle r) {
    GuiGroupBox(r, "Nhật ký (lăn chuột để cuộn)");
    const float fs = 15, lh = 18;
    const int visible = static_cast<int>((r.height - 22) / lh);
    const int n = static_cast<int>(log_.size());
    if (CheckCollisionPointRec(GetMousePosition(), r)) logScroll_ += static_cast<int>(GetMouseWheelMove());
    logScroll_ = std::clamp(logScroll_, 0, std::max(0, n - visible));
    const int first = std::max(0, n - visible - logScroll_);
    BeginScissorMode(static_cast<int>(r.x + 4), static_cast<int>(r.y + 10), static_cast<int>(r.width - 8),
                     static_cast<int>(r.height - 14));
    for (int i = first; i < n && i < first + visible; ++i) {
        const std::string& s = log_[i];
        const Color c = s.rfind("KẾT QUẢ", 0) == 0 ? DARKGREEN : (s.rfind("Lỗi", 0) == 0 ? RED : PANEL_TEXT);
        text(s, r.x + 8, r.y + 12 + (i - first) * lh, fs, c);
    }
    EndScissorMode();
}
