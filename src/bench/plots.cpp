#include "plots.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <map>
#include <regex>
#include <vector>

#include "csv.h"
#include "svg_chart.h"

namespace bench {

namespace {
const double NaN = std::numeric_limits<double>::quiet_NaN();

double d(const CsvRow& r, const char* k) {
    auto it = r.find(k);
    return it == r.end() || it->second.empty() ? NaN : std::stod(it->second);
}
std::string s(const CsvRow& r, const char* k) {
    auto it = r.find(k);
    return it == r.end() ? "" : it->second;
}

// Các giá trị phân biệt của cột `key`, giữ thứ tự xuất hiện.
std::vector<std::string> distinct(const std::vector<CsvRow>& rows, const char* key) {
    std::vector<std::string> out;
    for (const CsvRow& r : rows)
        if (std::find(out.begin(), out.end(), s(r, key)) == out.end()) out.push_back(s(r, key));
    return out;
}

const CsvRow* findRow(const std::vector<CsvRow>& rows, const std::string& group, const std::string& value) {
    for (const CsvRow& r : rows)
        if (s(r, "group") == group && s(r, "value") == value) return &r;
    return nullptr;
}

// "MiniMax(d=2,AB,k=12)" -> "MiniMax d=2";  "MCTS(it=500,c=1.40,heur,fm)" -> "MCTS 500 heur+fm"
std::string shortLabel(const std::string& l) {
    std::smatch m;
    if (std::regex_match(l, m, std::regex(R"(MiniMax\(d=(\d+),(AB|thuần|plain)?.*\))"))) return "MiniMax d=" + m[1].str();
    if (std::regex_match(l, m, std::regex(R"(MCTS\(it=(\d+),c=([\d.]+),(\w+)(,fm)?\))"))) {
        std::string out = "MCTS " + m[1].str() + " " + m[3].str();
        if (m[2].str() != "1.40") out += " c=" + m[2].str();
        if (m[4].matched) out += "+fm";
        return out;
    }
    return l;
}

std::string pct(double v) { return fmtNum(v * 100, 1) + "%"; }

std::string wdl(const CsvRow& r) { return s(r, "win") + "-" + s(r, "draw") + "-" + s(r, "loss"); }

// Bảng markdown chung cho các trận đấu
void matchTable(FILE* f, const std::vector<CsvRow>& rows, bool withParam) {
    std::fprintf(f, "| Nhóm | Tác nhân A | Đối thủ B |%s Ván | T-H-B | Tỉ lệ thắng | Điểm | ±KTC 95%% | ms/nước A | ms/nước B | Số nước TB |\n",
                 withParam ? " Tham số |" : "");
    std::fprintf(f, "|---|---|---|%s---|---|---|---|---|---|---|---|\n", withParam ? "---|" : "");
    for (const CsvRow& r : rows) {
        // khoảng tin cậy 95% xấp xỉ chuẩn của điểm: 1.96 * sqrt(p(1-p)/n)
        const double p = d(r, "score"), n = d(r, "games");
        const std::string ci = fmtNum(1.96 * std::sqrt(p * (1 - p) / n), 3);
        std::fprintf(f, "| %s | %s | %s |%s %s | %s | %s | %s | %s | %s | %s | %s |\n", s(r, "group").c_str(),
                     s(r, "agent_a").c_str(), s(r, "agent_b").c_str(),
                     withParam ? (" " + s(r, "param") + "=" + s(r, "value") + " |").c_str() : "", s(r, "games").c_str(),
                     wdl(r).c_str(), pct(d(r, "win_rate")).c_str(), fmtNum(p, 3).c_str(), ci.c_str(),
                     fmtNum(d(r, "avg_ms_a"), 2).c_str(), fmtNum(d(r, "avg_ms_b"), 2).c_str(),
                     fmtNum(d(r, "avg_plies"), 1).c_str());
    }
    std::fprintf(f, "\n");
}

// ---------------------------------------------------------------- E1: bản đồ nhiệt đấu vòng tròn
bool plotCross(const std::string& dir) {
    const std::vector<CsvRow> rows = readCsv(dir + "/cross.csv");
    if (rows.empty()) return false;
    std::vector<std::string> agents;
    for (const CsvRow& r : rows)
        for (const char* k : {"agent_a", "agent_b"})
            if (std::find(agents.begin(), agents.end(), s(r, k)) == agents.end()) agents.push_back(s(r, k));
    const size_t n = agents.size();
    std::vector<std::vector<double>> m(n, std::vector<double>(n, NaN));
    auto idx = [&](const std::string& a) { return static_cast<size_t>(std::find(agents.begin(), agents.end(), a) - agents.begin()); };
    for (const CsvRow& r : rows) {
        const size_t i = idx(s(r, "agent_a")), j = idx(s(r, "agent_b"));
        m[i][j] = d(r, "score");
        m[j][i] = 1 - d(r, "score");
    }
    std::vector<std::string> labels;
    for (const std::string& a : agents) labels.push_back(shortLabel(a));
    return saveHeatmap(dir + "/cross_heatmap.svg", "E1 - Đấu vòng tròn: điểm của tác nhân hàng khi gặp tác nhân cột",
                       "Điểm = (thắng + 0.5·hòa) / số ván; " + s(rows[0], "games") +
                           " ván mỗi cặp, đổi người đi trước theo cặp; seed " + s(rows[0], "seed"),
                       labels, m, "Xanh: hàng mạnh hơn, đỏ: hàng yếu hơn, xám: ngang nhau. fm = có nước bắt buộc.");
}

// ---------------------------------------------------------------- E2: quét c (cột chồng T/H/B)
bool plotCsweep(const std::string& dir) {
    const std::vector<CsvRow> rows = readCsv(dir + "/csweep.csv");
    if (rows.empty()) return false;
    Figure fig;
    fig.title = "E2 - Quét hằng số khám phá c của MCTS (rollout ngẫu nhiên)";
    fig.subtitle = "Tỉ lệ Thắng / Hòa / Thua của MCTS theo c; số trên cột = tỉ lệ thắng; seed " + s(rows[0], "seed");
    fig.cols = 3;
    fig.panelW = 330;
    fig.panelH = 290;
    for (const std::string& g : distinct(rows, "group")) {
        Panel p;
        p.kind = Panel::StackedBars;
        p.percent = true;
        p.valueLabels = true;
        p.xLabel = "hằng số c";
        Series w{"Thắng", color::WIN, {}}, dr{"Hòa", color::DRAW, {}}, l{"Thua", color::LOSS, {}};
        std::string games, it;
        for (const CsvRow& r : rows) {
            if (s(r, "group") != g) continue;
            p.cats.push_back(s(r, "value"));
            const double G = d(r, "games");
            w.values.push_back(d(r, "win") / G);
            dr.values.push_back(d(r, "draw") / G);
            l.values.push_back(d(r, "loss") / G);
            games = s(r, "games");
            std::smatch m;
            const std::string a = s(r, "agent_a");
            if (std::regex_search(a, m, std::regex(R"(it=(\d+))"))) it = m[1].str();
        }
        p.title = g.substr(0, g.find(" (")) + "  (it=" + it + ", " + games + " ván)";
        p.series = {w, dr, l};
        fig.panels.push_back(p);
    }
    fig.note = "Theo đề: c ∈ {0.5, 1.0, 1.4, 2.0} ở số iteration cố định 300. Khung cuối: quét bổ sung ở 10000 iteration.";
    return saveFigure(dir + "/csweep.svg", fig);
}

// ---------------------------------------------------------------- E3: quét số iteration
bool plotItersweep(const std::string& dir) {
    const std::vector<CsvRow> rows = readCsv(dir + "/itersweep.csv");
    if (rows.empty()) return false;
    Figure fig;
    fig.title = "E3 - Quét số iteration của MCTS (c = 1.4, rollout ngẫu nhiên)";
    fig.subtitle = "Trái: điểm theo từng đối thủ cố định. Phải: thời gian suy luận trung bình mỗi nước. Seed " + s(rows[0], "seed");
    fig.cols = 2;
    fig.panelW = 470;
    fig.panelH = 310;
    const std::vector<std::string> its = distinct(rows, "value"), groups = distinct(rows, "group");

    Panel score;
    score.kind = Panel::Lines;
    score.title = "Điểm của MCTS (thắng = 1, hòa = 0.5)";
    score.cats = its;
    score.percent = true;
    score.refLine = 0.5;
    score.xLabel = "số iteration";
    for (size_t gi = 0; gi < groups.size(); ++gi) {
        Series se{groups[gi], color::SERIES[gi % 4], {}};
        for (const std::string& it : its) {
            const CsvRow* r = findRow(rows, groups[gi], it);
            se.values.push_back(r ? d(*r, "score") : NaN);
        }
        score.series.push_back(se);
    }
    fig.panels.push_back(score);

    Panel time;
    time.kind = Panel::Lines;
    time.title = "Thời gian trung bình / nước (ms, thang log)";
    time.cats = its;
    time.logY = true;
    time.xLabel = "số iteration";
    Series ms{"ms/nước", color::SERIES[0], {}};
    double lo = 1e9, hi = 0;
    for (const std::string& it : its) {
        double sum = 0;
        int cnt = 0;
        for (const CsvRow& r : rows)
            if (s(r, "value") == it) sum += d(r, "avg_ms_a"), ++cnt;
        const double v = cnt ? sum / cnt : NaN;
        ms.values.push_back(v);
        lo = std::min(lo, v);
        hi = std::max(hi, v);
    }
    time.series = {ms};
    time.valueLabels = true;
    time.yMin = std::pow(10, std::floor(std::log10(lo)));
    time.yMax = std::pow(10, std::ceil(std::log10(hi)));
    fig.panels.push_back(time);
    fig.note = "Theo đề: iteration ∈ {100, 300, 500}; 1000-10000 là phần mở rộng. Đường đứt = 50% (ngang tay).";
    return saveFigure(dir + "/itersweep.svg", fig);
}

// ---------------------------------------------------------------- E4: số nút AB vs không cắt tỉa
bool plotAb(const std::string& dir) {
    const std::vector<CsvRow> rows = readCsv(dir + "/ab.csv");
    if (rows.empty()) return false;
    Figure fig;
    fig.title = "E4 - MiniMax có và không cắt tỉa Alpha-Beta: số nút đã duyệt";
    fig.subtitle = "8 thế cờ cố định (P1-P8), thang log; cùng cây tìm kiếm, cùng nước chọn";
    fig.cols = 2;
    fig.panelW = 470;
    fig.panelH = 280;
    const std::vector<std::string> positions = distinct(rows, "position");
    struct Cfg { const char* d; const char* k; const char* title; };
    for (const Cfg& c : {Cfg{"2", "12", "d = 2, top-k = 12"}, Cfg{"3", "12", "d = 3, top-k = 12"},
                         Cfg{"4", "12", "d = 4, top-k = 12"}, Cfg{"3", "0", "d = 3, không giới hạn ứng viên"}}) {
        Panel p;
        p.kind = Panel::Bars;
        p.logY = true;
        p.title = c.title;
        p.yLabel = "số nút";
        Series ab{"Alpha-Beta", color::SERIES[0], {}}, plain{"Không cắt tỉa", color::SERIES[1], {}};
        double hi = 1;
        for (const std::string& pos : positions) {
            p.cats.push_back(pos.substr(0, pos.find(' ')));
            double vab = NaN, vpl = NaN;
            for (const CsvRow& r : rows) {
                if (s(r, "position") != pos || s(r, "depth") != c.d || s(r, "topk") != c.k) continue;
                (s(r, "alphabeta") == "1" ? vab : vpl) = d(r, "nodes");
            }
            ab.values.push_back(vab);
            plain.values.push_back(vpl);
            if (!std::isnan(vpl)) hi = std::max(hi, vpl);
        }
        p.series = {ab, plain};
        p.yMin = 1;
        p.yMax = std::pow(10, std::ceil(std::log10(hi)));
        fig.panels.push_back(p);
    }
    return saveFigure(dir + "/ab_nodes.svg", fig);
}

// ---------------------------------------------------------------- E5: rollout heuristic vs ngẫu nhiên
bool plotRollout(const std::string& dir) {
    const std::vector<CsvRow> rows = readCsv(dir + "/rollout.csv");
    if (rows.empty()) return false;
    Figure fig;
    fig.title = "E5 (Bài 2 - Hướng A) - Rollout heuristic so với rollout ngẫu nhiên";
    fig.subtitle = "Điểm = (thắng + 0.5·hòa) / số ván; c = 1.4; seed " + s(rows[0], "seed");
    fig.cols = 2;
    fig.panelW = 470;
    fig.panelH = 290;
    const std::vector<std::string> its = distinct(rows, "value");
    auto single = [&](const std::string& group, const std::string& title) {
        Panel p;
        p.kind = Panel::Bars;
        p.title = title;
        p.cats = its;
        p.percent = true;
        p.refLine = 0.5;
        p.valueLabels = true;
        p.xLabel = "số iteration của MCTS heuristic";
        Series se{"MCTS heuristic", color::SERIES[1], {}};
        for (const std::string& it : its) {
            const CsvRow* r = findRow(rows, group, it);
            se.values.push_back(r ? d(*r, "score") : NaN);
        }
        p.series = {se};
        fig.panels.push_back(p);
    };
    single("heur vs rand (cùng iteration)", "Heuristic vs ngẫu nhiên, cùng số iteration");
    single("heur vs rand 20x iteration (cùng thời gian)", "Heuristic (it) vs ngẫu nhiên (20·it) ~ cùng thời gian");
    for (const char* opp : {"vs Random", "vs MiniMax d=1"}) {
        Panel p;
        p.kind = Panel::Bars;
        p.title = std::string("Điểm khi gặp ") + (opp + 3);
        p.cats = its;
        p.percent = true;
        p.valueLabels = true;
        p.xLabel = "số iteration";
        int k = 0;
        for (const char* roll : {"rand", "heur"}) {
            Series se{roll == std::string("rand") ? "Rollout ngẫu nhiên" : "Rollout heuristic", color::SERIES[k++], {}};
            for (const std::string& it : its) {
                const CsvRow* r = findRow(rows, std::string(opp) + "/" + roll, it);
                se.values.push_back(r ? d(*r, "score") : NaN);
            }
            p.series.push_back(se);
        }
        fig.panels.push_back(p);
    }
    fig.note = "Hai khung trên: > 50% nghĩa là heuristic mạnh hơn. Đường đứt = 50%.";
    return saveFigure(dir + "/rollout.svg", fig);
}
}  // namespace

int makePlots(const std::string& dir) {
    int n = 0;
    for (bool ok : {plotCross(dir), plotCsweep(dir), plotItersweep(dir), plotAb(dir), plotRollout(dir)}) n += ok;
    return n;
}

void writeTables(const std::string& dir) {
    FILE* f = std::fopen((dir + "/tables.md").c_str(), "w");
    if (!f) return;
    std::fprintf(f, "# Bảng kết quả thực nghiệm (sinh tự động bởi caro_bench)\n\n");
    std::fprintf(f, "Điểm = (thắng + 0.5·hòa) / số ván, theo góc nhìn tác nhân A. T-H-B = thắng-hòa-thua của A.\n\n");

    const std::vector<CsvRow> cross = readCsv(dir + "/cross.csv");
    if (!cross.empty()) {
        std::fprintf(f, "## E1 - Đấu vòng tròn\n\n");
        matchTable(f, cross, false);
        // điểm trung bình mỗi tác nhân
        std::map<std::string, std::pair<double, int>> tot;
        std::map<std::string, std::pair<double, int>> ms;
        std::vector<std::string> order;
        for (const CsvRow& r : cross) {
            for (int side = 0; side < 2; ++side) {
                const std::string a = s(r, side ? "agent_b" : "agent_a");
                if (!tot.count(a)) order.push_back(a);
                tot[a].first += side ? 1 - d(r, "score") : d(r, "score");
                tot[a].second += 1;
                ms[a].first += d(r, side ? "avg_ms_b" : "avg_ms_a");
                ms[a].second += 1;
            }
        }
        std::fprintf(f, "| Tác nhân | Điểm trung bình | ms/nước trung bình |\n|---|---|---|\n");
        for (const std::string& a : order)
            std::fprintf(f, "| %s | %s | %s |\n", a.c_str(), fmtNum(tot[a].first / tot[a].second, 3).c_str(),
                         fmtNum(ms[a].first / ms[a].second, 2).c_str());
        std::fprintf(f, "\n");
    }
    const struct { const char* file; const char* title; } sweeps[] = {
        {"csweep.csv", "E2 - Quét hằng số c"},
        {"itersweep.csv", "E3 - Quét số iteration"},
        {"rollout.csv", "E5 - Rollout heuristic vs ngẫu nhiên (Bài 2 - Hướng A)"}};
    for (const auto& sw : sweeps) {
        const std::vector<CsvRow> rows = readCsv(dir + "/" + sw.file);
        if (rows.empty()) continue;
        std::fprintf(f, "## %s\n\n", sw.title);
        matchTable(f, rows, true);
    }

    const std::vector<CsvRow> ab = readCsv(dir + "/ab.csv");
    if (!ab.empty()) {
        std::fprintf(f, "## E4 - MiniMax có / không cắt tỉa Alpha-Beta\n\n");
        std::fprintf(f, "| Thế cờ | d | top-k | Nút AB | Nút không cắt | Giảm | Lần cắt | ms AB | ms không cắt | Cùng nước | Cùng giá trị |\n");
        std::fprintf(f, "|---|---|---|---|---|---|---|---|---|---|---|\n");
        for (size_t i = 0; i + 1 < ab.size(); i += 2) {
            const CsvRow &a = ab[i], &p = ab[i + 1];  // hàng AB rồi hàng không cắt tỉa
            const std::string k = s(a, "topk") == "0" ? "tất cả" : s(a, "topk");
            std::fprintf(f, "| %s | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s |\n", s(a, "position").c_str(),
                         s(a, "depth").c_str(), k.c_str(), s(a, "nodes").c_str(), s(p, "nodes").c_str(),
                         pct(1 - d(a, "nodes") / d(p, "nodes")).c_str(), s(a, "cutoffs").c_str(),
                         fmtNum(d(a, "ms"), 3).c_str(), fmtNum(d(p, "ms"), 3).c_str(),
                         s(a, "move") == s(p, "move") ? "có" : "KHÔNG", s(a, "value") == s(p, "value") ? "có" : "KHÔNG");
        }
        std::fprintf(f, "\n");
    }
    std::fclose(f);
}

}  // namespace bench
