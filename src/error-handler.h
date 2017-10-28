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

#ifndef __ERROR_HANDLER_H
#define __ERROR_HANDLER_H

void set_warning_routine(void (*routine)(const char *warn, va_list params));
void set_fatal_routine(void (*routine)(const char *err, va_list params));

void warning(const char *err, ...);
void fatal(const char *err, ...);

#endif /* __ERROR_HANDLER_H */
