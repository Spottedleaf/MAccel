
#include <math.h>
#include <Windows.h>

#include "console_handler.h"
#include "interception\interception.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static double conv_x = 1.0;
static double conv_y = 1.0;

static char *next_argument(const char *str) {
    if (str == NULL) {
        return NULL;
    }
    char *ret = strchr(str, ' ');
    if (ret) {
        *ret = '\0';
        if (*(ret + 1) == '\0') {
            return NULL;
        }
        return ret + 1;
    }
    return NULL;
}

void command_input(void *param) {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
    const COORD cursor_pos = *(COORD *)param;
    free(param);

    const COORD info_pos = (COORD) { .X = 0, .Y = (cursor_pos.Y + 1) };

    uint32_t id_counter = 0;

    InterceptionContext context = interception_create_context();
    InterceptionDevice mouse = INTERCEPTION_MOUSE(0);
    InterceptionDevice keyboard = INTERCEPTION_KEYBOARD(0);

    char input_buffer[256];

    char clear[513];
    memset(clear, ' ', sizeof(clear) - 1);
    clear[sizeof(clear) - 1] = '\0';

    const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    
    LARGE_INTEGER freq;
    LARGE_INTEGER prevTime, currTime;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prevTime);

    for (;; SetConsoleCursorPosition(console, cursor_pos)) {
        const size_t read = read_stdin(input_buffer, sizeof(input_buffer));

        clear[read] = '\0';
        printf_to_console(cursor_pos, clear);
        clear[read] = ' ';

        QueryPerformanceCounter(&prevTime);

        printf_to_console_next(input_buffer);

        const char *arguments = strchr(input_buffer, ' ');

        if (strstr(input_buffer, "minput") == input_buffer) {

        } else if (strstr(input_buffer, "kinput") == input_buffer) {

        } else if (strstr(input_buffer, "delta") == input_buffer) {
            if (!arguments) {
                printf_to_console(info_pos, clear);
                printf_to_console(info_pos, "delta <x2> <y2> <x1> <y1> <delay> [ntimes] [period] [absolute]");
                continue;
            }
            ++arguments;
            const char *arg_x2 = arguments;
            const char *arg_y2 = next_argument(arg_x2);
            const char *arg_x1 = next_argument(arg_y2);
            const char *arg_y1 = next_argument(arg_x1);
            const char *arg_delay = next_argument(arg_y1);

            const char *arg_ntimes = next_argument(arg_delay);
            const char *arg_period = next_argument(arg_ntimes);
            const char *arg_absolute = next_argument(arg_period);

            if (!arg_delay) {
                continue;
            }

            double x2, y2, x1, y1;
            int delay;

            x2 = atof(arg_x2);
            y2 = atof(arg_y2);
            x1 = atof(arg_x1);
            y1 = atof(arg_y1);
            delay = atoi(arg_delay);

            int ntimes = 1;
            int period = -1;

            if (arg_period) {
                ntimes = atoi(arg_ntimes);
                period = atoi(arg_period);
            }

            unsigned short absolute = 0;

            if (arg_absolute) {
                absolute = (unsigned short) (atoi(arg_absolute) != 0);
            }

            const int delta_x = (int) round((x2 - x1) * conv_x);
            const int delta_y = (int) round((y2 - y1) * conv_y);
            
            InterceptionMouseStroke stroke;
            stroke.flags = absolute;
            stroke.information = 0;
            stroke.rolling = 0;
            stroke.state = 0;
            stroke.x = delta_x;
            stroke.y = delta_y;
            
            QueryPerformanceCounter(&prevTime);

            if (delay > 0) {
                Sleep(delay);
            }

            for (; ntimes > 0; --ntimes) {
                if (period > 0) {
                    Sleep(period);
                }
                interception_send(context, mouse, (const InterceptionStroke *) &stroke, 1);
                
                QueryPerformanceCounter(&currTime);
                
                printf_to_console(info_pos, clear);
                printf_to_console(info_pos, "Delta (%+4i, %+4i), time: %.2fms, absolute: %s", delta_x, delta_y,
                    ((currTime.QuadPart - prevTime.QuadPart) * 1000) / (double)(freq.QuadPart), 
                    (absolute == 1) ? "Yes" : "No");
                
                prevTime = currTime;
            }
        } else if (strstr(input_buffer, "multiplier_x") == input_buffer) {
            if (!arguments) {
                printf_to_console(info_pos, clear);
                printf_to_console(info_pos, "X Multiplier: %.6e", conv_x);
                continue;
            }
            const double temp = atof(arguments + 1);
            if (temp == 0.0) {
                printf_to_console(info_pos, clear);
                printf_to_console(info_pos, "X Multiplier: %.6e", conv_x);
                continue;
            }
            printf_to_console(info_pos, clear);
            printf_to_console(info_pos, "Old X Multiplier: %.6e, New: %.6e", conv_x, temp);
            conv_x = temp;
        } else if (strstr(input_buffer, "multiplier_y") == input_buffer) {
            if (!arguments) {
                printf_to_console(info_pos, clear);
                printf_to_console(info_pos, "Y Multiplier: %.6e", conv_y);
                continue;
            }
            const double temp = atof(arguments + 1);
            if (temp == 0.0) {
                printf_to_console(info_pos, clear);
                printf_to_console(info_pos, "Y Multiplier: %.6e", conv_y);
                continue;
            }
            printf_to_console(info_pos, clear);
            printf_to_console(info_pos, "Old Y Multiplier: %.6e, New: %.6e", conv_y, temp);
            conv_y = temp;
        }
    }
}

DWORD command_input_init(COORD cursor_pos) {
    COORD *cursor_pos_buffer = malloc(sizeof(cursor_pos));
    *cursor_pos_buffer = cursor_pos;
    const HANDLE debug_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)command_input, cursor_pos_buffer, 0, NULL);
    DWORD err = GetLastError();
    if (debug_handle == NULL) {
        printf_to_console_next("Could not start command handler thread. Error: %I32u", GetLastError());
        printf_to_console_next("Commands will not be available.");
        return err;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */