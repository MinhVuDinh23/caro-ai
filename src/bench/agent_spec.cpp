#include "agent_spec.h"

#include <sstream>
#include <stdexcept>

#include "random_agent.h"

namespace bench {

using namespace caro;

namespace {
double num(const std::string& key, const std::string& v) {
    try {
        size_t used = 0;
        const double d = std::stod(v, &used);
        if (used != v.size()) throw std::invalid_argument("");
        return d;
    } catch (...) {
        throw std::invalid_argument("giá trị không hợp lệ: " + key + "=" + v);
    }
}
}  // namespace

AgentSpec parseSpec(const std::string& text) {
    AgentSpec s;
    const size_t colon = text.find(':');
    const std::string kind = text.substr(0, colon);
    const std::string params = colon == std::string::npos ? "" : text.substr(colon + 1);
    if (kind == "random") s.kind = AgentSpec::Random;
    else if (kind == "mm" || kind == "minimax") s.kind = AgentSpec::Minimax;
    else if (kind == "mcts") s.kind = AgentSpec::Mcts;
    else throw std::invalid_argument("loại tác nhân không hợp lệ: " + kind);

    std::stringstream ss(params);
    std::string kv;
    while (std::getline(ss, kv, ',')) {
        if (kv.empty()) continue;
        const size_t eq = kv.find('=');
        if (eq == std::string::npos) throw std::invalid_argument("thiếu '=': " + kv);
        const std::string k = kv.substr(0, eq), v = kv.substr(eq + 1);
        if (s.kind == AgentSpec::Minimax && k == "d") s.mm.depth = static_cast<int>(num(k, v));
        else if (s.kind == AgentSpec::Minimax && k == "k") s.mm.topK = static_cast<int>(num(k, v));
        else if (s.kind == AgentSpec::Minimax && k == "ab") s.mm.alphaBeta = num(k, v) != 0;
        else if (s.kind == AgentSpec::Minimax && k == "r") s.mm.radius = static_cast<int>(num(k, v));
        else if (s.kind == AgentSpec::Mcts && k == "it") s.mc.iterations = static_cast<int>(num(k, v));
        else if (s.kind == AgentSpec::Mcts && k == "c") s.mc.c = num(k, v);
        else if (s.kind == AgentSpec::Mcts && k == "fm") s.mc.forcedMoves = num(k, v) != 0;
        else if (s.kind == AgentSpec::Mcts && k == "r") s.mc.radius = static_cast<int>(num(k, v));
        else if (s.kind == AgentSpec::Mcts && k == "roll" && (v == "rand" || v == "heur"))
            s.mc.rollout = v == "rand" ? RolloutPolicy::Random : RolloutPolicy::Heuristic;
        else throw std::invalid_argument("tham số không hợp lệ cho " + kind + ": " + kv);
    }
    if (s.kind == AgentSpec::Minimax && s.mm.depth < 1) throw std::invalid_argument("cần d >= 1");
    if (s.kind == AgentSpec::Mcts && (s.mc.iterations < 1 || s.mc.c <= 0))
        throw std::invalid_argument("cần it >= 1 và c > 0");
    s.label = makeAgent(s, 0)->name();
    return s;
}

std::unique_ptr<Agent> makeAgent(const AgentSpec& spec, uint32_t seed) {
    switch (spec.kind) {
        case AgentSpec::Minimax:
            return std::make_unique<MinimaxAgent>(spec.mm);
        case AgentSpec::Mcts: {
            MctsConfig c = spec.mc;
            c.seed = seed;
            return std::make_unique<MctsAgent>(c);
        }
        default:
            return std::make_unique<RandomAgent>(seed);
    }
}

}  // namespace bench
