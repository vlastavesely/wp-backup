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

static struct option long_options[] = {
	{"username",    required_argument, 0, 'u'},
	{"wpurl",       required_argument, 0, 'w'},
	{"output-file", required_argument, 0, 'o'},
	{"version",     no_argument,       0, 'v'},
	{"help",        no_argument,       0, 'h'},
	{0, 0, 0, 0}
};

static void getopt_parse(struct options *options, int argc, char **argv)
{
	int option_index = 0;
	int c = 0;

	while (c != -1) {
		c = getopt_long(argc, argv, "u:w:o:vh", long_options,
				&option_index);

		switch (c) {
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
				/* Unrecognized parameter. Error message has
				 * been already printed by getopt. */
				exit(1);
			default:
				break;
		}
	}
}

static void report_invalid_option(const char *message)
{
	fprintf(stderr, "fatal: %s\n", message);
	exit(1);
}

static void validate_options(struct options *options)
{
	if (options->help ||options->version) {
		return;
	}
	if (options->username == NULL) {
		report_invalid_option("username cannot be empty.");
	}
	if (options->wpurl == NULL) {
		report_invalid_option("WordPress URL cannot be empty.");
	}
	if (strncmp(options->wpurl, "https://", 8) &&
	    strncmp(options->wpurl, "http://", 7)) {
		report_invalid_option("WordPress URL does not have 'http://'"
				      " or 'https://' prefix.");
	}
}

struct options *options_parse(int argc, char **argv)
{
	struct options *options = malloc(sizeof(struct options));

	options->username = NULL;
	options->wpurl = NULL;
	options->output_file = "wordpress.xml";
	options->version = false;
	options->help = false;

	getopt_parse(options, argc, argv);
	validate_options(options);

	return options;
}

void options_free(struct options *options)
{
	free(options);
}
