
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
    std::atomic_store(&last_line_y, n);
}

void console_modify_nextline_number(const uint16_t n) {
    std::atomic_fetch_add(&last_line_y, n);
}

/* Prints to a single line with an option to clear */
int print_to_console_pos_internal(const COORD pos, const HANDLE to, const int clear, const char *message) {
    if (!to || to == INVALID_HANDLE_VALUE) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    if (pos.Y >= CONSOLE_MAX_CHARACTERS_LINE) {
        return 0;
    }
    
    const size_t max = CONSOLE_MAX_CHARACTERS_LINE - pos.Y;

    size_t len = strlen(message);
    if (len > max) {
        len = max;
    }

    CHAR_INFO writebuffer[CONSOLE_MAX_CHARACTERS_LINE];
    memset(writebuffer, 0, sizeof(writebuffer));

    for (size_t i = 0; i < len; ++i) {
        CHAR_INFO *current = writebuffer + i;

        current->Char.AsciiChar = message[i];
        current->Attributes = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;
    }
    if (clear) {
        CHAR_INFO reference;
        memset(&reference, 0, sizeof(reference));
        reference.Char.AsciiChar = ' ';
        reference.Attributes = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;
        for (size_t i = len; i < max; ++i) {
            writebuffer[i] = reference;
        }
        len = max;
    }

    if (!len) {
        return 0;
    }

    COORD buffer_size;
    buffer_size.X = (SHORT)len;
    buffer_size.Y = 1;

    COORD buffer_coord;
    memset(&buffer_coord, 0, sizeof(buffer_coord));

    SMALL_RECT region;
    region.Left = (SHORT)pos.X;
    region.Top = region.Bottom = (SHORT)pos.Y;
    region.Right = (SHORT)(pos.X + len);


    const BOOL err = WriteConsoleOutput(to, writebuffer, buffer_size, buffer_coord, &region);
    return err ? len : -1;
}

static size_t split_message(const char *message, const char **buff, const char splitter, const size_t max) {
    size_t i;
    for (i = 0; i < max; ++i) {
        const char *index = strchr(message, splitter);
        if (!index) {
            break;
        }

        const ptrdiff_t len = message - index;

        char *line = (char *) malloc(len + 1);
        if (!line) {
            // TODO
        }
        memcpy(line, message, len);
        line[len] = '\0';

        buff[i] = line;

        message = index + 1;
    }

    return i;
}

static void free_split_messages(const char **messages, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        free((void *) messages[i]);
    }
}

int print_to_console_pos(COORD pos, const char *message, const int clear) {
    char *messages[CONSOLE_MAX_CHARACTERS_LINE];

    const size_t lines = split_message(message, messages, '\n', sizeof(messages) / sizeof(*messages));

    const HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    for (size_t i = 0; i < lines; ++i, ++pos.Y) {
        print_to_console_pos_internal(pos, out, clear, messages[i]);
    }
}

int print_to_console(const char *message, const int clear) {
    char *messages[CONSOLE_MAX_CHARACTERS_LINE];

    const size_t lines = split_message(message, messages, '\n', sizeof(messages) / sizeof(*messages));

    const uint16_t start = std::atomic_fetch_add(&last_line_y, lines);

    const HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    COORD pos;
    pos.X = 0;
    pos.Y = start;
    for (size_t i = 0; i < lines; ++i, ++pos.Y) {
        print_to_console_pos_internal(pos, out, clear, messages[i]);
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */