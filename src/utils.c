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

static inline char nibble_to_hex(char code)
{
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}

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
			*buf++ = nibble_to_hex(*str >> 4);
			*buf++ = nibble_to_hex(*str & 15);
		}
		str++;
	}
	*buf = '\0';
}

static void pack_unicode_char(char **dest, unsigned long c)
{
	if (c <= 0x00007f) {
		*(*dest)   = c;
	} else if (c <= 0x0007ff) {
		*(*dest)++ = (0xc0 | ((c >>  6) & 0xff));
		*(*dest)   = (0x80 | ((c >>  0) & 0x3f));
	} else if (c <= 0x00ffff) {
		*(*dest)++ = (0xe0 | ((c >> 12) & 0xff));
		*(*dest)++ = (0x80 | ((c >>  6) & 0x3f));
		*(*dest)   = (0x80 | ((c >>  0) & 0x3f));
	} else if (c <= 0x10ffff) {
		*(*dest)++ = (0xf0 | ((c >> 18) & 0xff));
		*(*dest)++ = (0x80 | ((c >> 12) & 0x3f));
		*(*dest)++ = (0x80 | ((c >>  6) & 0x3f));
		*(*dest)   = (0x80 | ((c >>  0) & 0x3f));
	} else {
		warning("invalid unicode character.");
		*(*dest)   = '?';
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
	else {
		/* Other entities *should* not be needed in context
		 * we need. But you can never know when authors of
		 * WordPress decide to change something... */
		warning("unknown HTML entity.");
		*(*dest) = '?';
	}
}

void html_decode_entities_to_buf(const char *str, char *buf)
{
	unsigned long code;
	bool is_hex;
	char *ptr = buf, *end;

	while (*str) {
		if (*str == '&') {
			if (str[1] == '#') {
				is_hex = (str[2] == 'x' || str[2] == 'X');
				code = strtoul(str + (is_hex ? 3 : 2),
					       &end, is_hex ? 16 : 10);
				pack_unicode_char(&ptr, code);
				str = end;
			} else {
				decode_named_entity(&ptr, ++str);
				str = strchr(str, ';');
			}
		} else {
			*ptr = *str;
		}
		str++;
		ptr++;
	}
	*ptr = '\0';
}

char *mktemp_filename(const char *template)
{
	char *name;
	int fd, saved_errno;

	name = strdup(template);
	if (name == NULL)
		return ERR_PTR(-errno);

	fd = mkstemp(name);
	if (fd == -1)
		goto err;

	close(fd);
out:
	return name;
err:
	saved_errno = errno;
	free(name);
	name = ERR_PTR(-saved_errno);
	goto out;
}

int zeroize_file(const char *filename)
{
	struct stat st;
	void *map;
	size_t len;
	int fd, retval = 0;

	if (stat(filename, &st) != 0) {
		retval = -errno;
		goto out;
	}

	len = st.st_size;
	if (len == 0)
		goto out;

	fd = open(filename, O_RDWR);
	if (fd == -1) {
		retval = -errno;
		goto out;
	}

	map = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		retval = -errno;
		goto close_fd;
	}

	memset(map, 0, len);
	munmap(map, len);

close_fd:
	close(fd);
out:
	return retval;
}
