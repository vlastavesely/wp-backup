#ifndef __OPTIONS_H
#define __OPTIONS_H

#include <stdbool.h>

struct options
{
	const char *username;
	const char *wpurl;
	const char *output_file;
	bool version;
	bool help;
};

struct options *options_parse(int argc, char **argv);

void options_free(struct options *options);

#endif /* __OPTIONS_H */
