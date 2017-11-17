/*
 * Copyright (C) 2017 Vlasta Vesely <vlastavesely@protonmail.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <CUnit/CUnit.h>

#include "../src/utils.h"

static void test_utils_urlencode_to_buf(void)
{
	char buffer[128];

	urlencode_to_buf("some thing / %", buffer);
	CU_ASSERT_STRING_EQUAL("some+thing+%2f+%25", buffer);
}

static void test_utils_html_decode_entities_to_buf_named(void)
{
	char buffer[128];

	html_decode_entities_to_buf("&amp; &lt;b&gt;", buffer);
	CU_ASSERT_STRING_EQUAL("& <b>", buffer);

	html_decode_entities_to_buf("&quo;", buffer);
	CU_ASSERT_STRING_EQUAL("?", buffer); // ? = unimplemented
}

static void test_utils_html_decode_entities_to_buf_by_code(void)
{
	char buffer[128];

	/* <= 0x00007f */
	html_decode_entities_to_buf("&#x00028; &#40;", buffer);
	CU_ASSERT_STRING_EQUAL("( (", buffer);

	/* <= 0x0007ff */
	html_decode_entities_to_buf("&#x000f1; &#x0f1; &#xf1;", buffer);
	CU_ASSERT_STRING_EQUAL("ñ ñ ñ", buffer);

	/* <= 0x00ffff */
	html_decode_entities_to_buf("&#x02ad8;&#10968;", buffer);
	CU_ASSERT_STRING_EQUAL("⫘⫘", buffer);

	/* <= 0x10ffff */
	html_decode_entities_to_buf("&#x1d55e;", buffer);
	CU_ASSERT_STRING_EQUAL("𝕞", buffer);
}

void test_utils_add_tests(struct CU_Suite *suite)
{
	CU_add_test(suite, "test_utils_html_decode_entities_to_buf_named",
		test_utils_html_decode_entities_to_buf_named);
	CU_add_test(suite, "test_utils_html_decode_entities_to_buf_by_code",
		test_utils_html_decode_entities_to_buf_by_code);
	CU_add_test(suite, "test_utils_urlencode_to_buf",
		test_utils_urlencode_to_buf);
}
