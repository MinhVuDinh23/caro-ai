// Các thực nghiệm E1-E5 -> ghi CSV vào thư mục kết quả.
#pragma once
#include <cstdint>
#include <string>

namespace bench {

struct ExpContext {
    int games = 100;      // số ván mỗi cấu hình (đề: >= 20, khuyến nghị 40-100; đổi người đi trước theo cặp)
    int gamesSelf = 200;  // số ván cho MCTS đấu MCTS (nhanh, nhiều nhiễu -> cần nhiều ván hơn)
    uint32_t seed = 2026;
    int threads = 8;
    std::string outDir = "results";
};

void expCross(const ExpContext& ctx);      // E1: đấu vòng tròn các tác nhân
void expCsweep(const ExpContext& ctx);     // E2: quét c
void expItersweep(const ExpContext& ctx);  // E3: quét số iteration
void expAb(const ExpContext& ctx);         // E4: MiniMax có / không cắt tỉa
void expRollout(const ExpContext& ctx);    // E5: rollout heuristic vs ngẫu nhiên (Bài 2 - Hướng A)

}  // namespace bench
