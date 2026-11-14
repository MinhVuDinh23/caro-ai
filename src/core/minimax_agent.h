// Tác nhân MiniMax có (hoặc không) cắt tỉa Alpha-Beta.
//
// - Nút MAX: lượt của chính tác nhân (bên đến lượt ở gốc) -> chọn giá trị lớn nhất.
// - Nút MIN: lượt của đối thủ -> giả định đối thủ chọn giá trị nhỏ nhất (tệ nhất cho ta).
// - Điều kiện dừng: có người thắng, bàn đầy (hòa) hoặc hết độ sâu d -> dùng hàm đánh giá.
// - Giới hạn ứng viên: chỉ xét ô trong bán kính `radius` quanh quân đã có, sắp xếp theo
//   scoreMove (tấn công + phòng thủ) và giữ lại `topK` nước tốt nhất ở MỖI nút.
// - alphaBeta = false: cùng một hàm tìm kiếm nhưng không cắt tỉa, để so sánh số nút.
#pragma once
#include <vector>

#include "agent.h"

namespace caro {

struct MinimaxConfig {
    int depth = 2;          // độ sâu tìm kiếm d (>= 1)
    bool alphaBeta = true;  // bật/tắt cắt tỉa
    int topK = 12;          // số nước ứng viên tối đa mỗi nút (<= 0: không giới hạn)
    int radius = 2;         // bán kính sinh nước ứng viên
};

class MinimaxAgent : public Agent {
public:
    // Điểm thắng/thua tuyệt đối, lớn hơn mọi giá trị của hàm đánh giá.
    static constexpr int WIN_SCORE = 10'000'000;
    static constexpr int INF = 1'000'000'000;

    explicit MinimaxAgent(const MinimaxConfig& cfg = {}) : cfg_(cfg) {}
    std::string name() const override;
    Move choose(const Board& b, SearchStats& stats) override;

    const MinimaxConfig& config() const { return cfg_; }
    void setConfig(const MinimaxConfig& cfg) { cfg_ = cfg; }

    // Các nước ứng viên đã sắp xếp giảm dần theo scoreMove và cắt còn topK.
    std::vector<Move> orderedMoves(const Board& b) const;

private:
    int search(Board& b, int depth, int alpha, int beta, bool maximizing);

    MinimaxConfig cfg_;
    Cell me_ = Cell::X;  // bên MAX
    long long nodes_ = 0;
    long long cutoffs_ = 0;
};

}  // namespace caro
