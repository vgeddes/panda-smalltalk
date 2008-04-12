
AC_PREREQ([2.52])
AC_INIT([goo], [1.0])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([goo/st-lexer.c])

AM_INIT_AUTOMAKE([1.10 foreign subdir-objects])

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CC_C99
if test "x$ac_cv_prog_cc_c99" = "xno"; then
   AC_MSG_ERROR([cannot find a C99-conforming compiler])
fi

AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_LIBTOOL

# platform checks
AC_CHECK_SIZEOF([void *])
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long int])
AC_CHECK_ALIGNOF([void *])

GNOME_COMMON_INIT
GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])

# recommended gcc optimization flags for libtommath
if test "x$GCC" = "xyes"; then
    TOMMATH_CFLAGS="-O3 -funroll-loops -fomit-frame-pointer"
fi
AC_SUBST(TOMMATH_CFLAGS)

PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.10.0])
AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)

GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of the gettext package.])
AM_GLIB_GNU_GETTEXT

AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

AC_CONFIG_FILES([
Makefile
po/Makefile.in
])

AC_OUTPUT

