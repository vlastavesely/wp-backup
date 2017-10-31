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

#ifndef __OPTIONS_H
#define __OPTIONS_H

#include "error.h"

struct options {
	const char *username;
	const char *wpurl;
	const char *output_file;
	int version;
	int help;
	int ignore_ssl_errors;
};

#define OPTIONS_ERROR_UNRECOGNIZED_ARGUMENT 1
#define OPTIONS_ERROR_MISSING_ARGUMENT 2
#define OPTIONS_ERROR_BAD_ARGUMENT_VALUE 3

int options_parse(struct options *options, int argc, const char **argv,
		  struct error **error);

#endif /* __OPTIONS_H */
