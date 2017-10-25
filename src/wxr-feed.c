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
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <wp-backup/error.h>
#include <wp-backup/wxr-feed.h>

struct wxr_feed {
	xmlDoc *doc;
	xmlNode *rss;
};

struct wxr_feed *wxr_feed_load(const char *filename)
{
	struct wxr_feed *feed;

	feed = malloc(sizeof(*feed));
	feed->doc = xmlReadFile(filename, NULL, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);

	if (feed->doc == NULL)
		fatal("failed to load WXR feed.");

	feed->rss = xmlDocGetRootElement(feed->doc);

	return feed;
}

void wxr_feed_free(struct wxr_feed *feed)
{
	xmlFreeDoc(feed->doc);
	xmlCleanupParser();
	free(feed);
}
