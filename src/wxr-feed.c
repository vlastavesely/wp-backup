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
	struct post *posts;
	struct post *pages;
};

#define XML_PARSE_FLAGS		XML_PARSE_NOERROR | XML_PARSE_NOWARNING

/*
 * One of things identifying a valid WXR feed, is a header comment. It does
 * contain some information about how to import the feed back into your
 * WordPress installation. If this header is missing, we consider the given
 * file to be not a valid WXR feed. To be valid, it needs contain following
 * sentence:
 *
 *   "This is a WordPress eXtended RSS file generated by WordPress as an
 *   export of your site."
 *
 * If it does, it is unquestionable that this is a valid dump.
 */
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

/*
 * WXR dump loader.
 *
 * Valid file must:
 *   1. be a valid XML,
 *   2. have <rss> element as root and
 *   3. contain header comment identifying that is _really_ is a WXR 
 *      file and not just a regular RSS.
 */
struct wxr_feed *wxr_feed_load(const char *filename)
{
	struct wxr_feed *feed;
	int err;

	feed = malloc(sizeof(*feed));
	if (!feed) {
		err = -ENOMEM;
		goto out;
	}

	feed->posts = NULL;
	feed->pages = NULL;

	feed->doc = xmlReadFile(filename, NULL, XML_PARSE_FLAGS);
	if (!feed->doc) {
		err = -EINVALXML;
		goto drop_feed;
	}

	feed->rss = xmlDocGetRootElement(feed->doc);
	if (strcmp("rss", feed->rss->name)) {
		err = -EINVALROOT;
		goto drop_feed;
	}

	if (!wxr_feed_has_signature_comment(feed->rss)) {
		err = -EMISSSIG;
		goto drop_feed;
	}

out:
	return feed;
drop_feed:
	drop_wxr_feed(feed);
	feed = ERR_PTR(err);
	goto out;
}

void drop_wxr_feed(struct wxr_feed *feed)
{
	if (IS_ERR_OR_NULL(feed))
		return;

	if (feed->doc)
		xmlFreeDoc(feed->doc);
	xmlCleanupParser();
	free(feed);
}

/******************************************************************************/

struct post *wxr_feed_get_posts(struct wxr_feed *feed)
{
	return feed->posts;
}

struct post *wxr_feed_get_pages(struct wxr_feed *feed)
{
	return feed->posts;
}
