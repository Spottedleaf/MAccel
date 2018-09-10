#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include <math.h>

#include <Windows.h>

#include "interception\interception.h"
#include "rustedleaf\rl_config.h"
#include "rustedleaf\rl_cdrain_queue.h"

#include "utils.h"
#include "input_handler.h"
#include "command_input.h"
#include "console_handler.h"
#include "logger.h"

#define MACCEL_VERSION_RELEASE 1
#define MACCEL_VERSION_MAJOR 0
#define MACCEL_VERSION_MINOR 0

#define MACCEL_VERSION_STR MACCEL_MAKE_VERSION(MACCEL_VERSION_RELEASE, MACCEL_VERSION_MAJOR, MACCEL_VERSION_MINOR)
#define MACCEL_VERSION_WSTR MACCEL_MAKE_VERSION_WIDE(MACCEL_VERSION_RELEASE, MACCEL_VERSION_MAJOR, MACCEL_VERSION_MINOR)

static void init_config(struct rl_config_member **members, size_t *len) {
    const struct rl_config_member ret_temp[] = {
        //rl_create_confmem_u8 ("assume-constant-rate", struct accel_conf, assume_constant_rate),
        //rl_create_confmem_dbl("updates-per-second"  , struct accel_conf, updates_per_second  ),
        rl_create_confmem_u8 ("debug"               , struct accel_conf, debug               ),
        rl_create_confmem_u8 ("debug-input"         , struct accel_conf, debug_input         ),
        rl_create_confmem_u8 ("readonly"            , struct accel_conf, readonly            ),
        //rl_create_confmem_dbl("x-post-scale"        , struct accel_conf, x_post_scale        ),
        //rl_create_confmem_dbl("y-post-scale"        , struct accel_conf, y_post_scale        ),
        //rl_create_confmem_dbl("x-pre-scale"         , struct accel_conf, x_pre_scale         ),
        //rl_create_confmem_dbl("y-pre-scale"         , struct accel_conf, y_pre_scale         ),
        //rl_create_confmem_dbl("x-pow"               , struct accel_conf, x_pow               ),
        //rl_create_confmem_dbl("y-pow"               , struct accel_conf, y_pow               ),
        rl_create_confmem_u32("dpi"                 , struct accel_conf, dpi                 ),
        //rl_create_confmem_dbl("x-multiplier"        , struct accel_conf, x_multiplier        ),
        //rl_create_confmem_dbl("y-multiplier"        , struct accel_conf, y_multiplier        ),
        rl_create_confmem_u8 ("logging-enabled"     , struct accel_conf, logging_enabled     ),
        rl_create_confmem_u16("log-toggle-key"      , struct accel_conf, log_toggle          ),
        rl_create_confmem_u8 ("log-toggle-mode"     , struct accel_conf, log_toggle_mode     ),
    };
    const size_t size = sizeof(ret_temp) / sizeof(*ret_temp);
    struct rl_config_member *ret = calloc(size, sizeof(*ret_temp));
    if (!ret) {
        term(EXIT_FAILURE);
    }
    *len = size;
    *members = ret;
    memcpy(ret, ret_temp, sizeof(ret_temp));
}

int is_device(const InterceptionDevice dev) {
    return !interception_is_invalid(dev);
}

static void setup(InterceptionContext *context, const int logging) {
    *context = interception_create_context();
    interception_set_filter(*context, &interception_is_mouse, logging ? 
        INTERCEPTION_FILTER_MOUSE_ALL : INTERCEPTION_FILTER_MOUSE_MOVE);
}

static void load_config(struct accel_conf *conf) {
    struct rl_config_member *config_format;
    size_t config_format_len;

    init_config(&config_format, &config_format_len);

    memset(conf, 0, sizeof(*conf));

    int err = rl_read_config(conf, "accel_settings.txt", config_format, config_format_len, NULL); //
    if (err) {
        printf("Config read error: errno: %i, err: %i\n", errno, err);
        term(EXIT_FAILURE);
    }

    free(config_format);
}

