#ifndef CONSOLE_HANDLER_H
#define CONSOLE_HANDLER_H CONSOLE_HANDLER_H

#include <stdarg.h>
#include <stdint.h>

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CONSOLE_MAX_CHARACTERS_LINE 2048

/* Non-blocking printf functions */

/*
 * printf defined in stdio.h will block if the user highlights (marks) any text in the terminal 
 * This would block the main thread and would effectively block any user input, so these functions prevent that by 
 * manually setting characters in the console 
 */

/* All functions are thread-safe (unless on conflicting positions...) */
    
void console_set_nextline_number(const uint16_t n);
void console_modify_nextline_number(const uint16_t n);

int print_to_console(const char *message, const int clear);

static int vprintf_to_console(const char *format, va_list args) {
    char message[CONSOLE_MAX_CHARACTERS_LINE + 1];

    const int len = vsnprintf(message, sizeof(message), format, args);

    if (len >= CONSOLE_MAX_CHARACTERS_LINE) {
        SetLastError(ERROR_OUTOFMEMORY);
        return -1;
    } else if (len < 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    print_to_console(message, 0);
}

static int vprintf_to_console_clear(const char *format, va_list args) {
    char message[CONSOLE_MAX_CHARACTERS_LINE + 1];

    const int len = vsnprintf(message, sizeof(message), format, args);

    if (len >= CONSOLE_MAX_CHARACTERS_LINE) {
        SetLastError(ERROR_OUTOFMEMORY);
        return -1;
    } else if (len < 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    print_to_console(message, 1);
}

static inline int printf_to_console(const char *format, ...) {
    va_list args;
    va_start(args, format);

    return vprintf_to_console(format, args);
}

static inline int printf_to_console_clear(const char *format, ...) {
    va_list args;
    va_start(args, format);

    return vprintf_to_console_clear(format, args);
}

/* */

int print_to_console_pos(COORD pos, const char *message, const int clear);

static int vprintf_to_console_pos(const COORD pos, const char *format, va_list args) {
    char message[CONSOLE_MAX_CHARACTERS_LINE + 1];

    const int len = vsnprintf(message, sizeof(message), format, args);

    if (len >= CONSOLE_MAX_CHARACTERS_LINE) {
        SetLastError(ERROR_OUTOFMEMORY);
        return -1;
    } else if (len < 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    print_to_console_pos(pos, message, 0);
}

static int vprintf_to_console_pos_clear(const COORD pos, const char *format, va_list args) {
    char message[CONSOLE_MAX_CHARACTERS_LINE + 1];

    const int len = vsnprintf(message, sizeof(message), format, args);

    if (len >= CONSOLE_MAX_CHARACTERS_LINE) {
        SetLastError(ERROR_OUTOFMEMORY);
        return -1;
    } else if (len < 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    print_to_console_pos(pos, message, 1);
}

static inline int printf_to_console_pos(const COORD pos, const char *format, ...) {
    va_list args;
    va_start(args, format);

    return vprintf_to_console_pos(pos, format, args);
}

static inline int printf_to_console_pos_clear(const COORD pos, const char *format, ...) {
    va_list args;
    va_start(args, format);

    return vprintf_to_console_pos_clear(pos, format, args);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CONSOLE_HANDLER_H */