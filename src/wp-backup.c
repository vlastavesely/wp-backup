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
	fprintf(stdout, "%s v%s\n", APPNAME, VERSION);
}

static void print_usage()
{
	fprintf(stdout, "%s\n", "The usage.."); // TODO
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

	connection = wordpress_connection_initialize(options->wpurl);
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

	// TODO Check that the dump is valid

	logged = wordpress_connection_logout(connection);
	if (!logged) {
		fprintf(stderr, "\e[33mwarning: logout failed.\e[0m\n");
	}

out:
	wordpress_connection_free(connection);
	options_free(options);

	return retval;
}