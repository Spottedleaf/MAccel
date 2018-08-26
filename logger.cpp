#include <stdint.h>
#include <stdlib.h>

#include <atomic>

#include <Windows.h>

#include "interception/interception.h"

#include "rustedleaf/rl_cdrain_queue.hpp"

#include "console_handler.h"
#include "logger.h"
#include "utils.h"

#define LOG_BUFFER_SIZE 8192

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct logging_options {
    uint16_t toggle;
    uint16_t line;
    uint8_t mode;
    uint8_t debug;
};

static const char CSV_HEADER[] = "TYPE,INFO,ID,TIME (ms),ORIGX,ORIGY,ACCELX,ACCELY\n";
static const char MOUSE_MISC_INPUT_FORMAT[] = "MISC INPUT,%s,,%.3e\n";
static const char DEBUG_MESSAGE_FORMAT[] = "DEBUG MSG,%s,,%.3e\n";
static const char MOUSE_INPUT_FORMAT[] = "MOUSE MOVE,,%u,%.3e,%+i,%+i,%+i,%+i\n";

static struct rl_cdrain_queue queue;

static inline const char *mouse_state(const int value) {
    switch (value) {
    case INTERCEPTION_MOUSE_BUTTON_1_DOWN:
        return "LEFT BUTTON DOWN";
    case INTERCEPTION_MOUSE_BUTTON_1_UP:
        return "LEFT BUTTON UP";
    
    case INTERCEPTION_MOUSE_BUTTON_2_DOWN:
        return "RIGHT BUTTON DOWN";
    case INTERCEPTION_MOUSE_BUTTON_2_UP:
        return "RIGHT BUTTON UP";

    case INTERCEPTION_MOUSE_BUTTON_3_DOWN:
        return "MIDDLE BUTTON DOWN";
    case INTERCEPTION_MOUSE_BUTTON_3_UP:
        return "MIDDLE BUTTON UP";

    case INTERCEPTION_MOUSE_BUTTON_4_DOWN:
        return "BUTTON 4 DOWN";
    case INTERCEPTION_MOUSE_BUTTON_4_UP:
        return "BUTTON 4 UP";

    case INTERCEPTION_MOUSE_BUTTON_5_DOWN:
        return "BUTTON 5 DOWN";
    case INTERCEPTION_MOUSE_BUTTON_5_UP:
        return "BUTTON 5 UP";


    case INTERCEPTION_MOUSE_HWHEEL:
        return "HORIZONTAL SCROLL";
    case INTERCEPTION_MOUSE_WHEEL:
        return "VERTICAL SCROLL";

    }

    return "UNKNOWN INPUT";
}
void push_entry(const struct log_entry *log) {
    rl_cdrain_queue_add(&queue, log, 1, sizeof(*log), 0);
}

static DWORD get_time(char *buffer, const size_t len) {
    SYSTEMTIME curr_time;
    GetLocalTime(&curr_time);

    TCHAR time_buffer[32];
    int err = GetTimeFormat(MAKELCID(0x0C09, SORT_DEFAULT), 0, NULL, L"HH-mm-ss", time_buffer, (int)(sizeof(time_buffer) / sizeof(*time_buffer)));

    if (!err) {
        return GetLastError();
    }

    TCHAR temp[64];

    int length = _snwprintf_s(temp, sizeof(temp) / sizeof(*temp), L"%.4i-%.2i-%.2i_%ls",
        curr_time.wYear, curr_time.wMonth, curr_time.wDay,
        time_buffer);

    if (length >= (sizeof(temp) / sizeof(*temp))) {
        return ERROR_OUTOFMEMORY;
    }

    for (size_t i = 0; i < (length + 1); ++i) {
        *(buffer + i) = (char)temp[i];
    }

    return ERROR_SUCCESS;
}

static DWORD get_filename(const char *format, char *buffer, const size_t bufferlen) {
    char time[32];
    DWORD err = get_time(time, sizeof(time));

    if (err) {
        return err;
    }

    int len = snprintf(buffer, bufferlen, format, time);

    if (len >= bufferlen) {
        return ERROR_OUTOFMEMORY;
    }

    return ERROR_SUCCESS;
}

