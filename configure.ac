
AC_PREREQ([2.52])

m4_define(panda_major_version, 0)
m4_define(panda_minor_version, 0)
m4_define(panda_micro_version, 54)
m4_define(panda_version, panda_major_version.panda_minor_version.panda_micro_version)

AC_INIT([Panda], [panda_version],
	[https://bugs.launchpad.net/panda-smalltalk/+filebug],
	[panda])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/st-lexer.c])

AM_INIT_AUTOMAKE([1.10 foreign -Wall subdir-objects])

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O
if test "x$ac_cv_prog_cc_c99" = "xno"; then
   AC_MSG_ERROR([cannot find a C99-conforming compiler])
fi

AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_LIBTOOL

# platform checks
AC_CHECK_SIZEOF([void *])
AC_CHECK_SIZEOF([int])
AC_CHECK_ALIGNOF([void *])
debug_default=yes
AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug=@<:@no/yes@:>@],
                             [turn on debugging support @<:@default=yes@:>@]),,
              enable_debug=$debug_default)
if test "x$enable_debug" = "xyes"; then
AC_DEFINE([ST_DEBUG], [1], ["Debug support enabled"])
fi

# recommended gcc optimization flags for libtommath
if test "x$GCC" = "xyes"; then
    TOMMATH_CFLAGS="-O3 -funroll-loops -fomit-frame-pointer"
    GDTOA_CFLAGS="-O2 -fPIC -funroll-loops -finline-functions"
fi
AC_SUBST(TOMMATH_CFLAGS)
AC_SUBST(GDTOA_CFLAGS)

AC_DEFINE(PANDA_MAJOR_VERSION, panda_major_version, [Panda major version])
AC_SUBST(PANDA_MAJOR_VERSION, panda_major_version)
AC_DEFINE(PANDA_MINOR_VERSION, panda_minor_version, [Panda minor version])
AC_SUBST(PANDA_MINOR_VERSION, panda_minor_version)
AC_DEFINE(PANDA_MICRO_VERSION, panda_micro_version, [Panda micro version])
AC_SUBST(PANDA_MICRO_VERSION, panda_micro_version)

AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT

