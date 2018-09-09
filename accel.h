#ifndef ACCEL_H
#define ACCEL_H ACCEL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MACCEL_CONCAT(x, y) x ## y

#define MACCEL_MAKESTR(x) #x
#define MACCEL_MAKEWSTR(x) MACCEL_CONCAT(L, #x)

#define MACCEL_MAKE_VERSION(x, y, z) MACCEL_MAKESTR(x) "." MACCEL_MAKESTR(y) "." MACCEL_MAKESTR(z)
#define MACCEL_MAKE_VERSION_WIDE(x, y, z) MACCEL_MAKEWSTR(x) L"." MACCEL_MAKEWSTR(y) L"." MACCEL_MAKEWSTR(z)

#define RL_PI 3.1415926535897932384626

struct rl_accel_settings {
    double pre_scale;
    double post_scale;
    double power;

    double carry;
};

struct povohat_accel_settings {
    double sensitivity;
    double acceleration;
    double sensitivity_cap;
    double offset;
    double power;
    double pre_scale_x;
    double pre_scale_y;
    double post_scale_x;
    double post_scale_y;
    double angle_adjustment;
    double angle_snapping;
    double speed_cap;

    double carry_x;
    double carry_y;
};

struct named_double {
    const char *name;
    double value;
};

struct profile_conf {
    const char *name;
    const char *type;

    uint32_t keybind_toggle;

    uint8_t keybind_toggle_enabled;
    uint8_t invert_x;
    uint8_t invert_y;
    uint8_t use_constant_time;

    uint32_t updates_per_second;

    double default_pre_scale_x;
    double default_pre_scale_y;
    double default_post_scale_x;
    double default_post_scale_y;
    double default_power;

    double povohat_sensitivity;
    double povohat_acceleration;
    double povohat_sensitivity_cap;
    double povohat_offset;
    double povohat_power;
    double povohat_pre_scale_x;
    double povohat_pre_scale_y;
    double povohat_post_scale_x;
    double povohat_post_scale_y;
    double povohat_angle_adjustment;
    double povohat_angle_snapping;
    double povohat_speed_cap;

    struct named_double *extra;
    size_t extra_index;
    size_t extra_capacity;
};

struct accel_conf {
    //double updates_per_second;
    //double x_post_scale;
    //double y_post_scale;
    //double x_pre_scale;
    //double y_pre_scale;
    //double x_pow;
    //double y_pow;
    //double x_multiplier;
    //double y_multiplier;
    uint32_t dpi;
    uint16_t log_toggle;
    uint8_t debug;
    uint8_t debug_input;
    uint8_t readonly;
    //uint8_t assume_constant_rate;
    uint8_t logging_enabled;
    uint8_t log_toggle_mode;
};

struct acceleration_value {
    int unaccelx;
    int unaccely;
    int accelx;
    int accely;
    uint32_t id;
    float time;
};

enum acceleration_function {
    acceleration_function_default,
    acceleration_function_povohat,
    acceleration_function_custom,
};

enum acceleration_flag {
    acceleration_flag_invert_x      = (1 << 0),
    acceleration_flag_invert_y      = (1 << 1),
    acceleration_flag_keybound      = (1 << 2),
    acceleration_flag_constant_time = (1 << 3),
};

struct coordinate {
    int x;
    int y;
};

struct acceleration_profile {
    const char *name;
    void *data;
    struct coordinate (*transform_coordinate)(void *data, struct coordinate pos, const double time);

    double constant_time;

    unsigned int options;
    enum acceleration_function function_type;

    uint32_t keybind;
};

struct coordinate default_acceleration_equation(void *data, const struct coordinate pos, const double time);
struct coordinate povohat_acceleration_equation(void *data, const struct coordinate pos, const double time);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ACCEL_H */