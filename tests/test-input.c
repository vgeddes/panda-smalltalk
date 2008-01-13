
#include <st-input.h>
#include <glib.h>

#include <stdio.h>

static const char test_string_1[] = "abcdef\nghijklmno\npqrstuvwx\nz";
static const char test_string_2[] = "\nabcdef\n\nghijklmno\npqrstuvwxz\n";
static const char test_string_3[] = "\342\272\206\342\273\204\351\252\260\n\351\253\200";
static const char test_string_4[] = "\n";


static void
test_lookahead_consume (void)
{
    st_input_t *stream;

    char utf8char[6] = { 0, };

    gunichar uchar;
    int i = 0;

    printf ("\nTEST - lookahead/consume\n");

    // on test_data_1

    stream = st_input_new (test_string_1);

    printf ("\n\"%s\"\n", g_strescape (test_string_1, NULL));

    while ((uchar = st_input_look_ahead (stream, 1)) != ST_INPUT_EOF) {

	g_unichar_to_utf8 (uchar, utf8char);

	if (uchar != 0x000a)
	    printf ("%s  %i,%i\n", utf8char,
		    st_input_get_line (stream), st_input_get_column (stream));

	else
	    printf ("%s %i,%i\n", "\\n",
		    st_input_get_line (stream), st_input_get_column (stream));


	st_input_consume (stream);
    }

    // on test_data_2
    st_input_destroy (stream);
    stream = st_input_new (test_string_2);


    printf ("\n\"%s\"\n", g_strescape (test_string_2, NULL));

    while ((uchar = st_input_look_ahead (stream, 1)) != ST_INPUT_EOF) {

	g_unichar_to_utf8 (uchar, utf8char);

	if (uchar != 0x000a)
	    printf ("%s  %i,%i\n", utf8char,
		    st_input_get_line (stream), st_input_get_column (stream));

	else
	    printf ("%s %i,%i\n", "\\n",
		    st_input_get_line (stream), st_input_get_column (stream));

	st_input_consume (stream);
    }


    // on test_data_3
    st_input_destroy (stream);
    stream = st_input_new (test_string_3);


    // lookahead up to 4 chars this time, without consuming - line,col numbers not calculated though
    printf ("\n\"%s\"\n", g_strescape (test_string_3, NULL));

    i = 1;

    while ((uchar = st_input_look_ahead (stream, i)) != ST_INPUT_EOF) {

	g_unichar_to_utf8 (uchar, utf8char);

	if (uchar != 0x000a)
	    printf ("%s\n", utf8char);
	else
	    printf ("%s\n", "\\n");

	i++;
    }


    // on test_data_4
    st_input_destroy (stream);
    stream = st_input_new (test_string_4);


    printf ("\n\"%s\"\n", g_strescape (test_string_4, NULL));

    while ((uchar = st_input_look_ahead (stream, 1)) != ST_INPUT_EOF) {

	g_unichar_to_utf8 (uchar, utf8char);

	if (uchar != 0x000a)
	    printf ("%s  %i,%i\n", utf8char,
		    st_input_get_line (stream), st_input_get_column (stream));

	else
	    printf ("%s %i,%i\n", "\\n",
		    st_input_get_line (stream), st_input_get_column (stream));

	st_input_consume (stream);
    }



}

static void
test_mark_rewind (void)
{
    char buf[6] = { 0, };

    st_input_t *stream = st_input_new (test_string_1);

    printf ("\nTEST - mark/rewind\n");

    gunichar uchar;
    while ((uchar = st_input_look_ahead (stream, 1)) != ST_INPUT_EOF) {

	st_input_consume (stream);

	// add markers at each newline 
	if (uchar == 0x000a)
	    st_input_mark (stream);

    }

    st_input_rewind_to_marker (stream, 1);

    uchar = st_input_look_ahead (stream, 1);
    g_unichar_to_utf8 (uchar, buf);
    printf ("%s  %i,%i\n", buf,
	    st_input_get_line (stream), st_input_get_column (stream));

    st_input_rewind_to_marker (stream, 2);

    uchar = st_input_look_ahead (stream, 1);
    g_unichar_to_utf8 (uchar, buf);
    printf ("%s  %i,%i\n", buf,
	    st_input_get_line (stream), st_input_get_column (stream));

}


static void
test_seek (void)
{
    char buf[6] = { 0, };
    gunichar uchar;

    st_input_t *stream = st_input_new (test_string_2);

    printf ("\nTEST - seek\n");

    st_input_seek (stream, 6);

    uchar = st_input_look_ahead (stream, 1);
    g_unichar_to_utf8 (uchar, buf);
    printf ("%s  %i,%i\n", buf,
	    st_input_get_line (stream), st_input_get_column (stream));


    st_input_seek (stream, 3);

    uchar = st_input_look_ahead (stream, 1);
    g_unichar_to_utf8 (uchar, buf);
    printf ("%s  %i,%i\n", buf,
	    st_input_get_line (stream), st_input_get_column (stream));

    st_input_seek (stream, 9);

    uchar = st_input_look_ahead (stream, 1);
    g_unichar_to_utf8 (uchar, buf);
    printf ("%s  %i,%i\n", buf,
	    st_input_get_line (stream), st_input_get_column (stream));

}

static void
test_substring (void)
{



    st_input_t *stream = st_input_new (test_string_1);

    printf ("\nTEST - seek\n");

    gunichar *uchars = st_input_substring (stream, 3, 27);

    char *str = g_ucs4_to_utf8 (uchars, -1, NULL, NULL, NULL);

    printf ("%s\n", str);
}

int
main (int argc, char *argv[])
{

    test_lookahead_consume ();

    test_mark_rewind ();

    test_seek ();

    test_substring ();

    return 0;
}
