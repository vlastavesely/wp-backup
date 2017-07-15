#include <stdio.h>
#include <stdarg.h>

#include <wpbackup/debug.h>

void debug_info(char *format,...)
{
	va_list args;
	va_start(args, format);

	fprintf(stderr, "[ \e[34;1mINFO\e[0m ] ");
	vfprintf(stderr, format, args);

	va_end(args);
}
