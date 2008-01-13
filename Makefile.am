
SUBDIRS = libs goo tests


EXTRA_DIST = \
	COPYING.GPL \
	COPYING.LGPL \
	autogen.sh

dist-hook:
	@echo Generating ChangeLog
	which bzr && bzr log --short > $(distdir)/ChangeLog