int main(void) {
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    protect_thread();
    SetConsoleTitle(L"MAccel v" MACCEL_VERSION_WSTR);

    /* load config */

    struct accel_conf conf;

    load_config(&conf);

    const double rate = 1000.0 / conf.updates_per_second;
    struct rl_accel_settings settings[2];
    /* Init settings X */
    settings[0].pre_scale = conf.x_pre_scale;
    settings[0].post_scale = conf.x_post_scale;
    settings[0].power = conf.x_pow;

    settings[0].carry = 0.0;

    /* Init settings Y */
    settings[1].pre_scale = conf.y_pre_scale;
    settings[1].post_scale = conf.y_post_scale;
    settings[1].power = conf.y_pow;

    settings[1].carry = 0.0;

    if (conf.debug) {
        printf_to_console("\n\n\n\n\n");
        printf_to_console("Version: " MACCEL_VERSION_STR "\n");
        printf_to_console("Allow local debug input: %s\n", (conf.debug_input ? "true" : "false"));
        printf_to_console("Read-only: %s\n", (conf.readonly ? "true" : "false"));
        printf_to_console("Assume constant refresh-rate: %s\n", (conf.assume_constant_rate ? "true" : "false"));
        if (conf.assume_constant_rate) {
            printf("Constant rate: %.1f updates per second, %.3f ms per update\n", conf.updates_per_second, rate);
        }
        printf_to_console("X Power: %.3f, Y Power: %.3f\n", settings[0].power, settings[1].power);
        printf_to_console("X Pre-Scale: %.3f, Y Pre-Scale: %.3f\n", settings[0].pre_scale, settings[1].pre_scale);
        printf_to_console("X Post-Scale: %.3f, Y Post-Scale: %.3f\n", settings[0].post_scale, settings[1].post_scale);
        printf_to_console("X Multiplier: %.6f, Y Multiplier: %.6f\n", conf.x_multiplier, conf.y_multiplier);
        printf_to_console("DPI: %I32u\n", conf.dpi);
        if (conf.logging_enabled) {
            const char *toggle_mode = NULL;
            switch (conf.log_toggle_mode) {
            case 0:
                toggle_mode = "Push to enable";
                break;
            case 1:
                toggle_mode = "Press to toggle";
                break;
            case 2:
                toggle_mode = "Always on";
                break;
            }
            if (conf.log_toggle_mode <= 2) {
                printf_to_console("Logging Enabled: true\n");
                printf_to_console("Logging mode: %s\n", toggle_mode);
            } else {
                printf_to_console("Logging Enabled: false\n");
                printf_to_console("Invalid logging mode %u, disabling logging\n", conf.log_toggle_mode);
                conf.logging_enabled = 0;
            }
        } else {
            printf_to_console("Logging Enabled: false\n");
        }
        printf_to_console("Command: ");
    }

    /* Apply the modifiers */
    settings[0].post_scale *= conf.x_multiplier;
    settings[0].pre_scale *= conf.x_multiplier;

    settings[1].post_scale *= conf.y_multiplier;
    settings[1].pre_scale *= conf.y_multiplier;

    InterceptionContext context;
    InterceptionDevice device;
    
    double timer_freq = get_performance_frequency_ms();

    LARGE_INTEGER prev_time, curr_time, log_curr_time;

    CONSOLE_SCREEN_BUFFER_INFO cursor_info;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);

    console_modify_nextline_number(3);

    if (conf.debug) {
        DWORD err = init_input_handler(conf.dpi);
        if (err) {
            conf.debug_input = conf.debug = 0;
        } else if (conf.debug_input) {
            err = command_input_init(cursor_info.dwCursorPosition);
            if (err) {
                conf.debug_input = conf.debug = 0;
            }
        }
    }

    if (conf.logging_enabled) {
        const DWORD err = init_logger(conf.log_toggle_mode, conf.log_toggle, cursor_info.dwCursorPosition.Y + 2, conf.debug);
        if (err) {
            conf.logging_enabled = 0;
        }
    }

    TIMECAPS timings;

    timeGetDevCaps(&timings, sizeof(timings));
    timeBeginPeriod(timings.wPeriodMin);

    setup(&context, conf.logging_enabled);
    
    QueryPerformanceCounter(&prev_time);

    if (conf.logging_enabled) {
        log_start_time(prev_time);
        log_change_status(prev_time, conf.logging_enabled == 2 ? 1 : 0);
    }

    uint32_t current_id = 0;
    
    double time;
    double real_time;

    InterceptionStroke strokes[32];
    int read;

    while ((read = interception_receive(context, device = interception_wait(context), strokes, sizeof(strokes) / sizeof(*strokes))) > 0) {
        for (size_t i = 0; i < read; ++i) {
            InterceptionMouseStroke *coords = (InterceptionMouseStroke *) (strokes + i);

            if (coords->state != INTERCEPTION_MOUSE_MOVE_RELATIVE) {
                /* Mouse button input */
                interception_send(context, device, (InterceptionStroke *) coords, 1);
                QueryPerformanceCounter(&log_curr_time);
                log_mouse_misc_input(coords->state, log_curr_time);
                continue;
            }

            QueryPerformanceCounter(&curr_time);
            real_time = (curr_time.QuadPart - prev_time.QuadPart) / timer_freq;

            if (real_time < 0.01) {
                real_time = 0.01;
            }

            if (!conf.assume_constant_rate) {
                if (real_time > 200.0) {
                    time = 200.0;
                } else {
                    time = real_time;
                }
            } else {
                time = rate;
            }

            const int x = coords->x;
            const int y = coords->y;

            int newx;
            int newy;

            if (!conf.readonly) {
                coords->x = newx = rl_perform_accel(&settings[0], x, time);
                coords->y = newy = rl_perform_accel(&settings[1], y, time);
            } else {
                newx = x;
                newy = y;
            }

            interception_send(context, device, (InterceptionStroke *) coords, 1);

            prev_time = curr_time;

            struct acceleration_value element;
            element.time = (float) time;
            element.unaccelx = x;
            element.unaccely = y;
            element.accelx = newx;
            element.accely = newy;
            element.id = current_id++;

            if (conf.debug) {
                input_handler_write(&element);
            }

            if (conf.logging_enabled) {
                log_curr_time = curr_time;
                log_mouse_input(&element, log_curr_time);
            }
        }
    }
    timeEndPeriod(timings.wPeriodMin);
    return EXIT_SUCCESS;
}

