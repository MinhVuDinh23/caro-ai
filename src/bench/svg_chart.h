// Vẽ biểu đồ SVG thuần C++ (không cần Python): cột nhóm, cột chồng, đường, bản đồ nhiệt.
// Mỗi khung chỉ có MỘT trục y; hai đại lượng khác thang đo -> hai khung riêng.
#pragma once
#include <limits>
#include <string>
#include <vector>

namespace bench {

namespace color {
// Bảng màu phân loại (thứ tự cố định, đã kiểm tra mù màu) + bảng phân kỳ thắng/hòa/thua.
inline const char* SERIES[] = {"#2a78d6", "#eb6834", "#1baf7a", "#eda100"};
inline const char* WIN = "#2a78d6";
inline const char* DRAW = "#a3a29c";
inline const char* LOSS = "#e34948";
}  // namespace color

struct Series {
    std::string name;
    std::string color;
    std::vector<double> values;  // một giá trị cho mỗi nhãn trục x
};

struct Panel {
    enum Kind { Bars, StackedBars, Lines } kind = Bars;
    std::string title;
    std::vector<std::string> cats;  // nhãn trục x
    std::vector<Series> series;
    std::string xLabel, yLabel;
    double yMin = 0, yMax = 1;
    bool logY = false;
    bool percent = false;  // hiển thị trục y dạng %
    double refLine = std::numeric_limits<double>::quiet_NaN();  // đường tham chiếu (vd. 50%)
    bool valueLabels = false;  // ghi giá trị trên cột (cột chồng: ghi giá trị đoạn đầu tiên)
};

struct Figure {
    std::string title, subtitle;
    std::vector<Panel> panels;
    int cols = 2;
    double panelW = 420, panelH = 300;
    std::string note;  // dòng chú thích cuối hình
};

bool saveFigure(const std::string& path, const Figure& fig);

// Bản đồ nhiệt ma trận điểm: value[i][j] trong [0,1] (NaN = ô trống), màu phân kỳ đỏ - xám - xanh.
bool saveHeatmap(const std::string& path, const std::string& title, const std::string& subtitle,
                 const std::vector<std::string>& labels, const std::vector<std::vector<double>>& value,
                 const std::string& note);

}  // namespace bench
