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
#include <string.h>
#include <errno.h>

#include "error.h"
#include "error-handler.h"

struct error *error_new(int code, const char *err, ...)
{
	struct error *error;
	va_list params;
	char msg[4096];

	if ((error = malloc(sizeof(*error))) == NULL)
		return ERR_PTR(-ENOMEM);

	va_start(params, err);
	vsnprintf(msg, sizeof(msg), err, params);
	va_end(params);

	error->code = code;
	error->message = strdup(msg);
	return error;
}

void error_free(struct error *error)
{
	if (error != NULL) {
		free(error->message);
		free(error);
		error = NULL;
	}
}
