#ifndef UTILS_H
#define UTILS_H UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>

#include <io.h>
#include <Windows.h>

#include "console_handler.h"
#include "accel.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


static inline void *arraycopy(void *__restrict dst, const size_t dst_off, const void *__restrict src,
    const size_t src_off, const size_t nitems, const size_t elem_sizeof) {

    return memcpy(((char *__restrict)dst) + (dst_off * elem_sizeof), 
        ((const char *__restrict)src) + (src_off * elem_sizeof), nitems * elem_sizeof);
}

/* Copies the sign of the first operand to the second */
/* If the second operand is 0, 0 is returned */
static inline int32_t copy_sign(const int32_t original, const int32_t value) {
    const int32_t change = (original ^ value) >> (sizeof(original) * CHAR_BIT - 1);

    return (value ^ change) - change;
}

/* Changes the sign of the first operand to the opposite of the second */
/* If the first operand is 0, then the sign of the returned value is positive */
/* If the first operand is INT32_MIN, then the sign of the returned value is negative */
/* If the second operand is 0, then 0 is returned */
static inline int32_t invert_sign(const int32_t original, const int32_t value) {
    const int32_t change = (-original ^ value) >> (sizeof(original) * CHAR_BIT - 1);

    return (value ^ change) - change;
}

static void term(const int code) {
    printf_to_console("Press enter to exit...");
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

    const size_t read = (size_t)_read(0, buffer, (int) (len - 1));

    char *ptr;

    if (ptr = (char *) memchr(buffer, '\n', read)) {
        *ptr = '\0';
        return ptr - buffer;
    }
    if (ptr = (char *) memchr(buffer, '\r', read)) {
        *ptr = '\0';
        return ptr - buffer;
    }
    if (ptr = (char *) memchr(buffer, '\0', read)) {
        return ptr - buffer;
    }

    buffer[read] = '\0';
    return read;
}

void protect_thread(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UTILS_H */
