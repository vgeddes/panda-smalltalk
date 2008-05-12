
#include <st-types.h>
#include <st-symbol.h>
#include <st-compiler.h>
#include <st-interpreter.h>
#include <st-byte-array.h>
#include <st-lexer.h>
#include <st-node.h>
#include <st-universe.h>
#include <st-object.h>

#include <stdlib.h>
#include <glib.h>
#include <stdio.h>
#include <stdbool.h>

#define BUF_SIZE 10000

static void
describe_oop (st_oop value)
{
    char *class_name;

    class_name = (char *) st_byte_array_bytes (st_class_name (st_object_class (value)));

    if (st_object_is_smi (value))
	printf ("returned: %s [value = %i]\n", class_name, (int) st_smi_value (value));
    else if (st_object_is_byte_array (value))
	printf ("returned: %s [value = '%s']\n", class_name, (char *) st_byte_array_bytes (value));
    else
	printf ("returned: %s\n", class_name);	
}

int
main (int argc, char *argv[])
{
    STError *error = NULL;
    char buffer[BUF_SIZE];
    char *string;
    st_oop context;
    st_oop value;
    STExecutionState es;
    char c;
    int i = 0;

    while ((c = getchar ()) != EOF && i < (BUF_SIZE - 1))
	buffer[i++] = c;
    buffer[i] = '\0';

    string = g_strconcat ("doIt ^ [", buffer, "] value", NULL);

    /* the big bang */
    st_bootstrap_universe ();

    st_compile_string (st_undefined_object_class, string, &error);
    if (error) {
	fprintf (stderr, "test-interpreter:%i: %s\n",
		 ST_ERROR_LINE (error), error->message);
	return 1;
    }

    /* initialize context and enter interpreter */
    context = st_send_unary_message (st_nil,
			  	     st_nil,
				     st_symbol_new ("doIt"));

    st_interpreter_initialize_state (&es);
    st_interpreter_set_active_context (&es, context);
    st_interpreter_enter (&es);

    /* inspect the returned value on top of the stack */
    value = ST_STACK_PEEK ((&es));

    describe_oop (value);

    return 0;
}
