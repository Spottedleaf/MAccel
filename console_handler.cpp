
#include <cstdio>
#include <atomic>

#include <stdarg.h>
#include <malloc.h>

#include <Windows.h>

#include "console_handler.h"

/* Use C++ for atomic */

static std::atomic_uint16_t last_line_y;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void console_set_nextline_number(const uint16_t n) {
    atomic_store(&last_line_y, n);
}

/* For this function, ignore newline */
int vprintf_to_console(const COORD pos, const char *format, va_list args) {
    char buff[1024];

    const int len = vsnprintf(buff, sizeof(buff), format, args);

    if (len >= sizeof(buff)) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return -1;
    } else if (len < 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    /* vsnprintf handles va_end */

    CHAR_INFO writebuffer[1024];
    memset(writebuffer, 0, sizeof(*writebuffer) * len);

    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (stdout_handle == NULL) {
        return 0;
    } else if (stdout_handle == INVALID_HANDLE_VALUE) {
        return -1;
    }

    size_t write_len = (size_t)len;

    for (size_t i = 0, write_index = 0; i < (size_t)len; ++i) {
        const char c = buff[i];

        if (c == '\n' || c == '\r') {
            --write_len;
            continue;
        }

        CHAR_INFO *current = writebuffer + write_index;

        current->Char.AsciiChar = c;
        current->Attributes = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;
        ++write_index;
    }

    if (!write_len) {
        return 0;
    }

    COORD buffer_size;
    buffer_size.X = (SHORT)write_len;
    buffer_size.Y = 1;

    COORD buffer_coord;
    memset(&buffer_coord, 0, sizeof(buffer_coord));

    SMALL_RECT region;
    region.Left = (SHORT)pos.X;
    region.Top = region.Bottom = (SHORT)pos.Y;
    region.Right = (SHORT)(pos.X + write_len);

    BOOL err = WriteConsoleOutput(stdout_handle, writebuffer, buffer_size, buffer_coord, &region);
    return !err;
}

/* This function writes the line atomically, ignoring newline characters */
int vprintf_to_console_next(const char *format, va_list args) {
    uint16_t next_line = atomic_fetch_add(&last_line_y, 1);
    COORD pos;
    pos.X = 0;
    pos.Y = next_line;

    return vprintf_to_console(pos, format, args);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */