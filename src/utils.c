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
