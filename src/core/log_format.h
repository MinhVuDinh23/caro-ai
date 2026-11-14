// Định dạng một dòng nhật ký cho mỗi nước đi của máy (dùng chung GUI, benchmark, console).
#pragma once
#include <string>

#include "agent.h"

namespace caro {

// Ví dụ: "[MiniMax(d=3,AB,k=12)] O -> E6 | 2.31 ms | d=3 | nút=1834 | cắt tỉa=412 | giá trị=120"
std::string formatMoveLog(const std::string& agentName, Cell side, const SearchStats& s);

}  // namespace caro
