#include "log_format.h"

#include <cstdio>

namespace caro {

std::string formatMoveLog(const std::string& agentName, Cell side, const SearchStats& s) {
    char buf[96];
    std::snprintf(buf, sizeof(buf), "%.2f ms", s.ms);
    std::string out = "[" + agentName + "] " + cellChar(side) + " -> " + moveToString(s.move) + " | " + buf;
    if (s.depth > 0) out += " | d=" + std::to_string(s.depth);
    if (s.iterations > 0) out += " | iter=" + std::to_string(s.iterations);
    out += " | nút=" + std::to_string(s.nodes);
    if (s.depth > 0) {
        out += " | cắt tỉa=" + std::to_string(s.cutoffs);
        out += " | giá trị=" + std::to_string(s.value);
    }
    for (size_t i = 0; i < s.topChildren.size(); ++i) {
        const ChildInfo& c = s.topChildren[i];
        std::snprintf(buf, sizeof(buf), "%s%s N=%d W/N=%.2f", i == 0 ? " | top: " : "; ",
                      moveToString(c.move).c_str(), c.visits, c.meanReward);
        out += buf;
    }
    return out;
}

}  // namespace caro
