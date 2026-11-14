#include "gui_app.h"

#include <chrono>
#include <exception>

#include "log_format.h"
#include "random_agent.h"

using namespace caro;

GuiApp::GuiApp(const Font& font, const AppOptions& opt) : font_(font) {
    view_.x = 60;
    view_.y = 86;
    view_.cell = 70;
    sides_[0].type = opt.xType;
    sides_[1].type = opt.oType;
    for (SideConfig& s : sides_) {
        s.mm = MinimaxConfig{3, true, 12, 2};
        s.mc.iterations = 2000;
        s.mc.c = 1.4;
        s.mc.forcedMoves = true;  // GUI: bật nước bắt buộc để máy không bỏ lỡ thắng/chặn 4
    }
    autoRun_ = opt.autoRun;
    delayMs_ = static_cast<float>(opt.delayMs);
    seed_ = opt.seed;
    if (!opt.logFile.empty()) logFile_ = std::fopen(opt.logFile.c_str(), "w");
    addLog("Ván mới: X = " + playerName(Cell::X) + ", O = " + playerName(Cell::O));
}

GuiApp::~GuiApp() {
    if (future_.valid()) future_.wait();  // đợi luồng AI xong rồi mới thoát
    if (logFile_) std::fclose(logFile_);
}

std::unique_ptr<Agent> GuiApp::makeAgent(Cell c) const {
    const SideConfig& s = side(c);
    const uint32_t seed = seed_ + static_cast<uint32_t>(board_.stoneCount());  // tái lập được
    switch (s.type) {
        case PT_RANDOM:
            return std::make_unique<RandomAgent>(seed);
        case PT_MINIMAX:
            return std::make_unique<MinimaxAgent>(s.mm);
        case PT_MCTS: {
            MctsConfig m = s.mc;
            m.seed = seed;
            return std::make_unique<MctsAgent>(m);
        }
        default:
            return nullptr;
    }
}

std::string GuiApp::playerName(Cell c) const {
    if (isHuman(c)) return "Người";
    return makeAgent(c)->name();
}

void GuiApp::addLog(const std::string& line, const std::string& full) {
    log_.push_back(line);
    logScroll_ = 0;
    const std::string& out = full.empty() ? line : full;
    std::printf("%s\n", out.c_str());
    std::fflush(stdout);
    if (logFile_) {
        std::fprintf(logFile_, "%s\n", out.c_str());
        std::fflush(logFile_);
    }
}

// ---------------------------------------------------------------- AI

void GuiApp::startAi() {
    const Cell me = board_.toMove();
    std::unique_ptr<Agent> agent = makeAgent(me);
    if (!agent) return;
    thinkStart_ = GetTime();
    // Luồng phụ chỉ dùng bản sao bàn cờ `b` và tác nhân riêng `a`.
    future_ = std::async(std::launch::async, [b = board_, a = std::move(agent), me]() {
        AiResult r;
        r.side = me;
        r.name = a->name();
        r.move = a->choose(b, r.stats);
        return r;
    });
}

void GuiApp::pollAi() {
    if (!future_.valid() || future_.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
    AiResult r;
    try {
        r = future_.get();
    } catch (const std::exception& e) {
        addLog(std::string("Lỗi AI: ") + e.what());
        autoRun_ = false;
        return;
    }
    if (r.side != board_.toMove() || !board_.play(r.move)) {
        addLog("Lỗi: AI trả về nước không hợp lệ " + moveToString(r.move));
        autoRun_ = false;
        return;
    }
    lastStats_ = r.stats;
    lastName_ = r.name;
    lastSide_ = r.side;
    hasStats_ = true;
    char buf[160];
    std::snprintf(buf, sizeof(buf), "#%d %c -> %s  %s  %.1f ms", board_.stoneCount(), cellChar(r.side),
                  moveToString(r.move).c_str(), r.name.c_str(), r.stats.ms);
    addLog(buf, formatMoveLog(r.name, r.side, r.stats));
    afterMove();
}

// ---------------------------------------------------------------- vòng cập nhật

void GuiApp::update() {
    pollAi();
    if (thinking()) return;
    if (setup_) {
        setupClick();
        return;
    }
    if (board_.isTerminal()) return;

    const Cell me = board_.toMove();
    if (isHuman(me)) {
        humanClick();
        return;
    }
    const bool vsHuman = isHuman(opponent(me));  // Người - Máy: máy đi ngay
    const bool due = GetTime() - lastMoveTime_ >= delayMs_ / 1000.0;
    if (vsHuman || stepRequested_ || (autoRun_ && due)) {
        stepRequested_ = false;
        startAi();
    }
}

void GuiApp::humanClick() {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    const Move m = view_.cellAt(GetMousePosition());
    if (m == NO_MOVE) return;
    if (!board_.isLegal(m)) {
        addLog("Ô " + moveToString(m) + " đã có quân, hãy chọn ô trống");
        return;
    }
    const Cell me = board_.toMove();
    board_.play(m);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "#%d %c -> %s  Người", board_.stoneCount(), cellChar(me), moveToString(m).c_str());
    addLog(buf);
    afterMove();
}

