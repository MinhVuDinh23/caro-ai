#include "svg_chart.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <set>
#include <sstream>

namespace bench {

namespace {
const char* SURFACE = "#fcfcfb";
const char* TEXT1 = "#0b0b0b";
const char* TEXT2 = "#52514e";
const char* MUTED = "#8a8984";
const char* GRID = "#e6e5e0";
const char* AXIS = "#bdbcb6";
const char* FONT = "Noto Sans, Segoe UI, Arial, sans-serif";

std::string esc(const std::string& s) {
    std::string o;
    for (char c : s) {
        if (c == '&') o += "&amp;";
        else if (c == '<') o += "&lt;";
        else if (c == '>') o += "&gt;";
        else if (c == '"') o += "&quot;";
        else o += c;
    }
    return o;
}

std::string num(double v) {
    char b[32];
    std::snprintf(b, sizeof(b), "%.1f", v);
    return b;
}

class Svg {
public:
    Svg(double w, double h) : w_(w), h_(h) {}
    void rect(double x, double y, double w, double h, const std::string& fill, const std::string& tip = "") {
        o_ << "<rect x=\"" << num(x) << "\" y=\"" << num(y) << "\" width=\"" << num(std::max(0.0, w)) << "\" height=\""
           << num(std::max(0.0, h)) << "\" fill=\"" << fill << "\">" << tipTag(tip) << "</rect>\n";
    }
    // cột có 2 góc trên bo tròn r (đầu dữ liệu), đáy vuông nằm trên đường gốc
    void bar(double x, double y, double w, double h, const std::string& fill, const std::string& tip, double r = 4) {
        if (h <= 0 || w <= 0) return;
        r = std::min({r, w / 2, h});
        o_ << "<path d=\"M" << num(x) << "," << num(y + h) << " V" << num(y + r) << " Q" << num(x) << "," << num(y) << " "
           << num(x + r) << "," << num(y) << " H" << num(x + w - r) << " Q" << num(x + w) << "," << num(y) << " "
           << num(x + w) << "," << num(y + r) << " V" << num(y + h) << " Z\" fill=\"" << fill << "\">" << tipTag(tip)
           << "</path>\n";
    }
    void line(double x1, double y1, double x2, double y2, const std::string& stroke, double sw = 1,
              const std::string& dash = "") {
        o_ << "<line x1=\"" << num(x1) << "\" y1=\"" << num(y1) << "\" x2=\"" << num(x2) << "\" y2=\"" << num(y2)
           << "\" stroke=\"" << stroke << "\" stroke-width=\"" << sw << "\""
           << (dash.empty() ? "" : " stroke-dasharray=\"" + dash + "\"") << "/>\n";
    }
    void polyline(const std::vector<std::pair<double, double>>& pts, const std::string& stroke, double sw) {
        o_ << "<polyline fill=\"none\" stroke=\"" << stroke << "\" stroke-width=\"" << sw
           << "\" stroke-linejoin=\"round\" points=\"";
        for (const auto& p : pts) o_ << num(p.first) << "," << num(p.second) << " ";
        o_ << "\"/>\n";
    }
    void dot(double x, double y, double r, const std::string& fill, const std::string& tip) {
        o_ << "<circle cx=\"" << num(x) << "\" cy=\"" << num(y) << "\" r=\"" << r << "\" fill=\"" << fill
           << "\" stroke=\"" << SURFACE << "\" stroke-width=\"2\">" << tipTag(tip) << "</circle>\n";
    }
    void text(double x, double y, const std::string& s, double size, const std::string& fill,
              const char* anchor = "start", const char* weight = "normal", double rotate = 0) {
        o_ << "<text x=\"" << num(x) << "\" y=\"" << num(y) << "\" font-size=\"" << size << "\" fill=\"" << fill
           << "\" text-anchor=\"" << anchor << "\" font-weight=\"" << weight << "\"";
        if (rotate != 0) o_ << " transform=\"rotate(" << rotate << " " << num(x) << " " << num(y) << ")\"";
        o_ << ">" << esc(s) << "</text>\n";
    }
    bool save(const std::string& path) const {
        std::ofstream f(path, std::ios::binary);
        if (!f) return false;
        f << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << num(w_) << "\" height=\"" << num(h_)
          << "\" viewBox=\"0 0 " << num(w_) << " " << num(h_) << "\" font-family=\"" << FONT << "\">\n"
          << "<rect width=\"100%\" height=\"100%\" fill=\"" << SURFACE << "\"/>\n"
          << o_.str() << "</svg>\n";
        return static_cast<bool>(f);
    }

private:
    static std::string tipTag(const std::string& tip) { return tip.empty() ? "" : "<title>" + esc(tip) + "</title>"; }
    double w_, h_;
    std::ostringstream o_;
};

std::string fmtTick(double v, bool percent) {
    char b[32];
    if (percent) std::snprintf(b, sizeof(b), "%.0f%%", v * 100);
    else if (std::fabs(v) >= 1e6) std::snprintf(b, sizeof(b), "%.0fM", v / 1e6);
    else if (std::fabs(v) >= 1000) std::snprintf(b, sizeof(b), "%.0fk", v / 1000);
    else if (std::fabs(v) >= 1 || v == 0) std::snprintf(b, sizeof(b), "%.0f", v);
    else std::snprintf(b, sizeof(b), "%.2g", v);
    return b;
}

std::string fmtValue(double v, bool percent) {
    char b[32];
    if (percent) std::snprintf(b, sizeof(b), "%.0f%%", v * 100);
    else if (std::fabs(v) >= 100) std::snprintf(b, sizeof(b), "%.0f", v);
    else std::snprintf(b, sizeof(b), "%.2f", v);
    return b;
}

// Một khung biểu đồ trong hình chữ nhật (x, y, w, h).
void drawPanel(Svg& s, double x, double y, double w, double h, const Panel& p) {
    const double left = 52, right = p.kind == Panel::Lines ? 96 : 12, top = 44, bottom = 46;
    const double px = x + left, py = y + top, pw = w - left - right, ph = h - top - bottom;
    s.text(x + 4, y + 16, p.title, 13, TEXT1, "start", "600");

    const double lo = p.logY ? std::log10(std::max(p.yMin, 1e-9)) : p.yMin;
    const double hi = p.logY ? std::log10(p.yMax) : p.yMax;
    auto Y = [&](double v) {
        double t = p.logY ? std::log10(std::max(v, p.yMin)) : v;
        t = (t - lo) / (hi - lo);
        return py + ph - std::clamp(t, 0.0, 1.0) * ph;
    };

    // lưới + nhãn trục y
    std::vector<double> ticks;
    if (p.logY) {
        for (double v = std::pow(10, std::floor(lo)); v <= p.yMax * 1.0001; v *= 10)
            if (v >= p.yMin * 0.9999) ticks.push_back(v);
    } else {
        const double span = p.yMax - p.yMin;
        const double raw = span / 4, mag = std::pow(10, std::floor(std::log10(raw)));
        const double step = raw / mag <= 1 ? mag : raw / mag <= 2 ? 2 * mag : raw / mag <= 2.5 ? 2.5 * mag : raw / mag <= 5 ? 5 * mag : 10 * mag;
        for (double v = std::ceil(p.yMin / step) * step; v <= p.yMax + step * 1e-6; v += step) ticks.push_back(v);
    }
    for (double v : ticks) {
        s.line(px, Y(v), px + pw, Y(v), GRID, 1);
        s.text(px - 6, Y(v) + 4, fmtTick(v, p.percent), 11, TEXT2, "end");
    }
    s.line(px, py + ph, px + pw, py + ph, AXIS, 1);
    if (!std::isnan(p.refLine)) s.line(px, Y(p.refLine), px + pw, Y(p.refLine), MUTED, 1.2, "5,4");
    if (!p.yLabel.empty()) s.text(x + 12, py + ph / 2, p.yLabel, 11, TEXT2, "middle", "normal", -90);

    // trục x (nhãn phân loại, cách đều)
    const size_t n = p.cats.size();
    const double band = n ? pw / n : pw;
    auto X = [&](size_t i) { return px + band * (i + 0.5); };
    for (size_t i = 0; i < n; ++i) s.text(X(i), py + ph + 16, p.cats[i], 11, TEXT2, "middle");
    if (!p.xLabel.empty()) s.text(px + pw / 2, py + ph + 36, p.xLabel, 11, TEXT2, "middle");

    if (p.kind == Panel::Bars) {
        const size_t k = p.series.size();
        const double groupW = band * (k > 1 ? 0.78 : 0.5), bw = groupW / k;
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < k; ++j) {
                const double v = p.series[j].values[i];
                if (std::isnan(v)) continue;
                const double bx = X(i) - groupW / 2 + j * bw + 1;  // khe 2px giữa các cột
                const std::string tip = p.series[j].name + " | " + p.cats[i] + ": " + fmtValue(v, p.percent);
                s.bar(bx, Y(v), bw - 2, py + ph - Y(v), p.series[j].color, tip);
                if (p.valueLabels) s.text(bx + (bw - 2) / 2, Y(v) - 4, fmtValue(v, p.percent), 10, TEXT2, "middle");
            }
        }
    } else if (p.kind == Panel::StackedBars) {
        const double bw = band * 0.56;
        for (size_t i = 0; i < n; ++i) {
            double acc = 0;
            size_t topIdx = 0;
            for (size_t j = 0; j < p.series.size(); ++j)
                if (p.series[j].values[i] > 0) topIdx = j;
            for (size_t j = 0; j < p.series.size(); ++j) {
                const double v = p.series[j].values[i];
                if (!(v > 0)) continue;
                const double y0 = Y(acc), y1 = Y(acc + v);
                acc += v;
                const std::string tip = p.series[j].name + " | " + p.cats[i] + ": " + fmtValue(v, p.percent);
                const double gap = j == topIdx ? 0 : 2;  // khe 2px giữa các đoạn
                if (j == topIdx) s.bar(X(i) - bw / 2, y1, bw, y0 - y1, p.series[j].color, tip);
                else s.rect(X(i) - bw / 2, y1 + gap, bw, y0 - y1 - gap, p.series[j].color, tip);
            }
            if (p.valueLabels && !p.series.empty())
                s.text(X(i), Y(acc) - 5, fmtValue(p.series[0].values[i], p.percent), 10, TEXT2, "middle");
        }
    } else {  // Lines
        struct EndLabel { double y; std::string text; };
        std::vector<EndLabel> ends;
        for (const Series& se : p.series) {
            std::vector<std::pair<double, double>> pts;
            for (size_t i = 0; i < n; ++i)
                if (!std::isnan(se.values[i])) pts.push_back({X(i), Y(se.values[i])});
            s.polyline(pts, se.color, 2);
            for (size_t i = 0; i < n; ++i)
                if (!std::isnan(se.values[i]))
                    s.dot(X(i), Y(se.values[i]), 4, se.color, se.name + " | " + p.cats[i] + ": " + fmtValue(se.values[i], p.percent));
            if (!pts.empty()) ends.push_back({pts.back().second, se.name});
        }
        // nhãn trực tiếp ở cuối đường, đẩy xa nhau tối thiểu 14px
        std::sort(ends.begin(), ends.end(), [](const EndLabel& a, const EndLabel& b) { return a.y < b.y; });
        for (size_t i = 1; i < ends.size(); ++i) ends[i].y = std::max(ends[i].y, ends[i - 1].y + 14);
        if (p.series.size() > 1)
            for (const EndLabel& e : ends) s.text(px + pw + 8, e.y + 4, e.text, 10, TEXT2);
    }
}
}  // namespace

