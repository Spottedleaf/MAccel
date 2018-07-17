#ifndef CONSOLE_HANDLER_H
#define CONSOLE_HANDLER_H CONSOLE_HANDLER_H

#include <stdarg.h>
#include <stdint.h>

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Non-blocking printf functions */

/*
 * printf defined in stdio.h will block if the user highlights (marks) any text in the terminal 
 * This would block the main thread and would effectively block any user input, so these functions prevent that by 
 * manually setting characters in the console 
 */

/* All functions are thread-safe */


int vprintf_to_console(const COORD pos, const char *format, va_list args);

static inline int printf_to_console(const COORD pos, const char *format, ...) {
	va_list args;
	va_start(args, format);

	return vprintf_to_console(pos, format, args);
}

int vprintf_to_console_next(const char *format, va_list args);
void console_set_nextline_number(const uint16_t n);

static inline int printf_to_console_next(const char *format, ...) {
	va_list args;
	va_start(args, format);

	return vprintf_to_console_next(format, args);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CONSOLE_HANDLER_H */