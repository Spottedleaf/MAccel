#ifndef COMMAND_INPUT_H
#define COMMAND_INPUT_H COMMAND_INPUT_H

#include <stdint.h>

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

DWORD command_input_init(COORD cursor_pos);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMMAND_INPUT_H */