void GuiApp::afterMove() {
    lastMoveTime_ = GetTime();
    if (!board_.isTerminal() || resultLogged_) return;
    resultLogged_ = true;
    autoRun_ = false;
    if (board_.winner() == Cell::Empty) {
        addLog("KẾT QUẢ: HÒA (bàn cờ đã đầy)");
    } else {
        const Cell w = board_.winner();
        addLog(std::string("KẾT QUẢ: ") + cellChar(w) + " THẮNG (" + playerName(w) + ")");
    }
}

// ---------------------------------------------------------------- nút điều khiển

void GuiApp::newGame() {
    board_ = Board();
    setup_ = false;
    stepRequested_ = false;
    resultLogged_ = false;
    hasStats_ = false;
    lastMoveTime_ = GetTime();
    addLog("--- Ván mới: X = " + playerName(Cell::X) + ", O = " + playerName(Cell::O));
}

void GuiApp::undo() {
    if (board_.history().empty()) {
        addLog("Không còn nước để đi lại");
        return;
    }
    board_.undo();
    // Người - Máy: lùi thêm một nước để quay về lượt của người.
    const Cell me = board_.toMove();
    if (!isHuman(me) && isHuman(opponent(me)) && !board_.history().empty()) board_.undo();
    autoRun_ = false;
    resultLogged_ = false;
    hasStats_ = false;
    lastMoveTime_ = GetTime();
    addLog("Đi lại -> còn " + std::to_string(board_.stoneCount()) + " quân, lượt " + cellChar(board_.toMove()));
}

void GuiApp::toggleSetup() {
    if (!setup_) {
        setup_ = true;
        autoRun_ = false;
        setupToMove_ = board_.toMove() == Cell::X ? 0 : 1;
        addLog("Xếp thế cờ: click trái đổi ô (trống -> X -> O), click phải xóa");
        return;
    }
    setup_ = false;
    board_.setToMove(setupToMove_ == 0 ? Cell::X : Cell::O);
    resultLogged_ = false;
    hasStats_ = false;
    lastMoveTime_ = GetTime();
    int x = 0, o = 0;
    for (Move m = 0; m < CELLS; ++m) {
        if (board_.at(m) == Cell::X) ++x;
        if (board_.at(m) == Cell::O) ++o;
    }
    addLog("Xếp xong: X=" + std::to_string(x) + ", O=" + std::to_string(o) + " quân, lượt " + cellChar(board_.toMove()),
           "Xếp xong thế cờ, lượt " + std::string(1, cellChar(board_.toMove())) + ":\n" + board_.toString());
    if (x - o > 1 || o - x > 1) addLog("Lưu ý: số quân hai bên chênh lệch > 1");
    afterMove();  // thế cờ đã có người thắng / bàn đầy -> thông báo ngay
}

void GuiApp::setupClick() {
    const Move m = view_.cellAt(GetMousePosition());
    if (m == NO_MOVE) return;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        const Cell cur = board_.at(m);
        board_.setCell(m, cur == Cell::Empty ? Cell::X : (cur == Cell::X ? Cell::O : Cell::Empty));
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        board_.setCell(m, Cell::Empty);
    }
}
