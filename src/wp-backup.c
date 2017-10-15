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

static const char usage_string[] =
	"Usage: %s [options]\n"
	"\n"
	"Options:\n"
	"  -h, --help               shows this help and exits\n"
	"  -v, --version            shows version number and exits\n"
	"  -u, --username           username for login into WordPress\n"
	"  -w, --wpurl              URL of the WordPress installation (without \"wp-admin\" or \"wp-login\")\n"
	"  -o, --output-file        destination file name for downloaded XML (default: \"wordpress.xml\")\n"
	"      --ignore-ssl-errors  skips SSL certificate validation (this is not a good practice!)\n"
	"\n"
	"  %s reads password from standard input or "
	"environmental variable WPPASS.\n"
	"  WordPress must be v2.5.0 or higher!\n\n";

static void print_version(void)
{
	printf("%s v%s\n", PACKAGE_NAME, PACKAGE_VERSION);
}

static void print_usage(void)
{
	printf(usage_string, PACKAGE_NAME, PACKAGE_NAME);
}

int main(int argc, const char **argv)
{
	struct options options;
	struct wordpress *wordpress;
	char *password;
	int ret;

	options_parse(&options, argc, argv);

	if (options.help) {
		print_usage();
		return 0;
	}
	if (options.version) {
		print_version();
		return 0;
	}

	wordpress = wordpress_create(options.wpurl);

	password = password_resolver_resolve_password();

	ret = wordpress_login(wordpress, options.username, password);
	memset(password, 0, strlen(password));
	free(password);

	if (ret != 0) {
		fprintf(stderr, "fatal: login failed.\n");
		return 1;
	}

	if (wordpress_export(wordpress, options.output_file) != 0) {
		fprintf(stderr, "fatal: export failed.\n");
		return 1;
	}

	wordpress_logout(wordpress);

	return 0;
}
