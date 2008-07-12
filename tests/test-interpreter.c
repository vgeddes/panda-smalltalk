
#include <st-types.h>
#include <st-symbol.h>
#include <st-compiler.h>
#include <st-cpu.h>
#include <st-array.h>
#include <st-lexer.h>
#include <st-node.h>
#include <st-universe.h>
#include <st-object.h>
#include <st-float.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define BUF_SIZE 10000

int
main (int argc, char *argv[])
{
    st_compiler_error error;
    char buffer[BUF_SIZE];
    char *string;
    st_oop value;
    char c;
    int i = 0;

    /* the big bang */
    st_bootstrap_universe ();

    while ((c = getchar ()) != EOF && i < (BUF_SIZE - 1))
	buffer[i++] = c;
    buffer[i] = '\0';

    string = st_strconcat ("doIt ^ [", buffer, "] value", NULL);

    bool result = st_compile_string (ST_UNDEFINED_OBJECT_CLASS, string, &error);
    if (!result) {
	fprintf (stderr, "test-processor:%i: %s\n",
		 error.line, error.message);
	return 1;
    }

    st_cpu_initialize ();

    struct timeval before, after;
    double elapsed;

    gettimeofday (&before, NULL);

    st_cpu_main ();

    gettimeofday (&after, NULL);

    elapsed = after.tv_sec - before.tv_sec +
	(after.tv_usec - before.tv_usec) / 1.e6;
    
    /* inspect the returned value on top of the stack */
    value = ST_STACK_PEEK ((&__cpu));

    printf ("result: %s\n", st_object_printString (value));
    printf ("time %.9f seconds\n", elapsed);


    return 0;
}
