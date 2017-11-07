#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "cunit-colorful.h"
#include "test-options.h"
#include "test-password-resolver.h"
#include "test-http.h"
#include "test-wxr-feed.h"
#include "test-error-handler.h"
#include "test-error.h"
#include "test-utils.h"

static int cunit_add_tests(struct CU_Suite *suite)
{
	test_options_add_tests(suite);
	test_password_resolver_add_tests(suite);
	test_http_add_tests(suite);
	test_wxr_feed_add_tests(suite);
	test_error_handler_add_tests(suite);
	test_error_add_tests(suite);
	test_utils_add_tests(suite);

	return CU_get_error();
}

int main(int argc, const char **argv)
{
	struct CU_Suite *suite;
	int code;

	if ((code = CU_initialize_registry()) != CUE_SUCCESS)
		return code;

	if ((suite = CU_add_suite("suite", NULL, NULL)) == NULL) {
		code = -1;
		goto out;
	}

	if ((code = cunit_add_tests(suite)) != CUE_SUCCESS)
		goto out;

	/*
	 * If some test has failed, return code will be
	 * a non-zero value.
	 */
	code = CU_colorful_run_tests();

out:
	CU_cleanup_registry();
	return code;
}
