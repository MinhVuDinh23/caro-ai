// Ghi/đọc CSV đơn giản (trường có dấu phẩy hoặc ngoặc kép được đặt trong "...").
#pragma once
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace bench {

using CsvRow = std::map<std::string, std::string>;  // tên cột -> giá trị

void writeCsvLine(FILE* f, const std::vector<std::string>& fields);
// Đọc toàn bộ file (dòng đầu là tiêu đề). File không tồn tại -> rỗng.
std::vector<CsvRow> readCsv(const std::string& path);

std::string fmtNum(double v, int decimals);

}  // namespace bench
