/**
 * Copyright (c) 2017 Vlasta Vesely <vlastavesely@protonmail.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static void vreportf(const char *prefix, const char *err, va_list params)
{
	char msg[4096];

	vsnprintf(msg, sizeof(msg), err, params);
	fprintf(stderr, "%s%s\n", prefix, msg);
}

static void warning_builtin(const char *err, va_list params)
{
	vreportf("warning: ", err, params);
}

static void fatal_builtin(const char *err, va_list params)
{
	vreportf("fatal: ", err, params);
	exit(128);
}

static void (*warning_routine)(const char *err, va_list params) = warning_builtin;
static void (*fatal_routine)(const char *err, va_list params) = fatal_builtin;

void warning(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	warning_routine(err, params);
	va_end(params);
}

void fatal(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	fatal_routine(err, params);
	va_end(params);
}
