AC_PREREQ([2.69])
AC_INIT([wp-backup], [0.1], [vlastavesely@protonmail.ch])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
PKG_CHECK_MODULES(CURL, libcurl)
PKG_CHECK_MODULES(LIBXML, libxml-2.0)

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strdup strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
