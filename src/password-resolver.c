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
#include <unistd.h>
#include <termios.h>

#include <wp-backup/password-resolver.h>

static void print_password_prompt()
{
	fprintf(stderr, "Enter your WordPress password: ");
}

static void trim_trailing_newlines(char *str)
{
	char *last = str + strlen(str) - 1;

	while (last > str && ((char) *last == '\n' || (char) *last == '\r'))
		*(last--) = '\0';
}

char *password_resolver_resolve_password()
{
	struct termios oflags, nflags;
	char buffer[256];
	char *password;

	password = getenv("WPPASS");
	if (password)
		return strdup(password);

	if (isatty(0)) {
		tcgetattr(0, &oflags);
		nflags = oflags;
		nflags.c_lflag &= ~ECHO;
		nflags.c_lflag |= ECHONL;

		if (tcsetattr(0, TCSANOW, &nflags) != 0) {
			perror("tcsetattr");
			return NULL;
		}

		print_password_prompt();

		fgets(buffer, sizeof buffer, stdin);
		if (tcsetattr(0, TCSANOW, &oflags) != 0) {
			perror("tcsetattr");
			return NULL;
		}
	} else {
		fgets(buffer, sizeof buffer, stdin);
	}

	trim_trailing_newlines(buffer);
	password = strdup(buffer);
	memset(buffer, 0, sizeof buffer);

	return password;
}
