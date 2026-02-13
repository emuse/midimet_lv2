#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

dnl AC_PREREQ([2.71])
AC_INIT([midimet],[0.0.1],[qmidiarp-devel@lists.sourceforge.net])
AC_CONFIG_SRCDIR([src/midimet.cpp])
AC_CONFIG_HEADERS(src/config.h)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([dist-bzip2])


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

AC_ENABLE_SHARED
AC_DISABLE_STATIC
LT_INIT([dlopen])


# Checks for minimal required libraries for all components
AC_CHECK_LIB([m], [roundf], , AC_MSG_ERROR([required libm missing]))

dnl check for lv2atom
AC_CHECK_HEADER( lv2/lv2plug.in/ns/ext/atom/atom.h, lv2include_found="yes", lv2include_found="no" )
PKG_CHECK_MODULES(LV2, lv2 >= 1.10, lv2Works="yes", lv2Works="no")
CFLAGSsave=$CFLAGS
CFLAGS=$LV2_CFLAGS
AC_SUBST(LV2_CFLAGS)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SSIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_STAT
AC_CHECK_FUNCS([floor ftruncate getcwd memset malloc mkdir pow rint sqrt])

AC_SUBST([HOME])
AM_CONDITIONAL([LIBDIR_IS_HOME], [test "x$libdir" = "x$HOME"])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([strdup])

AC_CONFIG_FILES([Makefile] [src/Makefile])
AC_OUTPUT
