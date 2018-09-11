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

#include "utils.h"
#include "input_handler.h"
#include "command_input.h"
#include "console_handler.h"
#include "logger.h"

#define VERSION_RELEASE 0
#define VERSION_MAJOR 9
#define VERSION_MINOR 0
#define VERSION_STR "v0.9.0"

static void init_config(struct rl_config_member **members, size_t *len) {
    const struct rl_config_member ret_temp[] = {
        rl_create_confmem_u8 ("assume-constant-rate", struct accel_conf, assume_constant_rate),
        rl_create_confmem_dbl("updates-per-second"  , struct accel_conf, updates_per_second  ),
        rl_create_confmem_u8 ("debug"               , struct accel_conf, debug               ),
        rl_create_confmem_u8 ("debug-input"         , struct accel_conf, debug_input         ),
        rl_create_confmem_u8 ("readonly"            , struct accel_conf, readonly            ),
        rl_create_confmem_dbl("x-post-scale"        , struct accel_conf, x_post_scale        ),
        rl_create_confmem_dbl("y-post-scale"        , struct accel_conf, y_post_scale        ),
        rl_create_confmem_dbl("x-pre-scale"         , struct accel_conf, x_pre_scale         ),
        rl_create_confmem_dbl("y-pre-scale"         , struct accel_conf, y_pre_scale         ),
        rl_create_confmem_dbl("x-pow"               , struct accel_conf, x_pow               ),
        rl_create_confmem_dbl("y-pow"               , struct accel_conf, y_pow               ),
        rl_create_confmem_u32("dpi"                 , struct accel_conf, dpi                 ),
        rl_create_confmem_dbl("x-multiplier"        , struct accel_conf, x_multiplier        ),
        rl_create_confmem_dbl("y-multiplier"        , struct accel_conf, y_multiplier        ),
        rl_create_confmem_u8 ("logging-enabled"     , struct accel_conf, logging_enabled     ),
        rl_create_confmem_u16("log-toggle-key"      , struct accel_conf, log_toggle          ),
        rl_create_confmem_u8 ("log-toggle-mode"     , struct accel_conf, log_toggle_mode     ),
        rl_create_confmem_u8 ("log-keyboard-input"  , struct accel_conf, log_keyboard_input  ),
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

static void setup(InterceptionContext *context, int logging) {
    *context = interception_create_context();
    interception_set_filter(*context, logging ? &is_device : &interception_is_mouse, 
        logging ? (INTERCEPTION_FILTER_MOUSE_ALL | INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP | 
                   INTERCEPTION_FILTER_KEY_E0 | INTERCEPTION_FILTER_KEY_E1) 
        : INTERCEPTION_FILTER_MOUSE_MOVE);
}

static void load_config(struct accel_conf *conf) {
    struct rl_config_member *config_format;
    size_t config_format_len;

    init_config(&config_format, &config_format_len);

    memset(conf, 0, sizeof(*conf));

    int err = rl_read_config(conf, "accel_settings.txt", config_format, config_format_len);
    if (err) {
        printf("Config read error: errno: %i, err: %i\n", errno, err);
        term(EXIT_FAILURE);
    }

    free(config_format);
}

int main(void) {
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    system("title Accel" VERSION_STR);

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
        printf("\n\n\n\n\n");
        printf("Version: " VERSION_STR "\n");
        printf("Allow local debug input: %s\n", (conf.debug_input ? "true" : "false"));
        printf("Read-only: %s\n", (conf.readonly ? "true" : "false"));
        printf("Assume constant refresh-rate: %s\n", (conf.assume_constant_rate ? "true" : "false"));
        if (conf.assume_constant_rate) {
            printf("Constant rate: %.1f updates per second, %.3f ms per update\n", conf.updates_per_second, rate);
        }
        printf("X Power: %.3f, Y Power: %.3f\n", settings[0].power, settings[1].power);
        printf("X Pre-Scale: %.3f, Y Pre-Scale: %.3f\n", settings[0].pre_scale, settings[1].pre_scale);
        printf("X Post-Scale: %.3f, Y Post-Scale: %.3f\n", settings[0].post_scale, settings[1].post_scale);
        printf("X Multiplier: %.6f, Y Multiplier: %.6f\n", conf.x_multiplier, conf.y_multiplier);
        printf("DPI: %I32u\n", conf.dpi);
        printf("Logging Enabled: %s\n", (conf.logging_enabled ? "true" : "false"));
        if (conf.logging_enabled) {
            const char *toggle_mode;
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
            default:
                toggle_mode = "Invalid mode";
                break;
            }
            printf("Logging mode: %s\n", toggle_mode);
        }
        printf("Command: ");
    }

    /* Apply the modifiers */
    settings[0].post_scale *= conf.x_multiplier;
    settings[0].pre_scale *= conf.x_multiplier;

    settings[1].post_scale *= conf.y_multiplier;
    settings[1].pre_scale *= conf.y_multiplier;

    InterceptionContext context;
    InterceptionDevice device;
    InterceptionStroke stroke;
    
    LARGE_INTEGER timer_freq;
    LARGE_INTEGER prev_time, curr_time, log_prev_time, log_curr_time;

    CONSOLE_SCREEN_BUFFER_INFO cursor_info;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);

    console_set_nextline_number(cursor_info.dwCursorPosition.Y + 3);

    COORD log_toggle_line;
    log_toggle_line.X = 0;
    log_toggle_line.Y = cursor_info.dwCursorPosition.Y + 2;

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
        const DWORD err = init_logger();
        if (err) {
            conf.logging_enabled = 0;
        }
    }

    TIMECAPS timings;

    timeGetDevCaps(&timings, sizeof(timings));

    timeBeginPeriod(timings.wPeriodMin);

    /* Do not listen to keyboard if log is always on or is disabled */
    setup(&context, conf.logging_enabled && conf.log_toggle_mode != 2);
    
    QueryPerformanceFrequency(&timer_freq);
    QueryPerformanceCounter(&prev_time);
    log_prev_time = prev_time;

    uint32_t current_id = 0;
    
    double time;
    double real_time;

    int do_log = conf.log_toggle_mode == 2 ? 1 : 0;

    if (conf.logging_enabled) {
        printf_to_console(log_toggle_line, "LOG TOGGLE: %3s", do_log ? "ON" : "OFF");
    }

    while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0) {
        InterceptionKeyStroke *key = (InterceptionKeyStroke *)&stroke;
        InterceptionMouseStroke *coords = (InterceptionMouseStroke *)&stroke;

        if (conf.logging_enabled && interception_is_keyboard(device)) {
            interception_send(context, device, &stroke, 1);
            if (key->code != conf.log_toggle) {
                continue;
            }

            if (!conf.log_toggle_mode) {
                int old = do_log;
                do_log = (1 ^ (key->state & 1));
                if (old == do_log) {
                    continue;
                }
                if (conf.debug) {
                    printf_to_console(log_toggle_line, "LOG TOGGLE: %3s", do_log ? "ON" : "OFF");
                }
                QueryPerformanceCounter(&log_curr_time);
                printf_to_log_time("LOG TOGGLE %s", (float)(((log_curr_time.QuadPart - log_prev_time.QuadPart) * 1000) / (double)(timer_freq.QuadPart)),
                    ((do_log) ? "ON" : "OFF"));
                log_prev_time = log_curr_time;
            } else {
                if (key->state != INTERCEPTION_KEY_UP) {
                    continue;
                }
                do_log ^= 1;
                if (conf.debug) {
                    printf_to_console(log_toggle_line, "LOG TOGGLE: %3s", do_log ? "ON" : "OFF");
                }
                QueryPerformanceCounter(&log_curr_time);
                printf_to_log_time("LOG TOGGLE %s", (float)(((log_curr_time.QuadPart - log_prev_time.QuadPart) * 1000) / (double)(timer_freq.QuadPart)),
                    (do_log ? "ON" : "OFF"));
                log_prev_time = log_curr_time;
            }
            continue;
        }

        if (coords->state != INTERCEPTION_MOUSE_MOVE_RELATIVE) {
            /* Mouse button input */
            interception_send(context, device, &stroke, 1);
            if (!conf.logging_enabled || !do_log) {
                continue;
            }
            QueryPerformanceCounter(&log_curr_time);
            log_entry_mouse_misc(coords->state, ((log_curr_time.QuadPart - log_prev_time.QuadPart) * 1000) / (double)(timer_freq.QuadPart));
            log_prev_time = log_curr_time;
            continue;
        }

        QueryPerformanceCounter(&curr_time);
        real_time = ((curr_time.QuadPart - prev_time.QuadPart) * 1000) / (double)(timer_freq.QuadPart);

		if (real_time < 0.0625) {
			real_time = 0.0625;
		}

        if (!conf.assume_constant_rate) {
            time = fmin(real_time, 200.0);
        } else {
            time = rate;
        }
        
        const int x = coords->x;
        const int y = coords->y;

        int newx;
        int newy;

        if (!conf.readonly) {
            newx = rl_perform_accel(&settings[0], x, time, &settings[0].carry);
            newy = rl_perform_accel(&settings[1], y, time, &settings[1].carry);
        } else {
            newx = x;
            newy = y;
        }

        coords->x = newx;
        coords->y = newy;

        interception_send(context, device, &stroke, 1);

        prev_time = curr_time;

        struct acceleration_value element;
        element.time = (float) real_time;
        element.unaccelx = (int16_t)x;
        element.unaccely = (int16_t)y;
        element.accelx = (int16_t)newx;
        element.accely = (int16_t)newy;
        element.id = current_id++;

        if (conf.debug) {
            input_handler_write(&element);
        }

        if (conf.logging_enabled && do_log) {
            log_curr_time = curr_time;
            element.time = (float) (((log_curr_time.QuadPart - log_prev_time.QuadPart) * 1000) / (double)(timer_freq.QuadPart));
            log_prev_time = log_curr_time;
            log_entry_raw(&element);
        }
    }
    timeEndPeriod(timings.wPeriodMin);
    return EXIT_SUCCESS;
}