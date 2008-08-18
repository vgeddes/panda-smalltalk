
#include <st-compiler.h>
#include <st-lexer.h>
#include <st-node.h>
#include <st-universe.h>
#include <st-object.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


#define BUF_SIZE 10000

int
main (int argc, char *argv[])
{
    st_compiler_error error;
    st_lexer *lexer;
    st_node  *node;
    st_oop    method;
    char      buffer[BUF_SIZE];
    char      c;
    int       i = 0;

    printf ("Enter a Smalltalk method (for the Association class):\n");

    while ((c = getchar ()) != EOF && i < (BUF_SIZE - 1))
	buffer[i++] = c;
    buffer[i] = '\0';

    st_initialize ();
    
    lexer = st_lexer_new (buffer);

    node = st_parser_parse (lexer, &error);
    if (!node) {
	fprintf (stderr, "%s:%i: %s\n", "test-generator",
		 error.line,
		 error.message);
	exit (1);
    }

    method = st_generate_method (st_object_class (ST_ASSOCIATION_CLASS), node, &error);
    if (method == ST_NIL) {
	fprintf (stderr, "%s:%i: %s\n", "test-generator",
		 error.line,
		 error.message);
	exit (1);
    }

    printf ("\nGenerated Method:\n\n"); 
    st_print_method (method);
    st_node_destroy (node);    

    return 0;
}

