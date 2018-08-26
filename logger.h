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

enum entry_type {
    ENTRY_TYPE_MOUSE_MOVEMENT,
    ENTRY_TYPE_MOUSE_MISC,
    ENTRY_TYPE_MESSAGE,
    ENTRY_TYPE_START_TIME,
    ENTRY_TYPE_CHANGE_LOG_STATUS,
};

struct log_entry {
    const char *message;

    LARGE_INTEGER timestamp;

    uint32_t id;

    int16_t accelx;
    int16_t accely;
    int16_t unaccelx;
    int16_t unaccely;

    enum entry_type type;
};

struct log_raw {
    const char *format;

    struct acceleration_value unformatted;
};

DWORD init_logger(const uint8_t logmode, const uint16_t log_toggle, const uint16_t toggle_line, const uint8_t debug);

void push_entry(const struct log_entry *log);

static void log_start_time(const LARGE_INTEGER time) {
    struct log_entry entry;
    entry.timestamp = time;
    entry.type = ENTRY_TYPE_START_TIME;

    push_entry(&entry);
}

static void log_change_status(const LARGE_INTEGER time, int status) {
    struct log_entry entry;
    entry.timestamp = time;
    entry.accelx = (int16_t) status;
    entry.type = ENTRY_TYPE_CHANGE_LOG_STATUS;

    push_entry(&entry);
}

static void log_mouse_input(const struct acceleration_value *value, const LARGE_INTEGER time) {
    struct log_entry entry;
    entry.timestamp = time;
    entry.id = value->id;
    entry.accelx = value->accelx;
    entry.accely = value->accely;
    entry.unaccelx = value->unaccelx;
    entry.unaccely = value->unaccely;
    entry.type = ENTRY_TYPE_MOUSE_MOVEMENT;

    push_entry(&entry);
}

static void log_mouse_misc_input(const int16_t state, const LARGE_INTEGER time) {
    struct log_entry entry;
    entry.timestamp = time;
    entry.accelx = state;
    entry.type = ENTRY_TYPE_MOUSE_MISC;

    push_entry(&entry);
}

static void vprintf_to_log(const char *format, const LARGE_INTEGER time, va_list args) {
    static const size_t BUFFER_DFLSIZE = 128;

    char *buffer = (char *)malloc(BUFFER_DFLSIZE);
    int size = vsnprintf(buffer, BUFFER_DFLSIZE, format, args);

    if (size >= BUFFER_DFLSIZE) {
        buffer = (char *) realloc(buffer, size + 1);
        vsnprintf(buffer, size + 1, format, args);
    }

    /* Write to logbuffer */

    struct log_entry entry;
    entry.timestamp = time;
    entry.message = buffer;
    entry.type = ENTRY_TYPE_MESSAGE;

    push_entry(&entry);
}

static void printf_to_log(const char *format, const LARGE_INTEGER time, ...) {
    va_list args;

    va_start(args, format);
    vprintf_to_log(format, time, args);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOGGER_H */