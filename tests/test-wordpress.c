#include <stdio.h>
#include <unistd.h>
#include <CUnit/CUnit.h>

#include "../src/wordpress.h"

static const char *wpurl = "http://localhost/wordpress";
static const char *username = "test";
static const char *password = "secret";
static const char *output_file = "/tmp/wp-backup-test.xml";

static void test_wordpress_login_logout(void)
{
	struct wordpress *wordpress;
	FILE *fp;
	int ret;

	if ((fp = fopen(output_file, "r")) != NULL) {
		fclose(fp);
		unlink(output_file);
	}

	wordpress = wordpress_create(wpurl);

	ret = wordpress_login(wordpress, username, password);
	CU_ASSERT_EQUAL(0, ret);

	ret = wordpress_export(wordpress, output_file);
	CU_ASSERT_EQUAL(0, ret);

	ret = wordpress_logout(wordpress);
	CU_ASSERT_EQUAL(0, ret);

	fp = fopen(output_file, "r");
	CU_ASSERT_PTR_NOT_NULL(fp);
	fclose(fp);
}

void test_wordpress_add_tests(struct CU_Suite *suite)
{
	CU_add_test(suite, "test_wordpress_login_logout",
		test_wordpress_login_logout);
}
