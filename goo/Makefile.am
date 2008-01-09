
AM_CPPFLAGS = -DG_LOG_DOMAIN=\"Smalltalk\"

lib_LTLIBRARIES = libgoo.la

libgoo_la_SOURCES = \
	st-string-stream.h \
	st-string-stream.c \
	st-heap-object.c \
	st-heap-object.h \
 	st-lexer.c \
 	st-lexer.h \
 	st-utils.h \
 	st-utils.c \
  	st-hashed-collection.h \
    	st-hashed-collection.c \
  	st-array.h \
 	st-array.c \
  	st-byte-array.h \
 	st-byte-array.c \
  	st-small-integer.h \
   	st-small-integer.c \
    	st-large-integer.h \
   	st-large-integer.c \
  	st-compiled-code.h \
   	st-compiled-code.c \
	st-universe.h \
	st-universe.c \
	st-association.h \
	st-association.c \
	st-float.h \
	st-float.c \
	st-symbol.h \
	st-symbol.c \
	st-mark.h \
	st-class.h \
	st-class.c \
	st-object.h \
	st-object.c \
	st-mini.c \
	st-mini.h \
 	st-types.h
	
libgoo_la_CPPFLAGS =      \
	-I$(top_srcdir)   \
        -I$(top_srcdir)/libs/libtommath \
        $(AM_CPPFLAGS)

libgoo_la_CFLAGS =      \
	$(GLIB_CFLAGS)  \
	$(WARN_CFLAGS)

libgoo_la_LIBADD = $(GLIB_LIBS) $(top_builddir)/libs/libtommath/libtommath.la


bin_PROGRAMS = bootstrap

bootstrap_SOURCES = st-bootstrap.c

bootstrap_LDADD =    libgoo.la 
bootstrap_CFLAGS =   $(WARN_CFLAGS)
bootstrap_CPPFLAGS = $(GLIB_CFLAGS)

