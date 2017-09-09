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
#include <string.h>
#include <stdlib.h>

#include <wp-backup.h>

static void print_version()
{
	fprintf(stdout, "%s v%s\n", PACKAGE_NAME, PACKAGE_VERSION);
}

static void print_usage()
{
	fprintf(stdout,
		"Usage: " PACKAGE_NAME " [options]\n"
		"\n"
		"Options:\n"
		"  -h, --help               shows this help and exits\n"
		"  -v, --version            shows version number and exits\n"
		"  -u, --username           username for login into WordPress\n"
		"  -w, --wpurl              URL of the WordPress installation "
					"(without \"wp-admin\" or \"wp-login\")\n"
		"  -o, --output-file        destination file name for downloaded XML "
					"(default: \"wordpress.xml\")\n"
		"      --ignore-ssl-errors  skips SSL certificate validation "
					"(this is not a good practice!)\n"
		"\n"
		"  " PACKAGE_NAME " reads password from standard input or "
		"environmental variable WPPASS.\n"
		"  WordPress must be v2.5.0 or higher!\n"
		"\n");
}


int main(int argc, char **argv)
{
	struct options *options;
	struct wordpress_connection *connection;
	char *password;
	int retval = 0;
	bool logged;

	DEBUG("Parsing command line options...\n");
	options = options_parse(argc, argv);

	if (argc == 1 || options->help) {
		print_usage();
		options_free(options);
		return 0;
	}
	if (options->version) {
		print_version();
		options_free(options);
		return 0;
	}

	connection = wordpress_connection_initialize(options);
	password = password_resolver_resolve_password();

	logged = wordpress_connection_login(connection, options->username,
					    password);
	memset(password, 0, strlen(password));
	free(password);

	if (!logged) {
		fprintf(stderr, "fatal: login failed.\n");
		retval = 1;
		goto out;
	}


	/* Download the export XML */
	char *export_url;

	export_url = malloc(strlen(options->wpurl) + 48);
	strcpy(export_url, options->wpurl);
	if (export_url[strlen(export_url) - 1] != '/') {
		strcat(export_url, "/");
	}
	strcat(export_url, "wp-admin/export.php?content=all&download=true");
	DEBUG("Built export URL ('%s').\n", export_url);


	wordpress_connection_download_to_file(connection, export_url,
		options->output_file);
	free(export_url);

	/* TODO Check that the dump is valid */

	logged = wordpress_connection_logout(connection);
	if (!logged) {
		fprintf(stderr, "\x1b[33mwarning: logout failed.\x1b[0m\n");
	}

out:
	wordpress_connection_free(connection);
	options_free(options);

	return retval;
}
