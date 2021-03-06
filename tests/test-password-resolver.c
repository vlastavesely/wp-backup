/*
 * Copyright (C) 2017 Vlasta Vesely <vlastavesely@protonmail.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <CUnit/CUnit.h>

#include "../src/password-resolver.h"

static void test_utils_resolve_from_envvar(void)
{
	char *password;

	setenv("WPPASS", "iLoveYou123", 1);
	password = password_resolver_resolve_password();
	unsetenv("WPPASS");
	CU_ASSERT_STRING_EQUAL("iLoveYou123", password);
}

/*
 * FIXME - any possibility to mock this?
 *
static void test_utils_resolve_from_stdin()
{
	char *password;

	password = password_resolver_resolve_password();
}
*/

void test_password_resolver_add_tests(struct CU_Suite *suite)
{
	CU_add_test(suite, "test_utils_resolve_from_envvar",
		test_utils_resolve_from_envvar);
/*	CU_add_test(suite, "test_utils_resolve_from_stdin",
		test_utils_resolve_from_stdin);
*/
}
