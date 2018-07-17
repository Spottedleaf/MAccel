
#include <stdint.h>
#include <stdlib.h>

#include <atomic>

#include <Windows.h>

#include "interception/interception.h"

#include "console_handler.h"
#include "logger.h"

#define LOG_BUFFER_SIZE 8192

static const char CSV_HEADER[] = "TYPE,INFO,ID,TIME (ms),ORIGX,ORIGY,ACCELX,ACCELY\n";
static const char MOUSE_MISC_INPUT_FORMAT[] = "MISC INPUT,%s,,%.3e\n";
static const char DEBUG_MESSAGE_TIME[] = "DEBUG MSG,%s,,%.3e\n";
static const char DEBUG_MESSAGE_NOTIME[] = "DEBUG MSG,%s\n";
static const char MOUSE_INPUT_FORMAT[] = "MOUSE MOVE,,%u,%.3e,%+i,%+i,%+i,%+i\n";

static struct log_raw LOG_BUFFER[LOG_BUFFER_SIZE];
static std::atomic<size_t> LOG_INDEX = ATOMIC_VAR_INIT(SIZE_MAX);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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
/* NOTE: Not MT-Safe */
void log_entry(const struct log_raw *log) {
    size_t write_to = (atomic_load_explicit(&LOG_INDEX, std::memory_order_seq_cst) + 1) & (LOG_BUFFER_SIZE - 1);
    LOG_BUFFER[write_to] = *log;
    atomic_store_explicit(&LOG_INDEX, write_to, std::memory_order_seq_cst);
}


static int read_log_entry(struct log_raw *buffer, size_t last_index) {
    const size_t last_wrote = atomic_load_explicit(&LOG_INDEX, std::memory_order_seq_cst);
    if (last_index == last_wrote) {
        return 0;
    }

    const size_t read_index = (last_index + 1) & (LOG_BUFFER_SIZE - 1);
    memcpy(buffer, LOG_BUFFER + read_index, sizeof(*buffer));
    return 1;
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

void debug_logger_thr(void *param) {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

    char filename[64];

    DWORD terr = get_filename("MAccel-log %s.csv", filename, sizeof(filename));

    if (terr) {
        printf_to_console_next("Failed to create timestamp, errno %u", terr);
        return;
    }

    printf_to_console_next("Logging to file %s", filename);

    FILE *logfile;
    errno_t err = fopen_s(&logfile, filename, "w");
    if (err) {
        printf_to_console_next("Failed to open file \"%s\" for writing! (errno %i)", filename, err);
        return;
    }

    /* Init header */

    fprintf(logfile, "_%s", CSV_HEADER); /* Prefix with _ because Excel */

    struct log_raw log_entry;

    size_t last_index = SIZE_MAX;

    for (;;) {
        Sleep(500); /* Poll every 500ms */

        while (read_log_entry(&log_entry, last_index)) {
            last_index = (last_index + 1) & (LOG_BUFFER_SIZE - 1);

            if (log_entry.format != NULL) {
                /* Log message */
                const float time = log_entry.unformatted.time;

                if (time < 0.0f) {
                    fprintf(logfile, DEBUG_MESSAGE_NOTIME, log_entry.format);
                } else {
                    fprintf(logfile, DEBUG_MESSAGE_TIME, log_entry.format, time);
                }

                free((void *) log_entry.format);
                continue;
            }

            if (log_entry.unformatted.time < 0.0f) {
                /* Misc mouse input */
                const char *state = mouse_state(log_entry.unformatted.accelx);
                if (!state) {
                    continue;
                }
                fprintf(logfile, MOUSE_MISC_INPUT_FORMAT, state, -log_entry.unformatted.time);
                continue;
            }

            /* Regular mouse movement input */
            fprintf(logfile, MOUSE_INPUT_FORMAT, log_entry.unformatted.id,
                log_entry.unformatted.time, log_entry.unformatted.unaccelx, log_entry.unformatted.unaccely,
                log_entry.unformatted.accelx, log_entry.unformatted.accely);
        }

        fflush(logfile);
    }
}

DWORD init_logger(void) {
    HANDLE thr = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&debug_logger_thr, NULL, 0, NULL);
    DWORD err = GetLastError();
    if (thr == NULL) {
        printf_to_console_next("Could not start logger thread. Error: %I32u", err);
        printf_to_console_next("Logging will not be available.");
        return err;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */