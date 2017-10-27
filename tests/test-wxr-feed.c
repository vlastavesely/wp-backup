#include <stdio.h>
#include <CUnit/CUnit.h>

#include "../src/error.h"
#include "../src/wxr-feed.h"

static const char *test_wxr_feed_filename = "/tmp/wxr.xml";

static void create_test_wxr_feed_mock(const char *xml)
{
	FILE *fp;
	if ((fp = fopen(test_wxr_feed_filename, "w")) == NULL)
		fatal("Failed to create a temp WXR feed.");
	fprintf(fp, "%s", xml);
	fclose(fp);
}

static void test_wxr_feed_wxr_feed_load()
{
	struct wxr_feed *feed;

	create_test_wxr_feed_mock("This ain't a valid XML file.");
	feed = wxr_feed_load(test_wxr_feed_filename);
	CU_ASSERT_PTR_NULL(feed);

	create_test_wxr_feed_mock("<valid><xml/></valid>");
	feed = wxr_feed_load(test_wxr_feed_filename);
	CU_ASSERT_PTR_NOT_NULL(feed);
	wxr_feed_free(feed);

}

void test_wxr_feed_add_tests(struct CU_Suite *suite)
{
	CU_add_test(suite, "test_wxr_feed_wxr_feed_load",
		test_wxr_feed_wxr_feed_load);
}