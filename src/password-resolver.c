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

#define PASSWORD_BUFFER_SIZE 256

static void print_prompt()
{
	fprintf(stderr, "Enter your WordPress password: ");
}

static void trim_trailing_newlines(char **str)
{
	char *end;

	end = *str + strlen(*str) - 1;
	while(end > *str && ((char) *end == '\n' || (char) *end == '\r')) {
		end--;
	}
	end++;

	*end = 0;
}

char *password_resolver_resolve_password()
{
	char *password = getenv("WPPASS");
	if (password) {
		return strdup(password);
	} else {
		char *buffer = malloc(PASSWORD_BUFFER_SIZE);
		memset(buffer, 0, PASSWORD_BUFFER_SIZE);

		if (isatty(0)) {
			struct termios oflags;
			struct termios nflags;

			tcgetattr(0, &oflags);
			nflags = oflags;
			nflags.c_lflag &= ~ECHO;
			nflags.c_lflag |= ECHONL;

			if (tcsetattr(0, TCSANOW, &nflags) != 0) {
				perror("tcsetattr");
				return NULL;
			}

			print_prompt();

			fgets(buffer, sizeof(buffer), stdin);
			buffer[strlen(buffer)-1] = 0;
			if (tcsetattr(0, TCSANOW, &oflags) != 0) {
				perror("tcsetattr");
				return NULL;
			}
		} else {
			fgets(buffer, PASSWORD_BUFFER_SIZE, stdin);
		}

		trim_trailing_newlines(&buffer);
		password = malloc(strlen(buffer) + 1);
		strcpy(password, buffer);
		memset(buffer, 0, PASSWORD_BUFFER_SIZE);

		return password;
	}
}
