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
#include "wordpress.h"
#include "password-resolver.h"
#include "err.h"

#define PACKAGE_NAME "wp-backup"
#define PACKAGE_VERSION "0.1"

static const char *usage_string =
	PACKAGE_NAME " [options]\n"
	"\n"
	"options:\n"
	"  -h, --help               shows this help and exits\n"
	"  -v, --version            shows version number and exits\n"
	"  -u, --username           username for login into WordPress\n"
	"  -w, --wpurl              URL of the WordPress installation (without \"wp-admin\" or \"wp-login\")\n"
	"  -o, --output-file        destination file name for downloaded XML (default: \"wordpress.xml\")\n"
	"  -q, --quiet              disables output into terminal\n"
	"      --ignore-ssl-errors  skips SSL certificate validation (this is not a good practice!)\n"
	"\n"
	"  " PACKAGE_NAME " reads password from standard input or "
	"environmental variable WPPASS.\n"
	"  WordPress must be v2.5.0 or higher!\n";

static void print_usage(void)
{
	die("%s\n", usage_string);
}

static void print_version(void)
{
	die("%s v%s\n", PACKAGE_NAME, PACKAGE_VERSION);
}

int main(int argc, const char **argv)
{
	struct options options;
	struct wordpress *wordpress;
	char *password;
	int retval;

	retval = options_parse(&options, argc, argv);
	if (retval != 0)
		goto out;

	if (options.help)
		print_usage();

	if (options.version)
		print_version();

	wordpress = wordpress_create(options.wpurl);
	if (IS_ERR(wordpress)) {
		error("failed to prepare a WordPress connection.");
		retval = PTR_ERR(wordpress);
		goto out;
	}

	password = password_resolver_resolve_password();
	if (IS_ERR(password)) {
		error("failed to resolve a password.");
		retval = PTR_ERR(password);
		goto drop_wordpress;
	}

	retval = wordpress_login(wordpress, options.username, password);
	memset(password, 0, strlen(password));
	free(password);
	if (retval != 0) {
		error("login failed.");
		goto drop_wordpress;
	}

	if (wordpress_export(wordpress, options.output_file, options.quiet) != 0) {
		error("export failed.");
		goto drop_wordpress;
	}

	if (wordpress_logout(wordpress) != 0) {
		error("logout failed.");
		goto drop_wordpress;
	}

drop_wordpress:
	wordpress_drop(wordpress);
out:
	return retval;
}
