
#                                               -*- Autoconf -*-
# configure.ac -- Autoconf script for alsa-intf
#

# Process this file with autoconf to produce a configure script

# Requires autoconf tool later than 2.61
AC_PREREQ(2.61)
# Initialize the tcm package version
AC_INIT([tcm],[git_full_version], [otto.linnemann@peiker.de])
# Does not strictly follow GNU Coding standards
AM_INIT_AUTOMAKE([foreign])
# Disables auto rebuilding of configure, Makefile.ins
AM_MAINTAINER_MODE
# Verifies the --srcdir is correct by checking for the path
AC_CONFIG_SRCDIR([src/tcm_server.c])
# defines some macros variable to be included by source
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# makro for fetching full git tag number version() function
AX_GIT_REVISION

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(libolcutils, libolcutils >= 1.0.0 )
PKG_CHECK_MODULES(libintercom, libintercom >= 0.1.1 )
PKG_CHECK_MODULES(tinyscheme, tinyscheme >= 1.41 )
PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.16, dummy=yes, AC_MSG_ERROR(GLib >= 2.16 is required))
PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.16, dummy=yes, AC_MSG_ERROR(GThread >= 2.16 is required))

GLIB_CFLAGS="$GLIB_CFLAGS $GTHREAD_CFLAGS"
GLIB_LIBS="$GLIB_LIBS $GTHREAD_LIBS"

AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

AC_CHECK_LIB([c], [fmemopen],
[
    AC_DEFINE_UNQUOTED([HAVE_FMEMOPEN], 1, [Has fmemopen support.])
]
)

AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
    then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi

AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([doc/doxygen/doxygen.dox doc/doxygen/doxyfile doc/doxygen/Makefile])])

AC_SUBST([CFLAGS])
AC_SUBST([CC])
AC_CONFIG_FILES([ Makefile src/Makefile])
AC_OUTPUT
