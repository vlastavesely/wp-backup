/*
 * Copyright (C) 2017 Vlasta Vesely <vlastavesely@protonmail.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <CUnit/CUnit.h>

#include "../src/err.h"

static char buffer[4096];

static void test_warning_routine(const char *warn, va_list params)
{
	vsnprintf(buffer, sizeof(buffer), warn, params);
}

static void test_die_routine(const char *err, va_list params)
{
	vsnprintf(buffer, sizeof(buffer), err, params);
}

static void test_err_warning(void)
{
	set_warning_routine(test_warning_routine);
	warning("Some warning.");
	CU_ASSERT_STRING_EQUAL("Some warning.", buffer);
}

static void test_err_die(void)
{
	set_die_routine(test_die_routine);
	die("An die error occured.");
	CU_ASSERT_STRING_EQUAL("An die error occured.", buffer);
}

void test_err_add_tests(struct CU_Suite *suite)
{
	CU_add_test(suite, "test_err_warning", test_err_warning);
	CU_add_test(suite, "test_err_die", test_err_die);
}
