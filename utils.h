#ifndef UTILS_H
#define UTILS_H UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include <io.h>
#include <Windows.h>

#include "console_handler.h"
#include "accel.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static void term(const int code) {
    printf_to_console_next("Press enter to exit...");
    getc(stdin);
    exit(code);
}

static inline double get_performance_frequency_ms(void) {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return freq.QuadPart / 1000.0;
}

static size_t read_stdin(char *buffer, const size_t len) {
    /* \r or \n or \0 are considered end of line */

    const size_t read = (size_t)_read(0, buffer, (int)(len - 1));

    char *null_ptr;
    char *newline_ptr;
    char *carrage_ptr;

    if (null_ptr = (char *) memchr(buffer, '\0', read)) {
        *null_ptr = '\0';
        return null_ptr - buffer;
    }
    if (newline_ptr = (char *) memchr(buffer, '\n', read)) {
        *newline_ptr = '\0';
        return newline_ptr - buffer;
    }
    if (carrage_ptr = (char *) memchr(buffer, '\r', read)) {
        *carrage_ptr = '\0';
        return carrage_ptr - buffer;
    }
    buffer[read] = '\0';
    return read;
}

static inline double u64_to_double(const uint64_t x) {
    double ret;
    memcpy(&ret, &x, sizeof(ret));
    return ret;
}

void protect_thread(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UTILS_H */
