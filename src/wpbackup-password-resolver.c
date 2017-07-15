#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include <wpbackup-password-resolver.h>

static void print_prompt()
{
	fprintf(stderr, "Enter your WordPress password: ");
}

char *wpbackup_password_resolver_resolve_password()
{
	char *password = getenv("WPPASS");
	if (password) {
		return password;
	} else {
		char buffer[256];

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
			fgets(buffer, sizeof buffer, stdin);
		}

		password = malloc(strlen(buffer) + 1);
		strcpy(password, buffer);
		return password;
	}
}
