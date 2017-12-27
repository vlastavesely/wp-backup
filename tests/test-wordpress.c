/*
 * Copyright (C) 2017 Vlasta Vesely <vlastavesely@protonmail.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdbool.h>
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

	ret = wordpress_export(wordpress, output_file, true);
	CU_ASSERT_EQUAL(0, ret);

	ret = wordpress_logout(wordpress);
	CU_ASSERT_EQUAL(0, ret);

	fp = fopen(output_file, "r");
	CU_ASSERT_PTR_NOT_NULL(fp);
	fclose(fp);

	drop_wordpress(wordpress);
}

void test_wordpress_add_tests(struct CU_Suite *suite)
{
	CU_add_test(suite, "test_wordpress_login_logout",
		test_wordpress_login_logout);
}