void logging_thread(void *param) {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    protect_thread();

    char filename[64];

    DWORD terr = get_filename("MAccel-log %s.csv", filename, sizeof(filename));

    if (terr) {
        printf_to_console("Failed to create timestamp, errno %I32u\n", terr);
        return;
    }

    printf_to_console("Logging to file %s\n", filename);

    FILE *logfile;
    errno_t err = fopen_s(&logfile, filename, "w");
    if (err) {
        printf_to_console("Failed to open file \"%s\" for writing! (errno %i)\n", filename, err);
        return;
    }

    /* Init header */

    fprintf(logfile, "_%s", CSV_HEADER); /* Prefix with _ because Excel */

    static struct log_entry log_entries[4096];

    const double perf_freq = get_performance_frequency_ms();
    LARGE_INTEGER prev_time;

    int do_log = 1;

    for (;;) {
        const size_t items = rl_cdrain_queue_drain(&queue, log_entries, 
            sizeof(log_entries) / sizeof(*log_entries), sizeof(*log_entries), 0);

        if (!items) {
            Sleep(500);
            continue;
        }

        for (size_t i = 0; i < items; ++i) {
            struct log_entry *curr = log_entries + i;

            if (curr->type == ENTRY_TYPE_START_TIME) {
                prev_time = curr->timestamp;
                continue;
            }
            
            const double time = (curr->timestamp.QuadPart - prev_time.QuadPart) / perf_freq;

            if (curr->type == ENTRY_TYPE_CHANGE_LOG_STATUS) {
                int new_log = (int) curr->accelx;
                if (new_log == do_log) {
                    continue;
                }
                do_log ^= 1;

                if (new_log) {
                    fprintf(logfile, DEBUG_MESSAGE_FORMAT, "Logging toggled  ON", time);
                } else {
                    fprintf(logfile, DEBUG_MESSAGE_FORMAT, "Logging toggled OFF", time);
                }

                continue;
            }

            if (!do_log) {
                continue;
            }

            prev_time = curr->timestamp;

            if (curr->type == ENTRY_TYPE_MESSAGE) {
                fprintf(logfile, DEBUG_MESSAGE_FORMAT, curr->message, time);

                free((void *)curr->message);
                continue;
            }

            if (curr->type == ENTRY_TYPE_MOUSE_MISC) {
                const char *state = mouse_state(curr->accelx);
                if (!state) {
                    continue;
                }
                fprintf(logfile, MOUSE_MISC_INPUT_FORMAT, state, time);
                continue;
            }

            if (curr->type == ENTRY_TYPE_MOUSE_MOVEMENT) {
                fprintf(logfile, MOUSE_INPUT_FORMAT, curr->id,
                    time, curr->unaccelx, curr->unaccely,
                    curr->accelx, curr->accely);
            }
        }

        if (items != (sizeof(log_entries) / sizeof(*log_entries))) {
            fflush(logfile);
            Sleep(500); /* Only sleep if the read buffer was not full */
        }
    }
}

void keylistener_thread(void *param) {
    struct logging_options options = *(struct logging_options *)param;
    free(param);

    COORD debug_line;
    debug_line.X = 0;
    debug_line.Y = options.line;

    int logging_status = options.mode == 2 ? 1 : 0;

    InterceptionContext context = interception_create_context();
    InterceptionDevice device;
    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP |
        INTERCEPTION_FILTER_KEY_E0 | INTERCEPTION_FILTER_KEY_E1);

    LARGE_INTEGER curr_time;

    InterceptionStroke strokes[32];
    int read;

    while ((read = interception_receive(context, device = interception_wait(context), strokes, sizeof(strokes) / sizeof(*strokes))) > 0) {
        interception_send(context, device, strokes, read);
        for (size_t i = 0; i < read; ++i) {
            const InterceptionKeyStroke *key = (InterceptionKeyStroke *)(strokes + i);
               
            if (key->code != options.toggle) {
                continue;
            }

            if (options.mode == 1 && key->state != INTERCEPTION_KEY_UP) {
                continue;
            }

            if (options.mode == 1) {
                logging_status ^= 1;
            } else {
                int old_state = logging_status;
                logging_status = ((key->state & 1) ^ 1);
                if (logging_status == old_state) {
                    continue;
                }
            }

            QueryPerformanceCounter(&curr_time);

            const char *message;
            if (logging_status) {
                message = "Log:  ON";
            } else {
                message = "Log: OFF";
            }

            if (options.debug) {
                printf_to_console_pos(debug_line, message);
            }

            log_change_status(curr_time, logging_status);
        }
    }
}


DWORD init_logger(const uint8_t logmode, const uint16_t log_toggle, const uint16_t toggle_line, const uint8_t debug) {
    rl_cdrain_queue_init(&queue, 8192*2, sizeof(struct log_entry));
    HANDLE thr = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&logging_thread, NULL, 0, NULL);
    DWORD err = GetLastError();
    if (!thr) {
        printf_to_console("Could not start logger thread. Error: %I32u\n", err);
        printf_to_console("Logging will not be available.\n");
        return err;
    }
    struct logging_options *options = (struct logging_options *) malloc(sizeof(*options));
    if (!options) {
        /* TODO: Send debug message and ret 0 */
        return ERROR_OUTOFMEMORY;
    }

    /* Don't create a thread if it's always on... */
    if (logmode == 2) {
        if (!debug) {
            return 0;
        }
        COORD pos;
        pos.X = 0;
        pos.Y = toggle_line;
        printf_to_console_pos(pos, "Log:  ON");
        return 0;
    }

    options->toggle = log_toggle;
    options->mode = logmode;
    options->line = toggle_line;
    options->debug = debug;

    thr = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&keylistener_thread, options, 0, NULL);
    err = GetLastError();
    if (!thr) {
        printf_to_console("Could not start keylistener thread. Error: %I32u\n", err);
        printf_to_console("Features relying on key listening will not be available.\n");
        return err;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */