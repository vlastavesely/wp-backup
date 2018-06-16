AC_PREREQ([2.69])
AC_INIT([wp-backup], [0.1], [vlastavesely@protonmail.ch])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT([/usr])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
PKG_CHECK_MODULES(CURL, libcurl)
PKG_CHECK_MODULES(LIBXML, libxml-2.0)

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h termios.h unistd.h fcntl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([clock_gettime memset munmap setenv strchr strdup strstr strtoul])
AC_FUNC_REALLOC

# Enable for gcov if option --enable-gcov is present.
AC_MSG_CHECKING([whether gcov should be enabled])
AC_ARG_ENABLE(gcov,
	AS_HELP_STRING([--enable-gcov], [enable gcov code coverage analysis]), [
	if test "x$enableval" != "xno" ; then
		if test "$GCC" != "yes"; then
			AC_MSG_ERROR([gcc is required for --enable-gcov])
		fi
		AC_MSG_RESULT(yes)
		COVERAGE_CFLAGS="-O0 -g -fno-inline -fprofile-arcs -ftest-coverage"
	else
		AC_MSG_RESULT(no)
		COVERAGE_CFLAGS=""
	fi
], [AC_MSG_RESULT(no)])
AC_SUBST(COVERAGE_CFLAGS)

AC_CONFIG_FILES([config.mak])
AC_OUTPUT