bool saveFigure(const std::string& path, const Figure& fig) {
    // chú giải: hợp các chuỗi dữ liệu (theo tên) của mọi khung có >= 2 chuỗi
    std::vector<std::pair<std::string, std::string>> legend;
    std::set<std::string> seen;
    for (const Panel& p : fig.panels)
        if (p.series.size() > 1)
            for (const Series& s : p.series)
                if (seen.insert(s.name).second) legend.push_back({s.name, s.color});

    const int rows = static_cast<int>((fig.panels.size() + fig.cols - 1) / fig.cols);
    const double headH = 58 + (legend.empty() ? 0 : 24), footH = fig.note.empty() ? 12 : 30;
    const double W = fig.cols * fig.panelW + 20, H = headH + rows * fig.panelH + footH;
    Svg s(W, H);
    s.text(14, 26, fig.title, 17, TEXT1, "start", "600");
    s.text(14, 46, fig.subtitle, 12, TEXT2);
    double lx = 14;
    for (const auto& [name, col] : legend) {
        s.rect(lx, 60, 12, 12, col);
        s.text(lx + 17, 70, name, 12, TEXT2);
        lx += 30 + 6.6 * name.size();
    }
    for (size_t i = 0; i < fig.panels.size(); ++i) {
        const double x = 10 + (i % fig.cols) * fig.panelW, y = headH + (i / fig.cols) * fig.panelH;
        drawPanel(s, x, y, fig.panelW, fig.panelH, fig.panels[i]);
    }
    if (!fig.note.empty()) s.text(14, H - 10, fig.note, 11, MUTED);
    return s.save(path);
}

