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
#include <string.h>
#include <stdbool.h>
#include <getopt.h>

#include "options.h"
#include "error-handler.h"

static const char *short_opts = "hvu:w:o:";

static const struct option long_opts[] = {
	{"help",        no_argument,       0, 'h'},
	{"version",     no_argument,       0, 'v'},
	{"username",    required_argument, 0, 'u'},
	{"wpurl",       required_argument, 0, 'w'},
	{"output-file", required_argument, 0, 'o'},
	{"ignore-ssl-errors", no_argument, 0, 0},
	{0, 0, 0, 0}
};

static char errstr[256];

static int getopt_parse(struct options *options, int argc, const char **argv)
{
	int opt_index = 0;
	int c = 0;
	const char *name;

	opterr = 0; /* disable auto error message */
	while (c != -1) {
		c = getopt_long(argc, (char *const *) argv,
				short_opts, long_opts, &opt_index);

		switch (c) {
		case 0:
			name = long_opts[opt_index].name;
			if (!strcmp(name, "ignore-ssl-errors"))
				options->ignore_ssl_errors = true;
			break;
		case 'u':
			options->username = optarg;
			break;
		case 'w':
			options->wpurl = optarg;
			break;
		case 'o':
			options->output_file = optarg;
			break;
		case 'v':
			options->version = true;
			break;
		case 'h':
			options->help = true;
			break;
		case '?':
			snprintf(errstr, sizeof(errstr),
				"unrecognized option '-%s'.", optopt ?
				(char *) &(optopt) : argv[optind - 1] + 1);
			return -EUNRECOG;
		default:
			break;
		}
	}

	return 0;
}

static int validate_options(struct options *options)
{
	if (options->help || options->version)
		return 0;

	if (!options->username) {
		strcpy(errstr, "username cannot be empty.");
		return -EMISSARG;
	}
	if (!options->wpurl) {
		strcpy(errstr, "WordPress URL cannot be empty.");
		return -EMISSARG;
	}
	if (strncmp(options->wpurl, "https://", 8) &&
	    strncmp(options->wpurl, "http://", 7)) {
		strcpy(errstr, "WordPress URL does not have 'http://' "
			       "or 'https://' prefix.");
		return -EBADARGVAL;
	}

	return 0;
}

int options_parse(struct options *options, int argc, const char **argv)
{
	int retval = 0;

	options->username = NULL;
	options->wpurl = NULL;
	options->output_file = "wordpress.xml";
	options->version = false;
	options->help = false;
	options->ignore_ssl_errors = false;

	memset(errstr, 0, sizeof(errstr));

	/* No arguments provided */
	if (argc == 1) {
		options->help = true;
		goto out;
	}

	if ((retval = getopt_parse(options, argc, argv)) != 0)
		goto out;
	if ((retval = validate_options(options)) != 0)
		goto out;

out:
	return retval;
}

char *options_errstr()
{
	return strdup(errstr);
}
