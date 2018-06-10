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

#ifndef __WXR_FEED_H
#define __WXR_FEED_H

#define EINVALXML	1
#define EMISSSIG	2
#define EINVALROOT	3

enum post_type {
	WXR_POST_TYPE_POST = 0,
	WXR_POST_TYPE_PAGE = 1,
	WXR_POST_TYPE_ATTACHMENT = 2
};

struct wxr_feed;

struct post {
	char *name;
	char *url;
	char *attachment_url;
	enum post_type type;
	struct post *next;
};

struct wxr_feed *wxr_feed_load(const char *filename);
void drop_wxr_feed(struct wxr_feed *feed);

struct post *wxr_feed_get_posts(struct wxr_feed *feed);
struct post *wxr_feed_get_pages(struct wxr_feed *feed);
struct post *wxr_feed_get_attachments(struct wxr_feed *feed);

#endif /* __WXR_FEED_H */
