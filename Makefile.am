
SUBDIRS = po

ACLOCAL_AMFLAGS = -I .

lib_LTLIBRARIES =
noinst_LTLIBRARIES =
bin_PROGRAMS = 
noinst_PROGRAMS =
DISTCLEANFILES =

EXTRA_DIST = \
	COPYING    \
	st-mode.el \
	autogen.sh

include Makefile-libtommath.am
include Makefile-libgoo.am
include Makefile-tests.am

dist-hook:
	@echo Generating ChangeLog
	which bzr && bzr log --short > $(distdir)/ChangeLog

