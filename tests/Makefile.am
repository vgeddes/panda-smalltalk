
noinst_PROGRAMS = test-string-stream test-lexer

test_string_stream_SOURCES = test-string-stream.c

test_string_stream_LDADD =    $(GLIB_LIBS) $(top_builddir)/goo/libgoo.la
test_string_stream_CFLAGS =   $(AM_CFLAGS) $(WARN_CFLAGS)
test_string_stream_CPPFLAGS = $(GLIB_CFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/goo

test_lexer_SOURCES = test-lexer.c

test_lexer_LDADD =    $(GLIB_LIBS) $(top_builddir)/goo/libgoo.la
test_lexer_CFLAGS =   $(AM_CFLAGS) $(WARN_CFLAGS)
test_lexer_CPPFLAGS = $(GLIB_CFLAGS) $(AM_CPPFLAGS) -I$(top_srcdir)/goo
