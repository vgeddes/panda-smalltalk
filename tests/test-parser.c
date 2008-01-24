

#include <st-parser.h>
#include <st-lexer.h>
#include <st-ast.h>

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
    
    st_lexer_t *lexer = st_lexer_new (buffer);

    st_print_node ((st_node_t *) st_parser_parse (lexer));

    return 0;
}