/* Modified variant of povohat's equation */
/* Changes: 
    - Uses the velocity along an axis for computing acceleration instead of the mouse's velocity
    - Power is one greater
    - Equation are independent per axis
    - Carrying is done via modf (this affects each individual transformation but overall it remains the same except for the last transformation)
    - Multiplier directly to the input (this is very useful for dpi conversions)
*/
static int rl_perform_accel(struct rl_accel_settings *settings, const int old, const double time) {
    const double old_d = (double) old;
    double v = (old_d / time) * settings->pre_scale;
    if (old < 0) {
        v = -v;
    }

    double accel = pow(v, settings->power) + 1;
    double next = fma(old_d * accel, settings->post_scale, settings->carry);

    settings->carry = modf(next, &next);

    return (int)next;
}

struct coordinate default_acceleration_equation(void *data, const struct coordinate pos, const double time) {
    struct rl_accel_settings *settings = data;

    struct coordinate ret;

    ret.x = rl_perform_accel(settings + 0, pos.x, time);
    ret.y = rl_perform_accel(settings + 1, pos.y, time);

    return ret;
}


/* Credit to povohat for this equation */
/* TODO: Test */
struct coordinate povohat_acceleration_equation(void *data, const struct coordinate pos, const double time) {
    struct povohat_accel_settings *settings = data;

    double x = pos.x;
    double y = pos.y;

    const double length = hypot(x, y);

    if (settings->angle_adjustment != 0.0) {
        /* TODO: Convert angle adjustment to radians at load */
        const double angle = atan2(x, y) + (settings->angle_adjustment * (RL_PI / 180.0));

        x = length * cos(angle);
        y = length * sin(angle);
    }

    if (settings->angle_snapping != 0.0) {
        /* TODO: Convert angle snapping to radians at load */
        const double angle_snap = settings->angle_snapping * (RL_PI / 180.0);
        const double angle = atan2(x, y);

        const double cosa = cos(angle);
        const double sina = sin(angle);

        if (fabs(cosa) < angle_snap) {
            x = 0;
            if (sina > 0) {
                y = length;
            } else {
                y = -length;
            }
        } else if (fabs(sina) < angle_snap) {
            y = 0;
            if (cosa > 0) {
                x = length;
            } else {
                y = -length;
            }
        }
    }

    x *= settings->pre_scale_x;
    y *= settings->pre_scale_y;

    if (settings->speed_cap != 0.0 && length > settings->speed_cap) {
        x *= (settings->speed_cap / length);
        y *= (settings->speed_cap / length);
    }

    double accel = settings->sensitivity;

    if (settings->acceleration > 0.0) {
        const double speed = (length / time) - settings->offset;
        if (speed > 0.0) {
            accel += pow(speed, settings->power);
        }

        if (settings->sensitivity_cap > 0.0 && accel > settings->sensitivity_cap) {
            accel = settings->sensitivity_cap;
        }
    }

    accel /= settings->sensitivity;

    x = fma(x * accel, settings->post_scale_x, settings->carry_x);
    y = fma(y * accel, settings->post_scale_y, settings->carry_y);

    const double floorx = floor(x);
    const double floory = floor(y);

    settings->carry_x = x - floorx;
    settings->carry_y = y - floory;

    return (struct coordinate) { .x = (int) floorx, .y = (int) floory };
}