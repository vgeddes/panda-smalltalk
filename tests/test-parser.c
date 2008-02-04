

#include <st-parser.h>
#include <st-lexer.h>
#include <st-ast.h>
#include <st-universe.h>

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
    while ((c = getchar ()) != EOF && i < (BUF_SIZE - 1))
	buffer[i++] = c;
    buffer[i] = '\0';

    /* the big bang */
    st_bootstrap_universe ();

    STLexer *lexer = st_lexer_new (buffer);

    STNode *node = st_parser_parse (lexer);

    printf ("-------------------\n");	    

    st_print_method (node);

    return 0;
}

