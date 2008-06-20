
#include <st-types.h>
#include <st-symbol.h>
#include <st-compiler.h>
#include <st-processor.h>
#include <st-byte-array.h>
#include <st-lexer.h>
#include <st-node.h>
#include <st-universe.h>
#include <st-object.h>
#include <st-float.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>

#define BUF_SIZE 2000

static void
compile_input (void)
{
    st_compiler_error error;
    char buffer[BUF_SIZE];
    char *string;
    char c;
    int i = 0;

    while ((c = getchar ()) != EOF && i < (BUF_SIZE - 1))
	buffer[i++] = c;
    buffer[i] = '\0';

    string = st_strconcat ("doIt ^ [", buffer, "] value", NULL);

    if (!st_compile_string (st_undefined_object_class, string, &error)) {
	fprintf (stderr, "test-processor:%i: %s\n",
		 error.line, error.message);
	exit (1);
    }

    st_free (string);
}

static double
get_elapsed_time (struct timeval before, struct timeval after)
{
    return after.tv_sec - before.tv_sec + (after.tv_usec - before.tv_usec) / 1.e6;
}

#define ST_ROUND_UP_OOPS(m) (ST_BYTES_TO_OOPS ((m) + (ST_OOPS_TO_BYTES(1) - 1)))

int
main (int argc, char *argv[])
{
    st_processor processor;
    struct timeval before, after;
    st_oop value;

    st_bootstrap_universe ();
    compile_input ();

    st_processor_initialize (&processor);

    proc = &processor;

    gettimeofday (&before, NULL);
    st_processor_main (&processor);
    gettimeofday (&after, NULL);
    
    /* inspect the returned value on top of the stack */
    value = ST_STACK_PEEK ((&processor));

    printf ("\n");
    printf ("result: %s\n", st_object_printString (value));
    printf ("time:   %.3fs\n", get_elapsed_time (before, after));
    
    return 0;
}

