#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H INPUT_HANDLER_H\

#include <Windows.h>

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

void input_handler_write(const struct acceleration_value *element);
DWORD init_input_handler(const uint32_t api);

#ifdef __cplusplus
}
#endif

#endif /* INPUT_HANDLER_H */
