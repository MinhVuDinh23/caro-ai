// Các thế cờ thử nghiệm cố định: dùng trong test (AB == MiniMax thường) và thực nghiệm E4.
#pragma once
#include <string>
#include <vector>

#include "board.h"

namespace caro {

struct TestPosition {
    std::string name;
    Board board;
};

const std::vector<TestPosition>& testPositions();

}  // namespace caro
