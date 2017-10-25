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
#include <assert.h>
#include <ctype.h>

#include <wp-backup/utils.h>

static char byte_to_hex(char code)
{
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}

/* Inspired by: http://www.geekhideout.com/urlcode.shtml */
void urlencode_to_buf(const char *str, char *buf)
{
	while (*str) {
		if (isalnum(*str) || *str == '-' || *str == '_' ||
		    *str == '.' || *str == '~') {
			*buf++ = *str;
		} else if (*str == ' ') {
			*buf++ = '+';
		} else {
			*buf++ = '%';
			*buf++ = byte_to_hex(*str >> 4);
			*buf++ = byte_to_hex(*str & 15);
		}
		str++;
	}
	*buf = '\0';
}

static void pack_unicode_char(char **dest, unsigned long c)
{
	assert(c <= 0x10ffff);

	if (c <= 0x00007f) {
		*(*dest)   = c;
	} else if (c <= 0x0007ff) {
		*(*dest)++ = (0xc0 |  (c >> 6)         );
		*(*dest)   = (0x80 |  (c        & 0x3f));
	} else if (c <= 0x00ffff) {
		*(*dest)++ = (0xe0 |  (c >> 12)        );
		*(*dest)++ = (0x80 | ((c >> 6)  & 0x3f));
        	*(*dest)   = (0x80 |  (c        & 0x3f));
	} else if (c <= 0x10ffff) {
		*(*dest)++ = (0xf0 |  (c >> 18)        );
		*(*dest)++ = (0x80 | ((c >> 12) & 0x3f));
		*(*dest)++ = (0x80 | ((c >> 6)  & 0x3f));
		*(*dest)   = (0x80 |  (c        & 0x3f));
	}
}

static void decode_named_entity(char **dest, const char *src)
{
	if (!strncmp(src, "amp;", 4))
		*(*dest) = '&';
	else if (!strncmp(src, "lt;", 3))
		*(*dest) = '<';
	else if (!strncmp(src, "gt;", 3))
		*(*dest) = '>';
	else
		/* TODO add all other common entities */
		*(*dest) = '?';
}

void html_decode_entities_to_buf(const char *str, char *buf)
{
	unsigned long code;
	int is_hex;
	char *ptr = buf, *end;

	while (*str) {
		if (*str == '&') {
			if (str[1] == '#') {
				is_hex = (str[2] == 'x' || str[2] == 'X');
				code = strtoul(str + (is_hex ? 3 : 2),
					       &end, is_hex ? 16 : 10);
				pack_unicode_char(&ptr, code);
			} else {
				decode_named_entity(&ptr, ++str);
				end = strchr(str, ';');
			}
			str = end;
		} else {
			*ptr = *str;
		}
		str++;
		ptr++;
	}
	*ptr = '\0';
}
