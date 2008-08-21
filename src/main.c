
#include <st-types.h>
#include <st-symbol.h>
#include <st-compiler.h>
#include <st-machine.h>
#include <st-array.h>
#include <st-lexer.h>
#include <st-node.h>
#include <st-universe.h>
#include <st-object.h>
#include <st-float.h>
#include <optparse.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>

#define BUF_SIZE 2000

static const char version[] =
PACKAGE_STRING"\n"
"Copyright (C) 2007-2008 Vincent Geddes";

static bool verbose = false;

struct opt_spec options[] = {
    {opt_help, "h", "--help", NULL, "Show help information", NULL},
    {opt_version, "V", "--version", NULL, "Show version information" , (char *) version},
    {opt_store_1, "v", "--verbose", NULL, "Show verbose messages" , &verbose},
    {NULL}
};


static void
read_compile_stdin (void)
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

    if (!st_compile_string (ST_UNDEFINED_OBJECT_CLASS, string, &error)) {
	fprintf (stderr, "panda:%i: %s\n",
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

int
main (int argc, char *argv[])
{
    st_oop value;

    opt_basename (argv[0], '/');
    opt_message ("Read Smalltalk expressions from standard input and evaluate them.");
    opt_parse ("Usage: %s [options]", options, argv);
    
    st_set_verbose_mode (verbose);

    st_initialize ();

    read_compile_stdin ();

    st_machine_initialize (&__machine);
    st_machine_main (&__machine);
    
    /* inspect the returned value on top of the stack */
    value = ST_STACK_PEEK ((&__machine));
    if (st_object_format (value) != ST_FORMAT_BYTE_ARRAY)
	abort ();

    if (__machine.success) {
	putchar ('\n');
	printf ("result: %s\n", (char *) st_byte_array_bytes (value));
    }

    return 0;
}

