// caro_bench - chạy thực nghiệm, ghi CSV, vẽ biểu đồ SVG và bảng markdown.
//
//   caro_bench --exp all                       chạy E1-E5 rồi vẽ (mặc định)
//   caro_bench --exp csweep,itersweep          chỉ chạy một số thực nghiệm
//   caro_bench --exp plot                      chỉ vẽ lại biểu đồ + bảng từ CSV có sẵn
//   caro_bench --match "<spec A>" "<spec B>"   đấu thử hai tác nhân (không ghi file)
// Tùy chọn: --games 100  --games-self 200  --seed 2026  --threads <số luồng>  --out results
// Chuỗi tác nhân: random | mm:d=2,k=12,ab=1 | mcts:it=300,c=1.4,roll=rand|heur,fm=0
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
#include <sstream>
#include <string>
#include <thread>

#include "experiments.h"
#include "match.h"
#include "plots.h"

#if defined(_MSC_VER)
#include <intrin.h>
extern "C" __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int);
#endif

namespace {

std::string cpuName() {
#if defined(_MSC_VER)
    int info[4] = {0};
    char brand[49] = {0};
    __cpuid(info, 0x80000000);
    if (static_cast<unsigned>(info[0]) >= 0x80000004u) {
        for (int i = 0; i < 3; ++i) {
            __cpuid(info, 0x80000002 + i);
            std::memcpy(brand + 16 * i, info, 16);
        }
        std::string s(brand);
        while (!s.empty() && s.front() == ' ') s.erase(s.begin());
        return s;
    }
#endif
    return "unknown";
}

void writeMachine(const bench::ExpContext& ctx, const std::string& cmd, const std::string& exps, double seconds) {
    FILE* f = std::fopen((ctx.outDir + "/machine.txt").c_str(), "w");
    if (!f) return;
    const std::time_t now = std::time(nullptr);
    char when[64];
    std::strftime(when, sizeof(when), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    std::fprintf(f, "Thời điểm      : %s\n", when);
    std::fprintf(f, "CPU            : %s\n", cpuName().c_str());
    std::fprintf(f, "Số luồng phần cứng: %u\n", std::thread::hardware_concurrency());
#if defined(_WIN32)
    std::fprintf(f, "Hệ điều hành   : Windows\n");
#else
    std::fprintf(f, "Hệ điều hành   : POSIX\n");
#endif
#if defined(_MSC_VER)
    std::fprintf(f, "Trình biên dịch: MSVC %d, Release (/O2)\n", _MSC_VER);
#endif
    std::fprintf(f, "Lệnh           : %s\n", cmd.c_str());
    std::fprintf(f, "Thực nghiệm    : %s\n", exps.c_str());
    std::fprintf(f, "Số ván/cấu hình: %d (MCTS vs MCTS: %d), đổi người đi trước theo cặp\n", ctx.games, ctx.gamesSelf);
    std::fprintf(f, "Seed gốc       : %u (ván i dùng seed + 7919*(i/2) + 1|2)\n", ctx.seed);
    std::fprintf(f, "Số luồng chạy  : %d (mỗi ván chạy trên 1 luồng; thời gian đo bằng steady_clock)\n", ctx.threads);
    std::fprintf(f, "Giới hạn       : không giới hạn thời gian mỗi nước; dừng khi có người thắng hoặc bàn đầy\n");
    std::fprintf(f, "Tổng thời gian : %.1f s\n", seconds);
    std::fclose(f);
}

bool has(const std::string& list, const char* name) {
    if (list == "all") return true;
    std::stringstream ss(list);
    std::string item;
    while (std::getline(ss, item, ','))
        if (item == name) return true;
    return false;
}

}  // namespace

int main(int argc, char** argv) {
#if defined(_MSC_VER)
    SetConsoleOutputCP(65001);
#endif
    bench::ExpContext ctx;
    ctx.threads = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) / 2);
    std::string exps = "all", matchA, matchB, cmd = "caro_bench";
    for (int i = 1; i < argc; ++i) cmd += std::string(" ") + argv[i];

    try {
        for (int i = 1; i < argc; ++i) {
            const std::string a = argv[i];
            const bool next = i + 1 < argc;
            if (a == "--exp" && next) exps = argv[++i];
            else if (a == "--games" && next) ctx.games = std::atoi(argv[++i]);
            else if (a == "--games-self" && next) ctx.gamesSelf = std::atoi(argv[++i]);
            else if (a == "--seed" && next) ctx.seed = static_cast<uint32_t>(std::strtoul(argv[++i], nullptr, 10));
            else if (a == "--threads" && next) ctx.threads = std::max(1, std::atoi(argv[++i]));
            else if (a == "--out" && next) ctx.outDir = argv[++i];
            else if (a == "--match" && i + 2 < argc) matchA = argv[++i], matchB = argv[++i];
            else {
                std::fprintf(stderr, "Tham số không hợp lệ: %s (xem chú thích đầu src/bench/main.cpp)\n", a.c_str());
                return 2;
            }
        }
        if (ctx.games < 2 || ctx.games % 2 || ctx.gamesSelf < 2 || ctx.gamesSelf % 2) {
            std::fprintf(stderr, "Số ván phải là số chẵn >= 2 (đổi người đi trước theo cặp)\n");
            return 2;
        }

        if (!matchA.empty()) {
            const bench::MatchStats m =
                bench::runMatch(bench::parseSpec(matchA), bench::parseSpec(matchB), ctx.games, ctx.seed, ctx.threads);
            std::printf("%s vs %s: %d-%d-%d (T-H-B), điểm %.3f, ms/nước %.2f / %.2f, số nước TB %.1f\n", m.a.c_str(),
                        m.b.c_str(), m.win, m.draw, m.loss, m.score(), m.avgMsA(), m.avgMsB(), m.avgPlies());
            return 0;
        }

        std::filesystem::create_directories(ctx.outDir);
        const auto t0 = std::chrono::steady_clock::now();
        bool ran = false;
        if (has(exps, "ab")) bench::expAb(ctx), ran = true;
        if (has(exps, "cross")) bench::expCross(ctx), ran = true;
        if (has(exps, "csweep")) bench::expCsweep(ctx), ran = true;
        if (has(exps, "itersweep")) bench::expItersweep(ctx), ran = true;
        if (has(exps, "rollout")) bench::expRollout(ctx), ran = true;
        const double sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        if (ran) writeMachine(ctx, cmd, exps, sec);

        const int n = bench::makePlots(ctx.outDir);
        bench::writeTables(ctx.outDir);
        std::printf("Xong: %d biểu đồ SVG + tables.md trong %s/ (%.1f s)\n", n, ctx.outDir.c_str(), sec);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Lỗi: %s\n", e.what());
        return 1;
    }
    return 0;
}
