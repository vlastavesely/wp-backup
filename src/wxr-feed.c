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
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

struct wxr_feed {
	xmlDoc *doc;
	struct post *posts;
};

#define XML_PARSE_FLAGS		XML_PARSE_NOERROR | XML_PARSE_NOWARNING


static bool wxr_feed_has_signature_comment(xmlNode *node)
{
	while (node->prev && node->prev->type == XML_COMMENT_NODE) {
		node = node->prev;
		if (strstr((const char *) node->content,
			   "This is a WordPress eXtended RSS file"))
			return true;
	}

	return false;
}

static char *xml_node_content(const char *xpath, xmlNode *node,
			      xmlXPathContext *context)
{
	xmlXPathObject *result;
	char *retval = NULL;

	context->node = node;
	result = xmlXPathEvalExpression(BAD_CAST xpath, context);
	if (result == NULL)
		return NULL;

	if (result->nodesetval == NULL || result->nodesetval->nodeNr == 0)
		goto drop_result;

	retval = (char *) xmlNodeGetContent(result->nodesetval->nodeTab[0]);

drop_result:
	xmlXPathFreeObject(result);
	return retval;
}

static struct post *wxr_parse_post(xmlNode *node, xmlXPathContext *context)
{
	struct post *post;
	char *ptr;

	post = malloc(sizeof(*post));
	if (post == NULL)
		return ERR_PTR(-ENOMEM);

	post->name = xml_node_content("title", node, context);
	post->url = xml_node_content("link", node, context);

	ptr = xml_node_content("wp:post_type", node, context);
	if (strcmp(ptr, "page") == 0)
		post->type = WXR_POST_TYPE_PAGE;
	else if (strcmp(ptr, "attachment") == 0)
		post->type = WXR_POST_TYPE_ATTACHMENT;
	else
		post->type = WXR_POST_TYPE_POST;

	post->attachment_url = xml_node_content("wp:attachment_url", node, context);
	post->next = NULL;

	return post;
}

int wxr_parse_posts(struct wxr_feed *feed)
{
	xmlNode *root = xmlDocGetRootElement(feed->doc);
	xmlXPathContext *context;
	xmlXPathObject *result;
	xmlNodeSet *nodes;
	xmlNs *ns;
	struct post **posts = &(feed->posts);
	int retval = 0, i, nnodes;

	context = xmlXPathNewContext(feed->doc);
	if (context == NULL) {
		retval = -1;
		goto out;
	}

	for (ns = root->nsDef; ns; ns = ns->next)
		if (xmlXPathRegisterNs(context, ns->prefix, ns->href) != 0) {
			retval = -2;
			goto drop_context;
		}

	result = xmlXPathEvalExpression(BAD_CAST "//rss/channel/item", context);
	if (result == NULL) {
		retval = -3;
		goto drop_context;
	}

	nodes = result->nodesetval;
	nnodes = nodes ? nodes->nodeNr : 0;
	for (i = 0; i < nnodes; i++) {
		*posts = wxr_parse_post(nodes->nodeTab[i], context);
		posts = &(*posts)->next;
	}

	xmlXPathFreeObject(result);
drop_context:
	xmlXPathFreeContext(context);
out:
	return retval;

	return 0;
}

struct wxr_feed *wxr_feed_load(const char *filename)
{
	struct wxr_feed *feed;
	xmlNode *root;
	int retval = 0;

	feed = malloc(sizeof(*feed));
	if (!feed) {
		feed = ERR_PTR(-ENOMEM);
		goto out;
	}

	memset(feed, 0, sizeof(*feed));
	feed->doc = xmlReadFile(filename, NULL, XML_PARSE_FLAGS);
	if (!feed->doc) {
		retval = -EINVALXML;
		goto drop_feed;
	}

	root = xmlDocGetRootElement(feed->doc);
	if (strcmp("rss", (char *) root->name) != 0) {
		retval = -EINVALROOT;
		goto drop_feed;
	}

	if (wxr_feed_has_signature_comment(root) == false) {
		retval = -EMISSSIG;
		goto drop_feed;
	}

	retval = wxr_parse_posts(feed);
	if (retval != 0)
		feed = ERR_PTR(retval);

out:
	return feed;

drop_feed:
	wxr_feed_drop(feed);
	feed = ERR_PTR(retval);
	goto out;
}

void wxr_feed_drop(struct wxr_feed *feed)
{
	if (IS_ERR_OR_NULL(feed))
		return;

	if (feed->doc)
		xmlFreeDoc(feed->doc);
	xmlCleanupParser();
	free(feed);
}

struct post *wxr_feed_get_posts(struct wxr_feed *feed)
{
	return feed->posts;
}
