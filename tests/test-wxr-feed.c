/*
 * Copyright (C) 2017 Vlasta Vesely <vlastavesely@protonmail.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <CUnit/CUnit.h>

#include "../src/err.h"
#include "../src/wxr-feed.h"

static const char *test_wxr_feed_filename = "/tmp/wxr.xml";

static void create_test_wxr_feed_mock(const char *xml)
{
	FILE *fp;
	if ((fp = fopen(test_wxr_feed_filename, "w")) == NULL)
		die("Failed to create a temp WXR feed.");
	fprintf(fp, "%s", xml);
	fclose(fp);
}

static void test_wxr_feed_wxr_feed_load_fail(void)
{
	struct wxr_feed *feed;

	/* Invalid XML */
	create_test_wxr_feed_mock("This ain't a valid XML file.");
	feed = wxr_feed_load(test_wxr_feed_filename);
	CU_ASSERT_TRUE(IS_ERR(feed));
	CU_ASSERT_EQUAL(-EINVALXML, PTR_ERR(feed));

	/* Root is not 'rss' */
	create_test_wxr_feed_mock("<!-- This is a WordPress eXtended RSS file -->"
		"<valid><xml/></valid>");
	feed = wxr_feed_load(test_wxr_feed_filename);
	CU_ASSERT_TRUE(IS_ERR(feed));
	CU_ASSERT_EQUAL(-EINVALROOT, PTR_ERR(feed));

	/* Missing header comment */
	create_test_wxr_feed_mock("<rss><xml/></rss>");
	feed = wxr_feed_load(test_wxr_feed_filename);
	CU_ASSERT_TRUE(IS_ERR(feed));
	CU_ASSERT_EQUAL(-EMISSSIG, PTR_ERR(feed));
}

static void test_wxr_feed_wxr_feed_load_success(void)
{
	struct wxr_feed *feed;

	/* Valid */
	create_test_wxr_feed_mock("<!-- This is a WordPress eXtended RSS file -->"
		"<rss><xml/></rss>");
	feed = wxr_feed_load(test_wxr_feed_filename);
	CU_ASSERT_FALSE(IS_ERR(feed));
	drop_wxr_feed(feed);
}

void test_wxr_feed_add_tests(struct CU_Suite *suite)
{
	CU_add_test(suite, "test_wxr_feed_wxr_feed_load_fail",
		test_wxr_feed_wxr_feed_load_fail);
	CU_add_test(suite, "test_wxr_feed_wxr_feed_load_success",
		test_wxr_feed_wxr_feed_load_success);
}
