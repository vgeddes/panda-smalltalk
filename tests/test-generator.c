
#include <st-compiler.h>
#include <st-lexer.h>
#include <st-node.h>
#include <st-universe.h>
#include <st-object.h>

#include <stdlib.h>
#include <glib.h>
#include <stdio.h>
#include <stdbool.h>


#define BUF_SIZE 10000

int
main (int argc, char *argv[])
{
    /* read input from stdin */
    char buffer[BUF_SIZE];
    char c;
    int i = 0;

    printf ("Enter a Smalltalk method (for the Association class):\n");

    while ((c = getchar ()) != EOF && i < (BUF_SIZE - 1))
	buffer[i++] = c;
    buffer[i] = '\0';

    /* the big bang ! */
    st_bootstrap_universe ();
    
    /* compile */
    STLexer *lexer = st_lexer_new (buffer, NULL);
    STError *error = NULL;
    STNode  *node;
    st_oop   method;

    node = st_parser_parse (lexer, &error);
    if (error) {
	fprintf (stderr, "%s:%i: %s\n", "test-generator",
		 ST_ERROR_LINE (error),
		 error->message);
	exit (1);
    }

    method = st_generate_method (st_object_class (st_association_class), node, &error);
    if (error) {
	fprintf (stderr, "%s:%i: %s\n", "test-generator",
		 ST_ERROR_LINE (error),
		 error->message);
	exit (1);
    }

    printf ("\nGenerated Method:\n\n"); 

    st_print_method (method);

    st_node_destroy (node);    

    return 0;
}

