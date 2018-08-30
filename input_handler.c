#include <stdint.h>
#include <math.h>

#include <Windows.h>

#include "utils.h"
#include "rustedleaf\rl_cdrain_queue.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static struct rl_cdrain_queue queue;

void input_handler(void *param) {
    const HANDLE curr_thr = GetCurrentThread();
    SetThreadPriority(curr_thr, THREAD_PRIORITY_BELOW_NORMAL);
    protect_thread();

    const double dots_per_cm = ((double)(*((uint32_t *)param))) / 2.54;

    free(param);

    static struct acceleration_value element_buffer[128];

    uint32_t last_id = UINT32_MAX; // -1

    double total_distance = 0.0;

    for (;;) {
        const size_t read = rl_cdrain_queue_drain(&queue, element_buffer, 
            sizeof(element_buffer) / sizeof(*element_buffer), sizeof(*element_buffer), 0);

        if (!read) {
            Sleep(4);
            continue;
        }

        for (size_t i = 0; i < read; ++i) {
            struct acceleration_value *curr = element_buffer + i;
            total_distance += hypot(curr->unaccelx, curr->unaccely);
        }

        struct acceleration_value *show = element_buffer + read - 1;

        uint32_t missed = show->id - last_id - 1;
        
        COORD line;
        memset(&line, 0, sizeof(line));

        printf_to_console_pos(line, "Unaccelerated: (%+5i,%+5i)", (int)show->unaccelx,
            (int)show->unaccely);
        ++line.Y;
        printf_to_console_pos(line, "Accelerated: (%+5i,%+5i), time: %6.4ems ", (int)show->accelx,
            (int)show->accely, show->time);
        ++line.Y;
        printf_to_console_pos(line, "Missed (to display) %4i updates", missed);
        ++line.Y;
        printf_to_console_pos(line, "Last Id: %8I32u", show->id);
        ++line.Y;
        printf_to_console_pos(line, "Distance travelled: %7.3fcm", total_distance / dots_per_cm);
        if (read != (sizeof(element_buffer) / sizeof(*element_buffer))) {
            Sleep(4); /* Only sleep if the read buffer was not full */
        }
    }
}

void input_handler_write(const struct acceleration_value *element) {
    rl_cdrain_queue_add(&queue, element, 1, sizeof(*element), 0);
}

DWORD init_input_handler(const uint32_t dpi) {
    uint32_t *dpi_ptr = (uint32_t *) malloc(sizeof(dpi));
    if (!dpi_ptr) {
        return ERROR_OUTOFMEMORY;
    }
    *dpi_ptr = dpi;
    
    rl_cdrain_queue_init(&queue, 2048, sizeof(struct acceleration_value)); 

    HANDLE debug_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&input_handler, dpi_ptr, 0, NULL);
    DWORD err = GetLastError();
    if (debug_handle == NULL) {
        printf_to_console("Could not start debug output thread. Error: %I32u\n", err);
        printf_to_console("Debug output will not be available.\n");
        return err;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#undef INPUT_BUFFER_SIZE