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

static const char usage_string[] =
	PACKAGE_NAME " [options]\n"
	"\n"
	"options:\n"
	"  -h, --help               shows this help and exits\n"
	"  -v, --version            shows version number and exits\n"
	"  -u, --username           username for login into WordPress\n"
	"  -w, --wpurl              URL of the WordPress installation (without \"wp-admin\" or \"wp-login\")\n"
	"  -o, --output-file        destination file name for downloaded XML (default: \"wordpress.xml\")\n"
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

	/* May fail when invalid or no arguments provided. */
	retval = options_parse(&options, argc, argv);
	if (retval != 0) {
		error(options_errstr());
		goto out;
	}

	if (options.help)
		print_usage();

	if (options.version)
		print_version();

	/* FIXME That's DIRTY. What about remove all the support for
	 * skipping invalid SSL?
	 */
	if (options.ignore_ssl_errors)
		warning("skiping validation of SSL certificate\n"
			"is considered to be a risk. You should do your\n"
			"best to fix your server's SSL settings and not\n"
			"use this option at all!\n");

	wordpress = wordpress_create(options.wpurl);

	password = password_resolver_resolve_password();
	if (IS_ERR(password))
		die("failed to resolve a password.");

	retval = wordpress_login(wordpress, options.username, password);
	memset(password, 0, strlen(password));
	free(password);

	if (retval != 0)
		die("login failed.\n");

	if (wordpress_export(wordpress, options.output_file) != 0)
		die("export failed.\n");

	if (wordpress_logout(wordpress) != 0)
		die("logout failed.\n");

	wordpress_free(wordpress);

out:
	return retval;
}
