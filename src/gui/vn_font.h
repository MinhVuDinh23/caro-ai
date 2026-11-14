// Nạp font TTF có đủ chữ tiếng Việt (font mặc định của raylib không có dấu tiếng Việt).
#pragma once
#include "raylib.h"

// relPath: đường dẫn tương đối, ví dụ "assets/fonts/NotoSans-Regular.ttf".
// Tìm cạnh file exe trước, sau đó tới thư mục hiện tại. Không tìm thấy -> font mặc định.
Font loadVietnameseFont(const char* relPath, int size);
