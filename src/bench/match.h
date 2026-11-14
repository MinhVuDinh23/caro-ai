// Đấu nhiều ván giữa hai tác nhân A và B, đổi người đi trước theo cặp, chạy song song.
#pragma once
#include <cstdint>
#include <string>

#include "agent_spec.h"

namespace bench {

struct MatchStats {
    std::string a, b;          // nhãn tác nhân
    int games = 0, win = 0, draw = 0, loss = 0;  // theo góc nhìn của A
    double totalMsA = 0, totalMsB = 0;
    long long movesA = 0, movesB = 0, plies = 0;
    uint32_t seed = 0;

    double winRate() const { return games ? double(win) / games : 0; }
    double score() const { return games ? (win + 0.5 * draw) / games : 0; }  // thắng 1, hòa 0.5
    double avgMsA() const { return movesA ? totalMsA / movesA : 0; }
    double avgMsB() const { return movesB ? totalMsB / movesB : 0; }
    double avgPlies() const { return games ? double(plies) / games : 0; }
};

// Ván i: i chẵn -> A cầm X (đi trước), i lẻ -> B cầm X. Hai ván của một cặp dùng cùng seed,
// chỉ khác bên đi trước. Seed tác nhân: A = seed + 7919*(i/2) + 1, B = ... + 2
// -> kết quả không phụ thuộc số luồng hay thứ tự chạy.
MatchStats runMatch(const AgentSpec& a, const AgentSpec& b, int games, uint32_t seed, int threads);

}  // namespace bench
