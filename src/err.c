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

#include "compat.h"

static void vreportf(const char *prefix, const char *err, va_list params)
{
	char msg[4096];

	vsnprintf(msg, sizeof(msg), err, params);
	fprintf(stderr, "[%s] %s%s\n", "wp-backup", prefix, msg);
}

static void warning_builtin(const char *warn, va_list params)
{
	vreportf("warning: ", warn, params);
}

static void error_builtin(const char *err, va_list params)
{
	vreportf("error: ", err, params);
}

static void die_builtin(const char *err, va_list params)
{
	fprintf(stderr, err, params);
	exit(128);
}

static void (*warning_routine)(const char *warn, va_list params) = warning_builtin;
static void (*error_routine)(const char *err, va_list params) = error_builtin;
static void (*die_routine)(const char *err, va_list params) = die_builtin;

void set_warning_routine(void (*routine)(const char *warn, va_list params))
{
	warning_routine = routine;
}

void set_error_routine(void (*routine)(const char *err, va_list params))
{
	error_routine = routine;
}

void set_die_routine(void (*routine)(const char *err, va_list params))
{
	die_routine = routine;
}

void warning(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	warning_routine(err, params);
	va_end(params);
}

#undef error
void error(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	error_routine(err, params);
	va_end(params);
}

void die(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	die_routine(err, params);
	va_end(params);
}
