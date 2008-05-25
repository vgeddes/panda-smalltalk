
#include <st-types.h>
#include <st-symbol.h>
#include <st-compiler.h>
#include <st-interpreter.h>
#include <st-byte-array.h>
#include <st-lexer.h>
#include <st-node.h>
#include <st-universe.h>
#include <st-object.h>
#include <st-float.h>

#include <stdlib.h>
#include <glib.h>
#include <stdio.h>
#include <stdbool.h>

#define BUF_SIZE 10000

int
main (int argc, char *argv[])
{
    STError *error = NULL;
    char buffer[BUF_SIZE];
    char *string;
    st_oop value;
    STExecutionState es;
    char c;
    int i = 0;

    /* the big bang */
    st_bootstrap_universe ();

    while ((c = getchar ()) != EOF && i < (BUF_SIZE - 1))
	buffer[i++] = c;
    buffer[i] = '\0';

    string = g_strconcat ("doIt ^ [", buffer, "] value", NULL);

    st_compile_string (st_undefined_object_class, string, &error);
    if (error) {
	fprintf (stderr, "test-interpreter:%i: %s\n",
		 ST_ERROR_LINE (error), error->message);
	return 1;
    }

    st_interpreter_initialize (&es);
    st_interpreter_main (&es);

    /* inspect the returned value on top of the stack */
    value = ST_STACK_PEEK ((&es));

    printf ("result: %s\n", st_object_printString (value));

    return 0;
}
