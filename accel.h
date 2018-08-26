#ifndef ACCEL_H
#define ACCEL_H ACCEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MACCEL_CONCAT(x, y) x ## y

#define MACCEL_MAKESTR(x) #x
#define MACCEL_MAKEWSTR(x) MACCEL_CONCAT(L, #x)

#define MACCEL_MAKE_VERSION(x, y, z) MACCEL_MAKESTR(x) "." MACCEL_MAKESTR(y) "." MACCEL_MAKESTR(z)
#define MACCEL_MAKE_VERSION_WIDE(x, y, z) MACCEL_MAKEWSTR(x) L"." MACCEL_MAKEWSTR(y) L"." MACCEL_MAKEWSTR(z)

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
};

struct acceleration_value {
    int16_t unaccelx;
    int16_t unaccely;
    int16_t accelx;
    int16_t accely;
    uint32_t id;
    float time; /* time is float to fit this struct into 16 bytes */
};

static int rl_perform_accel(struct rl_accel_settings *settings, const int old, const double time) {
    double v = (((double)old) / time) * settings->pre_scale;
    if (old < 0) {
        v = -v;
    }

    double accel = pow(v, settings->power) + 1;
    double next = fma(old * accel, settings->post_scale, settings->carry);

    settings->carry = modf(next, &next);

    return (int)next;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ACCEL_H */