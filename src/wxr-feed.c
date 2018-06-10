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
	struct post *attachments;
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
	char *content;
	const char *ptr;

	while (node->prev && node->prev->type == XML_COMMENT_NODE) {
		node = node->prev;

		content = (char *) xmlNodeGetContent(node);
		ptr = strstr(content, "This is a WordPress eXtended RSS file");
		xmlFree(content);

		if (ptr != NULL)
			return true;
	}

	return false;
}

/******************************************************************************/

static struct post *rss_parse_post(xmlNode *item)
{
	struct post *post;
	xmlNode *child;

	post = malloc(sizeof(*post));
	if (post == NULL)
		return ERR_PTR(-ENOMEM);

	post->type = WXR_POST_TYPE_POST;
	post->attachment_url = NULL;
	post->next = NULL;

	for (child = item->children; child; child = child->next) {
		if (strcmp((char *) child->name, "title") == 0) {
			post->name = (char *) xmlNodeGetContent(child);

		} else if (strcmp((char *) child->name, "post_type") == 0) {
			xmlChar *data = xmlNodeGetContent(child);

			if (strcmp((char *) data, "page") == 0)
				post->type = WXR_POST_TYPE_PAGE;
			else if (!strcmp((char *) data, "attachment"))
				post->type = WXR_POST_TYPE_ATTACHMENT;

			xmlFree(data);
		} else if (strcmp((char *) child->name, "link") == 0) {
			post->url = (char *) xmlNodeGetContent(child);
		} else if (strcmp((char *) child->name, "attachment_url") == 0) {
			post->attachment_url = (char *) xmlNodeGetContent(child);
		}
	}

	return post;
}

static int rss_process_items(xmlNode *channel, struct wxr_feed *feed)
{
	xmlNode *item;
	struct post *post;
	struct post **posts = &(feed->posts);
	struct post **pages = &(feed->pages);
	struct post **attachments = &(feed->attachments);

	for (item = channel->children; item; item = item->next)
		if (strcmp((char *) item->name, "item") == 0) {

			post = rss_parse_post(item);
			if (IS_ERR(post))
				return -1;

			switch (post->type) {
			case WXR_POST_TYPE_POST:
				*posts = post;
				posts = &(*posts)->next;
				break;
			case WXR_POST_TYPE_PAGE:
				*pages = post;
				pages = &(*pages)->next;
				break;
			case WXR_POST_TYPE_ATTACHMENT:
				*attachments = post;
				attachments = &(*attachments)->next;
				break;
			}
		}

	return 0;
}

static int wxr_parse_posts(struct wxr_feed *feed)
{
	xmlNode *channel;
	int retval = 0;

	for (channel = feed->rss->children; channel; channel = channel->next)
		if (strcmp((char *) channel->name, "channel") == 0) {
			retval = rss_process_items(channel, feed);
			if (retval != 0)
				return retval;
		}

	return retval;
}

/******************************************************************************/

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

	memset(feed, 0, sizeof(*feed));

	feed->doc = xmlReadFile(filename, NULL, XML_PARSE_FLAGS);
	if (!feed->doc) {
		err = -EINVALXML;
		goto drop_feed;
	}

	feed->rss = xmlDocGetRootElement(feed->doc);
	if (strcmp("rss", (char *) feed->rss->name)) {
		err = -EINVALROOT;
		goto drop_feed;
	}

	if (!wxr_feed_has_signature_comment(feed->rss)) {
		err = -EMISSSIG;
		goto drop_feed;
	}

	err = wxr_parse_posts(feed);
	if (err)
		feed = ERR_PTR(err);

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
	return feed->pages;
}

struct post *wxr_feed_get_attachments(struct wxr_feed *feed)
{
	return feed->attachments;
}
