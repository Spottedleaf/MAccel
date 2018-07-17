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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct rl_accel_settings {
    double pre_scale;
    double post_scale;
    double power;

    double carry;
};

struct accel_conf {
    double updates_per_second;
    double x_post_scale;
    double y_post_scale;
    double x_pre_scale;
    double y_pre_scale;
    double x_pow;
    double y_pow;
    double x_multiplier;
    double y_multiplier;
    uint32_t dpi;
    uint16_t log_toggle;
    uint8_t debug;
    uint8_t debug_input;
    uint8_t readonly;
    uint8_t assume_constant_rate;
    uint8_t logging_enabled;
    uint8_t log_toggle_mode;
    uint8_t log_keyboard_input;
};

struct acceleration_value {
    int16_t unaccelx;
    int16_t unaccely;
    int16_t accelx;
    int16_t accely;
    uint32_t id;
    float time; /* time is float to fit this struct into 16 bytes */
};

static inline int is_valid_accel_value(struct acceleration_value *value) {
    return isfinite(value->time);
}

static void term(const int code) {
    printf_to_console_next("Press enter to exit...");
    getc(stdin);
    exit(code);
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

static int rl_perform_accel(struct rl_accel_settings *settings, const int old, const double time, double *carry) {
    double v = (((double)old) / time) * settings->pre_scale;
    if (old < 0) {
        v = -v;
    }

    double accel = pow(v, settings->power) + 1;
    double f = fma(old * accel, settings->post_scale, *carry);

    *carry = modf(f, &f);

    return (int)f;
}

static double u64_to_double(const uint64_t x) {
    double ret;
    memcpy(&ret, &x, sizeof(ret));
    return ret;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UTILS_H */
