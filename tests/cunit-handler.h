#ifndef __CUNIT_HANDLER_H
#define __CUNIT_HANDLER_H

#include <CUnit/CUnit.h>

CU_EXPORT CU_ErrorCode CU_custom_run_tests(void);
CU_EXPORT void CU_custom_show_failures(struct CU_FailureRecord *failure);

#endif /* __CUNIT_HANDLER_H */
