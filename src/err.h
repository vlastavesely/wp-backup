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

#ifndef __ERR_H
#define __ERR_H

#include <stdint.h>

#define MAX_ERRNO 4095

#define IS_ERR_VALUE(x) ((uintptr_t)(void *)(x) >= (uintptr_t) - MAX_ERRNO)

static inline void *ERR_PTR(long error)
{
	return (void *) error;
}

static inline long PTR_ERR(const void *ptr)
{
	return (long) ptr;
}

static inline int IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((uintptr_t) ptr);
}

static inline int IS_ERR_OR_NULL(const void *ptr)
{
	return !ptr || IS_ERR_VALUE((uintptr_t) ptr);
}

void set_warning_routine(void (*routine)(const char *warn, va_list params));
void set_fatal_routine(void (*routine)(const char *err, va_list params));

void warning(const char *err, ...);
void fatal(const char *err, ...);

#endif /* __ERR_H */
