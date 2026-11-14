#include "csv.h"

#include <fstream>

namespace bench {

void writeCsvLine(FILE* f, const std::vector<std::string>& fields) {
    for (size_t i = 0; i < fields.size(); ++i) {
        const std::string& s = fields[i];
        if (i) std::fputc(',', f);
        if (s.find_first_of(",\"\n") == std::string::npos) {
            std::fputs(s.c_str(), f);
        } else {
            std::fputc('"', f);
            for (char c : s) {
                if (c == '"') std::fputc('"', f);
                std::fputc(c, f);
            }
            std::fputc('"', f);
        }
    }
    std::fputc('\n', f);
    std::fflush(f);
}

namespace {
std::vector<std::string> splitLine(const std::string& line) {
    std::vector<std::string> out;
    std::string cur;
    bool quoted = false;
    for (size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (quoted) {
            if (c == '"' && i + 1 < line.size() && line[i + 1] == '"') cur += '"', ++i;
            else if (c == '"') quoted = false;
            else cur += c;
        } else if (c == '"') {
            quoted = true;
        } else if (c == ',') {
            out.push_back(cur);
            cur.clear();
        } else if (c != '\r') {
            cur += c;
        }
    }
    out.push_back(cur);
    return out;
}
}  // namespace

std::vector<CsvRow> readCsv(const std::string& path) {
    std::vector<CsvRow> rows;
    std::ifstream in(path);
    std::string line;
    if (!std::getline(in, line)) return rows;
    const std::vector<std::string> header = splitLine(line);
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        const std::vector<std::string> f = splitLine(line);
        CsvRow r;
        for (size_t i = 0; i < header.size() && i < f.size(); ++i) r[header[i]] = f[i];
        rows.push_back(r);
    }
    return rows;
}

std::string fmtNum(double v, int decimals) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", decimals, v);
    return buf;
}

}  // namespace bench
