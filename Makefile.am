
ACLOCAL_AMFLAGS = -I .

lib_LTLIBRARIES =
noinst_LTLIBRARIES =
bin_PROGRAMS = 
noinst_PROGRAMS =
DISTCLEANFILES =
CLEANFILES = 
BUILT_SOURCES = 

EXTRA_DIST = \
	COPYING    \
	st-mode.el \
	autogen.sh \
	docs/bytecodes.txt \
	docs/object-memory.txt

include Makefile-libtommath.am
include Makefile-libpanda.am
include Makefile-libgdtoa.am
include Makefile-libmpa.am
include Makefile-optparse.am
include Makefile-tests.am

dist-hook:
	@echo Generating ChangeLog
	which bzr && bzr log --short > $(distdir)/ChangeLog

