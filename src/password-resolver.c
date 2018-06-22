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
#include "err.h"
#include "password-resolver.h"


static void trim_trailing_newlines(char *str)
{
	char *last = str + strlen(str) - 1;

	while (last > str && ((char) *last == '\n' || (char) *last == '\r'))
		*(last--) = '\0';
}

char *password_resolver_resolve_password(void)
{
	struct termios oflags, nflags;
	char buffer[256];
	char *password;
	int saved_errno;

	if ((password = getenv("WPPASS"))) {
		password = strdup(password);
		unsetenv("WPPASS");
		return password;
	}

	if (isatty(0)) {
		tcgetattr(0, &oflags);
		nflags = oflags;
		nflags.c_lflag &= ~ECHO;
		nflags.c_lflag |= ECHONL;

		if (tcsetattr(0, TCSANOW, &nflags) != 0)
			return ERR_PTR(-errno);

		fprintf(stderr, "Enter your WordPress password: ");

		if (fgets(buffer, sizeof(buffer), stdin) == NULL)
			return ERR_PTR(-1);

		if (tcsetattr(0, TCSANOW, &oflags) != 0) {
			saved_errno = errno;
			memset(buffer, 0, sizeof(buffer));
			return ERR_PTR(-saved_errno);
		}

	} else {
		if (fgets(buffer, sizeof(buffer), stdin) == NULL)
			return ERR_PTR(-1);
	}

	trim_trailing_newlines(buffer);
	password = strdup(buffer);
	memset(buffer, 0, sizeof(buffer));

	return password;
}
