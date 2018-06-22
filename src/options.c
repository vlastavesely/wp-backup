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
#include "options.h"
#include "err.h"

static const char *short_opts = "hvqu:w:o:";

static const struct option long_opts[] = {
	{"help",        no_argument,       0, 'h'},
	{"version",     no_argument,       0, 'v'},
	{"quiet",       no_argument,       0, 'q'},
	{"username",    required_argument, 0, 'u'},
	{"wpurl",       required_argument, 0, 'w'},
	{"output-file", required_argument, 0, 'o'},
	{"ignore-ssl-errors", no_argument, 0, 0},
	{0, 0, 0, 0}
};

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
		case 'q':
			options->quiet = true;
			break;
		case '?':
			error("unrecognized option '-%s'.", optopt ?
				(char *) &(optopt) : argv[optind - 1] + 1);
			return -1;
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
		error("username cannot be empty.");
		return -1;
	}

	if (!options->wpurl) {
		error("WordPress URL cannot be empty.");
		return -1;
	}

	if (strncmp(options->wpurl, "https://", 8) &&
	    strncmp(options->wpurl, "http://", 7)) {
		error("WordPress URL does not have 'http[s]://' prefix.");
		return -1;
	}

	return 0;
}

int options_parse(struct options *options, int argc, const char **argv)
{
	int retval;

	options->username = NULL;
	options->wpurl = NULL;
	options->output_file = "wordpress.xml";
	options->quiet = false;
	options->version = false;
	options->help = false;
	options->ignore_ssl_errors = false;

	if (argc == 1) {
		options->help = true;
		return 0;
	}

	retval = getopt_parse(options, argc, argv);
	if (retval != 0)
		return retval;

	retval = validate_options(options);
	if (retval != 0)
		return retval;

	return 0;
}
