/*
 * cunit-colorful.c - colored output handler for cUnit framework.
 *
 * Copyright (C) 2017  Vlasta Vesely <vlastavesely@protonmail.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __CUNIT_COLORFUL_H
#define __CUNIT_COLORFUL_H

#include <CUnit/CUnit.h>

CU_EXPORT CU_ErrorCode CU_colorful_run_tests(void);

#endif /* __CUNIT_COLORFUL_H */
