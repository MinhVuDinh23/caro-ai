// Mô tả tác nhân bằng chuỗi để cấu hình thực nghiệm từ dòng lệnh:
//   random
//   mm:d=2,k=12,ab=1,r=2            (MiniMax: độ sâu, top-k, alpha-beta, bán kính)
//   mcts:it=300,c=1.4,roll=rand,fm=0,r=2   (roll = rand | heur; fm = nước bắt buộc)
#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include "agent.h"
#include "mcts_agent.h"
#include "minimax_agent.h"

namespace bench {

struct AgentSpec {
    enum Kind { Random, Minimax, Mcts } kind = Random;
    caro::MinimaxConfig mm;
    caro::MctsConfig mc;
    std::string label;  // tên hiển thị = Agent::name(), ví dụ "MCTS(it=300,c=1.40,rand)"
};

// Ném std::invalid_argument nếu chuỗi sai.
AgentSpec parseSpec(const std::string& text);
// Tạo tác nhân mới với seed cho trước (Random, MCTS dùng seed; MiniMax tất định).
std::unique_ptr<caro::Agent> makeAgent(const AgentSpec& spec, uint32_t seed);

}  // namespace bench
