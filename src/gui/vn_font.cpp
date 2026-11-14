#include "vn_font.h"

#include <string>
#include <vector>

Font loadVietnameseFont(const char* relPath, int size) {
    // Các khối Unicode chứa chữ tiếng Việt: ASCII, Latin-1, Latin Extended-A/B (đ, ă, ơ, ư),
    // Latin Extended Additional (ạ ả ấ ầ ... ỹ) và vài ký hiệu hay dùng.
    std::vector<int> cps;
    auto range = [&](int a, int b) {
        for (int c = a; c <= b; ++c) cps.push_back(c);
    };
    range(32, 126);
    range(0xA0, 0xFF);
    range(0x100, 0x17F);
    range(0x1A0, 0x1B0);
    range(0x1EA0, 0x1EF9);
    for (int c : {0x2013, 0x2014, 0x2026, 0x2022}) cps.push_back(c);

    std::string path = std::string(GetApplicationDirectory()) + relPath;
    if (!FileExists(path.c_str())) path = relPath;
    if (!FileExists(path.c_str())) {
        TraceLog(LOG_WARNING, "Không tìm thấy font %s -> dùng font mặc định", relPath);
        return GetFontDefault();
    }
    Font f = LoadFontEx(path.c_str(), size, cps.data(), static_cast<int>(cps.size()));
    if (!IsFontValid(f)) return GetFontDefault();
    SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
    return f;
}
