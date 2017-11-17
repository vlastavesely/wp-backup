/*
 * Copyright (C) 2017 Vlasta Vesely <vlastavesely@protonmail.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <CUnit/CUnit.h>

#include "../src/error.h"

static void test_error_error(void)
{
	struct error *error;

	error = error_new(11, "This is a error.");
	CU_ASSERT_EQUAL(11, error->code);
	CU_ASSERT_STRING_EQUAL("This is a error.", error->message);
	error_free(error);

	error = error_new(44, "%d + %d is not %d.", 2, 2, 5);
	CU_ASSERT_EQUAL(44, error->code);
	CU_ASSERT_STRING_EQUAL("2 + 2 is not 5.", error->message);
	error_free(error);
}

void test_error_add_tests(struct CU_Suite *suite)
{
	CU_add_test(suite, "test_error_error",
		test_error_error);
}
