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
#include <getopt.h>

#include <wp-backup/options.h>

static const char *short_opts = "u:w:o:vh";

static const struct option long_opts[] = {
	{"username",    required_argument, 0, 'u'},
	{"wpurl",       required_argument, 0, 'w'},
	{"output-file", required_argument, 0, 'o'},
	{"version",     no_argument,       0, 'v'},
	{"help",        no_argument,       0, 'h'},
	{"ignore-ssl-errors", no_argument, 0, 0},
	{0, 0, 0, 0}
};

static void getopt_parse(struct options *options, int argc, const char **argv)
{
	int opt_index = 0;
	int c = 0;
	const char *name;

	while (c != -1) {
		c = getopt_long(argc, (char *const *) argv,
				short_opts, long_opts, &opt_index);

		switch (c) {
		case 0:
			name = long_opts[opt_index].name;
			if (!strcmp(name, "ignore-ssl-errors"))
				options->ignore_ssl_errors = 1;
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
			options->version = 1;
			break;
		case 'h':
			options->help = 1;
			break;
		case '?':
			/* Unrecognized parameter. Error message has
			 * been already printed by getopt.
			 */
			exit(1);
		default:
			break;
		}
	}
}

static void die_on_invalid_option(const char *message)
{
	fprintf(stderr, "fatal: %s\n", message);
	exit(1);
}

static void validate_options(struct options *options)
{
	if (options->help || options->version)
		return;

	if (!options->username)
		die_on_invalid_option("username cannot be empty.");

	if (!options->wpurl)
		die_on_invalid_option("WordPress URL cannot be empty.");

	if (strncmp(options->wpurl, "https://", 8) &&
	    strncmp(options->wpurl, "http://", 7))
		die_on_invalid_option("WordPress URL does not have 'http://'"
				      " or 'https://' prefix.");

	/* FIXME That's DIRTY. What about remove all the support for
	 * skipping invalid SSL?
	 */
	if (options->ignore_ssl_errors)
		fprintf(stderr,
			"\x1b[33m\n"
			"WARNING: skiping validation of SSL certificate\n"
			"is considered to be a risk. You should do your\n"
			"best to fix your server's SSL settings and not\n"
			"use this option at all!\n"
			"\x1b[0m\n");
}

int options_parse(struct options *options, int argc, const char **argv)
{
	options->username = NULL;
	options->wpurl = NULL;
	options->output_file = "wordpress.xml";
	options->version = 0;
	options->help = 0;
	options->ignore_ssl_errors = 0;

	if (argc == 1) {
		options->help = 1;
	} else {
		getopt_parse(options, argc, argv);
		validate_options(options);
	}

	return 0;
}
