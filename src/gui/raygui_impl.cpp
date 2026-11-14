// Phần cài đặt của raygui (thư viện header-only) - biên dịch đúng một lần, tắt cảnh báo của thư viện.
#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
