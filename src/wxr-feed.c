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
#include "wxr-feed.h"
#include "err.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

struct wxr_feed {
	xmlDoc *doc;
	xmlNode *rss;
};

static bool wxr_feed_has_signature_comment(xmlNode *node)
{
	xmlChar *content;
	const char *ptr;

	while (node->prev && node->prev->type == XML_COMMENT_NODE) {
		node = node->prev;

		content = xmlNodeGetContent(node);
		ptr = strstr(content, "This is a WordPress eXtended RSS file");
		xmlFree(content);

		if (ptr)
			return true;
	}

	return false;
}

struct wxr_feed *wxr_feed_load(const char *filename)
{
	struct wxr_feed *feed;

	if ((feed = malloc(sizeof(*feed))) == NULL)
		return ERR_PTR(-ENOMEM);

	if ((feed->doc = xmlReadFile(filename, NULL,
	   XML_PARSE_NOERROR | XML_PARSE_NOWARNING)) == NULL)
		return ERR_PTR(-EINVALXML);

	feed->rss = xmlDocGetRootElement(feed->doc);
	if (strcmp("rss", feed->rss->name)) {
		wxr_feed_free(feed);
		return ERR_PTR(-EINVALROOT);
	}

	if (wxr_feed_has_signature_comment(feed->rss) == false) {
		wxr_feed_free(feed);
		return ERR_PTR(-EMISSSIG);
	}

	return feed;
}

void wxr_feed_free(struct wxr_feed *feed)
{
	if (feed == NULL)
		return;

	xmlFreeDoc(feed->doc);
	xmlCleanupParser();
	free(feed);
}
