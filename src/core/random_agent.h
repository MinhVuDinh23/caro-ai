// Tác nhân Random: chọn ngẫu nhiên đều một ô trống bất kỳ (đối thủ cơ sở trong thực nghiệm).
#pragma once
#include <cstdint>
#include <random>

#include "agent.h"

namespace caro {

class RandomAgent : public Agent {
public:
    explicit RandomAgent(uint32_t seed = 0) : rng_(seed) {}
    std::string name() const override { return "Random"; }
    Move choose(const Board& b, SearchStats& stats) override;

private:
    std::mt19937 rng_;
};

}  // namespace caro
