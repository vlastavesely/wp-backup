#include <stdio.h>
#include <CUnit/CUnit.h>

#include "../src/error-handler.h"
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

static void test_wxr_feed_wxr_feed_load_fail()
{
	struct wxr_feed *feed;
	struct error *error;

	/* Invalid XML */
	create_test_wxr_feed_mock("This ain't a valid XML file.");
	feed = wxr_feed_load(test_wxr_feed_filename, &error);
	CU_ASSERT_PTR_NULL(feed);
	CU_ASSERT_EQUAL(WXR_FEED_ERROR_INVALID_XML,
			error->code);
	CU_ASSERT_STRING_EQUAL("File does not contain valid XML data.",
			       error->message);
	error_free(error);

	/* Missing header comment */
	create_test_wxr_feed_mock("<valid><xml/></valid>");
	feed = wxr_feed_load(test_wxr_feed_filename, &error);
	CU_ASSERT_PTR_NULL(feed);
	CU_ASSERT_EQUAL(WXR_FEED_ERROR_MISSING_SIGNATURE,
			error->code);
	CU_ASSERT_STRING_EQUAL("File does not contain signature comment.",
			       error->message);
	error_free(error);

	/* Root is not 'rss' */
	create_test_wxr_feed_mock("<!-- This is a WordPress eXtended RSS file -->"
		"<valid><xml/></valid>");
	feed = wxr_feed_load(test_wxr_feed_filename, &error);
	CU_ASSERT_PTR_NULL(feed);
	CU_ASSERT_EQUAL(WXR_FEED_ERROR_ROOT_IS_NOT_RSS,
			error->code);
	CU_ASSERT_STRING_EQUAL("Root element is not 'rss'.",
			       error->message);
	error_free(error);
}

static void test_wxr_feed_wxr_feed_load_success()
{
	struct wxr_feed *feed;
	struct error *error;

	/* Valid */
	create_test_wxr_feed_mock("<!-- This is a WordPress eXtended RSS file -->"
		"<rss><xml/></rss>");
	feed = wxr_feed_load(test_wxr_feed_filename, &error);
	CU_ASSERT_PTR_NOT_NULL(feed);
	wxr_feed_free(feed);
}

void test_wxr_feed_add_tests(struct CU_Suite *suite)
{
	CU_add_test(suite, "test_wxr_feed_wxr_feed_load_fail",
		test_wxr_feed_wxr_feed_load_fail);
	CU_add_test(suite, "test_wxr_feed_wxr_feed_load_success",
		test_wxr_feed_wxr_feed_load_success);
}
