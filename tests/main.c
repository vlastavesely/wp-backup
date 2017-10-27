#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "test-password-resolver.h"
#include "test-wxr-feed.h"
#include "test-error.h"
#include "test-utils.h"

int main(int argc, const char **argv)
{
	struct CU_Suite *suite;

	CU_initialize_registry();

	suite = CU_add_suite("wpbackup_suite", NULL, NULL);

	test_password_resolver_add_tests(suite);
	test_wxr_feed_add_tests(suite);
	test_error_add_tests(suite);
	test_utils_add_tests(suite);

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_basic_show_failures(CU_get_failure_list());
	CU_cleanup_registry();

	return CU_get_error();
}
