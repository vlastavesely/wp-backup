/*
 * Copyright (C) 2017 Vlasta Vesely <vlastavesely@protonmail.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <CUnit/CUnit.h>
#include <getopt.h>
#include <stdlib.h>

#include "../src/options.h"

static void getopt_reset(void)
{
	optind = 0;
}

static void test_options_options_parse_usage(void)
{
	struct options options;
	const char *argv[] = {"./progname"};

	getopt_reset();
	CU_ASSERT_EQUAL(0, options_parse(&options, 1, argv));
	CU_ASSERT_TRUE(options.help);
}

static void test_options_options_parse_help_explicit(void)
{
	struct options options;
	const char *argv[] = {"./progname", "--help"};

	getopt_reset();
	CU_ASSERT_EQUAL(0, options_parse(&options, 2, argv));
	CU_ASSERT_TRUE(options.help);
}

static void test_options_options_parse_version(void)
{
	struct options options;
	const char *argv[] = {"./progname", "--version"};

	getopt_reset();
	CU_ASSERT_EQUAL(0, options_parse(&options, 2, argv));
	CU_ASSERT_TRUE(options.version);
}


static void test_options_options_parse_quiet(void)
{
	struct options options;
	const char *argv[] = {"./progname", "--quiet"};

	getopt_reset();
	CU_ASSERT_EQUAL(0, options_parse(&options, 2, argv));
	CU_ASSERT_TRUE(options.quiet);
}

static void test_options_options_parse_missing_username(void)
{
	struct options options;
	char *errstr;
	const char *argv[]
		= {"./progname", "--wpurl", "https://example.com/"};

	getopt_reset();
	CU_ASSERT_EQUAL(-EMISSARG, options_parse(&options, 3, argv));

	errstr = options_errstr();
	CU_ASSERT_STRING_EQUAL("username cannot be empty.", errstr);
	free(errstr);
}

static void test_options_options_parse_missing_wpurl(void)
{
	struct options options;
	char *errstr;
	const char *argv[]
		= {"./progname", "--username", "admin"};

	getopt_reset();
	CU_ASSERT_EQUAL(-EMISSARG, options_parse(&options, 3, argv));

	errstr = options_errstr();
	CU_ASSERT_STRING_EQUAL("WordPress URL cannot be empty.", errstr);
	free(errstr);
}

static void test_options_options_parse_bad_wpurl(void)
{
	struct options options;
	char *errstr;
	const char *argv[]
		= {"./progname", "-u", "user", "--wpurl", "not-an-url"};

	getopt_reset();
	CU_ASSERT_EQUAL(-EBADARGVAL, options_parse(&options, 5, argv));

	errstr = options_errstr();
	CU_ASSERT_STRING_EQUAL("WordPress URL does not have 'http://' or 'https://' prefix.", errstr);
	free(errstr);
}

static void test_options_options_parse_missing_unrecognized(void)
{
	struct options options;
	char *errstr;
	const char *argv[]
		= {"./progname", "--xxx"};

	getopt_reset();
	CU_ASSERT_EQUAL(-EUNRECOG, options_parse(&options, 2, argv));

	errstr = options_errstr();
	CU_ASSERT_STRING_EQUAL("unrecognized option '--xxx'.", errstr);
	free(errstr);
}

static void test_options_options_parse_short(void)
{
	struct options options;
	const char *argv[]
		= {"./progname",
		   "-u", "admin",
		   "-w", "http://wp.com/",
		   "-o", "wp.xml"};

	getopt_reset();
	CU_ASSERT_EQUAL(0, options_parse(&options, 7, argv));
	CU_ASSERT_STRING_EQUAL("admin", options.username);
	CU_ASSERT_STRING_EQUAL("http://wp.com/", options.wpurl);
	CU_ASSERT_STRING_EQUAL("wp.xml", options.output_file);
}

static void test_options_options_parse_long(void)
{
	struct options options;
	const char *argv[]
		= {"./progname",
		   "--username", "admin",
		   "--wpurl", "http://wp.com/",
		   "--output-file", "wp.xml",
		   "--ignore-ssl-errors"};

	getopt_reset();
	CU_ASSERT_EQUAL(0, options_parse(&options, 8, argv));
	CU_ASSERT_STRING_EQUAL("admin", options.username);
	CU_ASSERT_STRING_EQUAL("http://wp.com/", options.wpurl);
	CU_ASSERT_STRING_EQUAL("wp.xml", options.output_file);
}

void test_options_add_tests(struct CU_Suite *suite)
{
	CU_add_test(suite, "test_options_options_parse_usage",
		test_options_options_parse_usage);
	CU_add_test(suite, "test_options_options_parse_help_explicit",
		test_options_options_parse_help_explicit);
	CU_add_test(suite, "test_options_options_parse_version",
		test_options_options_parse_version);
	CU_add_test(suite, "test_options_options_parse_missing_username",
		test_options_options_parse_missing_username);
	CU_add_test(suite, "test_options_options_parse_missing_wpurl",
		test_options_options_parse_missing_wpurl);
	CU_add_test(suite, "test_options_options_parse_bad_wpurl",
		test_options_options_parse_bad_wpurl);
	CU_add_test(suite, "test_options_options_parse_missing_unrecognized",
		test_options_options_parse_missing_unrecognized);
	CU_add_test(suite, "test_options_options_parse_short",
		test_options_options_parse_short);
	CU_add_test(suite, "test_options_options_parse_long",
		test_options_options_parse_long);
}
