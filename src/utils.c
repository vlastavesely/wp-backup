#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <wp-backup/utils.h>

static char byte_to_hex(char code)
{
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}

/* http://www.geekhideout.com/urlcode.shtml */
char *urlencode(const char *url)
{
	const char *pstr = url;
	char *buf;
	char *pbuf;

	buf = malloc(strlen(url) * 3 + 1);
	pbuf = buf;

	while (*pstr) {
		if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' ||
		    *pstr == '.' || *pstr == '~') {
			*pbuf++ = *pstr;
		} else if (*pstr == ' ') {
			*pbuf++ = '+';
		} else {
			*pbuf++ = '%';
			*pbuf++ = byte_to_hex(*pstr >> 4);
			*pbuf++ = byte_to_hex(*pstr & 15);
		}
		pstr++;
	}
	*pbuf = '\0';

	return buf;
}