bool saveHeatmap(const std::string& path, const std::string& title, const std::string& subtitle,
                 const std::vector<std::string>& labels, const std::vector<std::vector<double>>& value,
                 const std::string& note) {
    const size_t n = labels.size();
    const double cell = 66, left = 170, top = 150;
    const double W = left + n * cell + 120, H = top + n * cell + 70;
    Svg s(W, H);
    s.text(14, 26, title, 17, TEXT1, "start", "600");
    s.text(14, 46, subtitle, 12, TEXT2);

    // màu phân kỳ: 0 = đỏ (hàng thua), 0.5 = xám trung tính, 1 = xanh (hàng thắng)
    auto lerp = [](int a, int b, double t) { return static_cast<int>(std::lround(a + (b - a) * t)); };
    auto colorOf = [&](double v) {
        const int red[3] = {0xe3, 0x49, 0x48}, mid[3] = {0xf0, 0xef, 0xec}, blue[3] = {0x2a, 0x78, 0xd6};
        const int* a = v < 0.5 ? red : mid;
        const int* b = v < 0.5 ? mid : blue;
        const double t = v < 0.5 ? v / 0.5 : (v - 0.5) / 0.5;
        char buf[8];
        std::snprintf(buf, sizeof(buf), "#%02x%02x%02x", lerp(a[0], b[0], t), lerp(a[1], b[1], t), lerp(a[2], b[2], t));
        return std::string(buf);
    };
    for (size_t i = 0; i < n; ++i) {
        s.text(left - 8, top + i * cell + cell / 2 + 4, labels[i], 11, TEXT1, "end");
        s.text(left + i * cell + cell / 2 + 4, top - 8, labels[i], 11, TEXT1, "start", "normal", -40);
        for (size_t j = 0; j < n; ++j) {
            const double v = value[i][j];
            const double cx = left + j * cell, cy = top + i * cell;
            if (std::isnan(v)) {
                s.rect(cx + 1, cy + 1, cell - 2, cell - 2, "#f5f4f1");
                continue;
            }
            s.rect(cx + 1, cy + 1, cell - 2, cell - 2, colorOf(v), labels[i] + " vs " + labels[j] + ": " + fmtValue(v, true));
            const bool dark = std::fabs(v - 0.5) > 0.3;
            s.text(cx + cell / 2, cy + cell / 2 + 4, fmtValue(v, true), 12, dark ? "#ffffff" : TEXT1, "middle");
        }
    }
    // thang màu
    const double ly = top + n * cell + 24;
    for (int k = 0; k <= 10; ++k) s.rect(left + k * 22, ly, 22, 12, colorOf(k / 10.0));
    s.text(left - 8, ly + 10, "0%", 11, TEXT2, "end");
    s.text(left + 11 * 22 + 6, ly + 10, "100% (hàng thắng hết)", 11, TEXT2);
    if (!note.empty()) s.text(14, H - 10, note, 11, MUTED);
    return s.save(path);
}

}  // namespace bench
