#include <CUnit/CUnit.h>

#include "../src/error-handler.h"

static char buffer[4096];

static void test_warning_routine(const char *warn, va_list params)
{
	vsnprintf(buffer, sizeof(buffer), warn, params);
}

static void test_fatal_routine(const char *err, va_list params)
{
	vsnprintf(buffer, sizeof(buffer), err, params);
}

static void test_error_handler_warning()
{
	set_warning_routine(test_warning_routine);
	warning("Some warning.");
	CU_ASSERT_STRING_EQUAL("Some warning.", buffer);
}

static void test_error_handler_fatal()
{
	set_fatal_routine(test_fatal_routine);
	fatal("An fatal error occured.");
	CU_ASSERT_STRING_EQUAL("An fatal error occured.", buffer);
}

void test_error_handler_add_tests(struct CU_Suite *suite)
{
	CU_add_test(suite, "test_error_handler_warning",
		test_error_handler_warning);
	CU_add_test(suite, "test_error_handler_fatal",
		test_error_handler_fatal);
}
