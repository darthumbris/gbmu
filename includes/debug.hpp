#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <cstdint>
#include <cstdio>
#include <stdarg.h>

#ifdef DEBUG_MODE
#define DEBUG_MSG(msg, ...) (debug_printf(msg, ##__VA_ARGS__))
#else
#define DEBUG_MSG(msg, ...)
#endif

#define ERROR_MSG(msg, ...) (debug_fprintf(msg, ##__VA_ARGS__))

inline void debug_printf(const char *const msg, ...) {
	static int64_t count = 0;

	if (count > 0) {
		char buff[512];
		va_list args;

		va_start(args, msg);
		vsprintf(buff, msg, args);
		va_end(args);
		printf("%s", buff);
	}
	count++;
}

inline void debug_fprintf(const char *const msg, ...) {
	char buff[512];
	va_list args;

	va_start(args, msg);
	vsprintf(buff, msg, args);
	va_end(args);
	fprintf(stderr, "%s", buff);
}

#endif