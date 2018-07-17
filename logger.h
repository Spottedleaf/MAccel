#ifndef LOGGER_H
#define LOGGER_H LOGGER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "utils.h"

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct log_raw {
    const char *format;

    struct acceleration_value unformatted;
};

DWORD init_logger(void);

void log_entry(const struct log_raw *log);

static inline void log_entry_raw(const struct acceleration_value *val) {
    struct log_raw write;
    write.format = NULL;
    write.unformatted = *val;

    /* Write to logbuffer */
    log_entry(&write);
}

static inline void log_entry_mouse_misc(const unsigned short int state, const double time) {
    struct acceleration_value write;
    write.accelx = state;
    write.time = (float) -time;

    log_entry_raw(&write);
}

static void vprintf_to_log_time(const char *format, const float time, va_list args) {
    static const size_t BUFFER_DFLSIZE = 128;

    char *buffer = (char *)malloc(BUFFER_DFLSIZE);
    int size = vsnprintf(buffer, BUFFER_DFLSIZE, format, args);

    if (size >= BUFFER_DFLSIZE) {
        buffer = (char *) realloc(buffer, size + 1);
        vsnprintf(buffer, size + 1, format, args);
    }

    struct log_raw write;
    write.format = buffer;
    write.unformatted.time = time;

    /* Write to logbuffer */

    log_entry(&write);
}

static void vprintf_to_log(const char *format, va_list args) {
    vprintf_to_log_time(format, -1.0f, args);
}

static void printf_to_log(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vprintf_to_log(format, args);
}

static void printf_to_log_time(const char *format, const float time, ...) {
    va_list args;

    va_start(args, time);
    vprintf_to_log_time(format, time, args);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOGGER_H */