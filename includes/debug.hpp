#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <cstdio>
#include <stdarg.h>

#ifdef DEBUG_MODE
#define DEBUG_MSG(msg, ...) (debug_printf(msg, ##__VA_ARGS__))
#else
#define DEBUG_MSG(msg, ...)
#endif

inline void debug_printf(const char *const msg, ...) {
	static int count = 1;
	char buff[512];
	va_list args;

	va_start(args, msg);
	vsprintf(buff, msg, args);
	va_end(args);
	printf("%d: %s\n", count, buff);
	count++;
}

#endif