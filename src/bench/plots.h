// Đọc CSV kết quả -> vẽ biểu đồ SVG và xuất bảng markdown (tables.md) để đưa vào báo cáo.
#pragma once
#include <string>

namespace bench {

// Vẽ mọi biểu đồ có CSV tương ứng trong thư mục `dir`; trả về số hình đã vẽ.
int makePlots(const std::string& dir);
// Ghi dir/tables.md từ các CSV hiện có.
void writeTables(const std::string& dir);

}  // namespace bench
