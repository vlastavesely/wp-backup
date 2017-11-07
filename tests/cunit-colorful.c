/*
 * cunit-colorful.c - colored output handler for cUnit framework.
 *
 * Copyright (C) 2017  Vlasta Vesely <vlastavesely@protonmail.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <assert.h>
#include "cunit-colorful.h"

static void cunit_colorful_test_complete_msg_handler(struct CU_Test *test,
		struct CU_Suite *suite,
		struct CU_FailureRecord *failures)
{
	assert(suite != NULL);
	assert(test != NULL);

	fprintf(stdout, failures == NULL ? "." : "\x1b[41;1mF\x1b[0m");
	fflush(stdout);
}

static void cunit_colorful_show_failures(struct CU_FailureRecord *failure)
{
	while (failure) {
		fprintf(stdout, "\x1b[31;1m-- FAILED %s:%d\x1b[0m\n   %s\x1b[0m\n\n",
			failure->strFileName, failure->uiLineNumber,
			failure->strCondition);
		failure = failure->pNext;
	}
}

static void cunit_colorful_show_summary(void)
{
	struct CU_RunSummary *summary = CU_get_run_summary();

	if (summary->nFailureRecords)
		fprintf(stdout,
			"\x1b[41;1mFAILURES! (%d test%s, %d failure%s, %1.3f seconds)\x1b[0m\n",
			summary->nTestsRun, summary->nTestsRun == 1 ? "" : "s",
			summary->nTestsFailed,
			summary->nTestsFailed == 1 ? "" : "s",
			summary->ElapsedTime);
	else
		fprintf(stdout,
			"\x1b[42;1mOK (%d test%s, %1.3f seconds)\x1b[0m\n",
			summary->nTestsRun, summary->nTestsRun == 1 ? "" : "s",
			summary->ElapsedTime);
}

static void cunit_colorful_all_tests_complete_msg_handler(struct CU_FailureRecord *failure)
{
	fprintf(stdout, "\n\n");

	if (failure)
		cunit_colorful_show_failures(failure);

	cunit_colorful_show_summary();
}

static CU_ErrorCode cunit_colorful_initialize(void)
{
	CU_set_error(CUE_SUCCESS);

	fprintf(stdout, " ____ _  _ __ _ _ ___\n");
	fprintf(stdout, " |___ |__| | \\| |  |  %s\n\n", CU_VERSION);

	CU_set_test_start_handler(NULL);
	CU_set_test_complete_handler(cunit_colorful_test_complete_msg_handler);
	CU_set_all_test_complete_handler(cunit_colorful_all_tests_complete_msg_handler);
	/*
	 * If any assertion has failed, we want to ignore the failure
	 * and keep the testing process running (if possible).
	 */
	CU_set_error_action(CUEA_IGNORE);

	return CU_get_error();
}

CU_ErrorCode CU_colorful_run_tests(void)
{
	CU_ErrorCode code;

	if (CU_get_registry() == NULL)
		return CUE_NOREGISTRY;

	if ((code = cunit_colorful_initialize()) == CUE_SUCCESS) {
		code = CU_run_all_tests();

		/*
		 * If any test has failed, we do *not* want to get a zero
		 * return code.
		 */
		if (CU_get_failure_list())
			code = -1;
		CU_set_error(code);
	}

	return code;
}
