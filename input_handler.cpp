#include <stdint.h>
#include <math.h>

#include <atomic>

#include <Windows.h>

#include "utils.h"

/* Use C++ for atomics */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static std::atomic<struct acceleration_value> last_edit;

static HANDLE thread;

static void input_handler_read(struct acceleration_value *dst) {
    *dst = std::atomic_load_explicit(&last_edit, std::memory_order_seq_cst);
}

void input_handler(void *param) {
    const HANDLE curr_thr = GetCurrentThread();
    SetThreadPriority(curr_thr, THREAD_PRIORITY_BELOW_NORMAL);

    const double dots_per_cm = ((double)(*((uint32_t *)param))) / 2.54;

    free(param);

    struct acceleration_value element_buffer;

    uint32_t last_id = UINT32_MAX;

    double total_distance = 0.0;

    for (;;) {
        SuspendThread(curr_thr);
        
        input_handler_read(&element_buffer);

        if (element_buffer.id == last_id || !isfinite(element_buffer.time)) {
            continue;
        }

        total_distance += hypot(element_buffer.unaccelx, element_buffer.unaccely);

        uint32_t missed = element_buffer.id - last_id - 1;

        last_id = element_buffer.id;
        
        COORD line;
        memset(&line, 0, sizeof(line));

        printf_to_console(line, "Unaccelerated: (%+5i,%+5i)", (int)element_buffer.unaccelx, 
            (int)element_buffer.unaccely);
        ++line.Y;
        printf_to_console(line, "Accelerated: (%+5i,%+5i), time: %6.4ems ", (int)element_buffer.accelx,
            (int)element_buffer.accely, element_buffer.time);
        ++line.Y;
        printf_to_console(line, "Missed (to display) %4i updates", missed);
        ++line.Y;
        printf_to_console(line, "Last Id: %8I32u", element_buffer.id);
        ++line.Y;
        printf_to_console(line, "Distance travelled: %7.3fcm", total_distance / dots_per_cm);
    }
}

void input_handler_write(const struct acceleration_value *element) {
    std::atomic_store_explicit(&last_edit, *element, std::memory_order_seq_cst);
    ResumeThread(thread);
}

DWORD init_input_handler(const uint32_t dpi) {
    struct acceleration_value init;
    memset(&init, 0, sizeof(init));
    init.time = NAN;
    std::atomic_store_explicit(&last_edit, init, std::memory_order_relaxed);

    uint32_t *dpi_ptr = (uint32_t *) malloc(sizeof(dpi));
    *dpi_ptr = dpi;

    HANDLE debug_handle = thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&input_handler, dpi_ptr, 0, NULL);
    DWORD err = GetLastError();
    if (debug_handle == NULL) {
        printf_to_console_next("Could not start debug output thread. Error: %I32u", GetLastError());
        printf_to_console_next("Debug output will not be available.");
        return err;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#undef INPUT_BUFFER_SIZE