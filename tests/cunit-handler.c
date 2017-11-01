#include "cunit-handler.h"

CU_ErrorCode CU_custom_run_tests(void)
{

}

void CU_custom_show_failures(struct CU_FailureRecord *failure)
{
	int i = 1;

	while (failure) {
		fprintf(stdout, "  %d. %s:%d  - %s\n", i++,
			failure->strFileName, failure->uiLineNumber,
			failure->strCondition);
		failure = failure->pNext;
	}
}
