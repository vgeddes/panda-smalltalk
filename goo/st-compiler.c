
#include "st-compiler.h"
#include "st-symbol.h"
#include "st-universe.h"
#include "st-hashed-collection.h"
#include "st-class.h"
#include "st-input.h"
#include "st-node.h"
#include <string.h>

#include "st-lexer.h"
#include "stdlib.h"


typedef struct {
    
    const char *filename;
    STLexer    *lexer;

} FileInParser;

/*
 * st_compile_string:
 * @klass: The class for which the compiled method will be bound.
 * @string: Source code for the method
 * @error: return location for errors
 *
 * This function will compile a source string into a new CompiledMethod,
 * and place the method in the methodDictionary of the given class.
 */

bool
st_compile_string (st_oop klass, const char *string, GError **error)
{
    STNode *node;    
    st_oop method;
    STLexer *lexer;
    
    g_assert (klass != st_nil);
    
    lexer = st_lexer_new (string);
    
    node = st_parser_parse (lexer, false, error);

    st_lexer_destroy (lexer);

    if (!node)
	return false;

    method = st_generate_method (klass, node, error);
    if (method == st_nil) {
	st_node_destroy (node);
	return false;
    }

    st_dictionary_at_put (st_behavior_method_dictionary (klass),
			  node->selector,
			  method);

    st_node_destroy (node);

    return true;
}

static void
filein_error (FileInParser *parser, STToken *token, const char *message)
{
    fprintf (stderr, "%s: %i: %s\n", parser->filename, st_token_line (token), message);
    exit (1);
}


static STToken *
next_token (FileInParser *parser)
{
    STToken *token;

    token = st_lexer_next_token (parser->lexer);
    
    if (st_token_type (token) == ST_TOKEN_COMMENT)
	return next_token (parser);
    else if (st_token_type (token) == ST_TOKEN_INVALID)
	filein_error (parser, token, st_lexer_error_message (parser->lexer));
    
    return token;
}

/* parse a category of methods */
static void
parse_category (FileInParser *parser)
{
    st_oop klass;
    STToken *token;
    GError *error = NULL;

    token = st_lexer_current_token (parser->lexer);
    if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
	filein_error (parser, token, "expected identifier");
  
    klass = st_global_get (st_token_text (token));
    if (!st_object_is_class (klass))
	filein_error (parser, token, "name does refer to an existing class");
    
    token = next_token (parser);
    if (st_token_type (token) != ST_TOKEN_KEYWORD_SELECTOR
	|| !streq (st_token_text (token), "methodsFor:"))
	filein_error (parser, token, "expected 'methodsFor:' message");

    token = next_token (parser);
    if (st_token_type (token) != ST_TOKEN_STRING_CONST)
	filein_error (parser, token, "expected string constant");

    token = next_token (parser);
    if (st_token_type (token) != ST_TOKEN_BLOCK_BEGIN)
	filein_error (parser, token, "expected '['");

    token = next_token (parser);    

    while (st_token_type (token) != ST_TOKEN_EOF &&
	   st_token_type (token) != ST_TOKEN_BLOCK_END) {

	STNode *node;
	st_oop method;

	node = st_parser_parse (parser->lexer, true, &error);
	if (node == NULL)
	    goto error;

	method = st_generate_method (klass, node, &error);
	if (method == st_nil)
	    goto error;
	
	st_dictionary_at_put (st_behavior_method_dictionary (klass),
			      node->selector,
			      method);
	
	st_node_destroy (node);

	token = st_lexer_current_token (parser->lexer);

	continue;

error:
	st_node_destroy (node);
	fprintf (stderr, "%s:%s\n", parser->filename, error->message);
	exit (1);
    }

    if (st_token_type (token) != ST_TOKEN_BLOCK_END)
	filein_error (parser, token, "expected ']'");

}

static void
parse_filein (FileInParser *parser)
{
    STToken *token;

    token = next_token (parser);
    
    while (st_token_type (token) != ST_TOKEN_EOF) {
	    
	parse_category (parser);
	
	token = next_token (parser);
    }
}

void
st_file_in (const char *filename)
{
    char *contents;
    GError *error = NULL;
    FileInParser *parser;

    g_assert (filename != NULL);

    g_file_get_contents (filename,
			 &contents,
			 NULL,
			 &error);
    if (error) {
	g_warning ("could not file in '%s'", filename);
	g_error_free (error);
	return;
    }
    
    parser = g_slice_new0 (FileInParser);

    parser->lexer    = st_lexer_new (contents);
    parser->filename = g_path_get_basename (filename);

    parse_filein (parser);

    g_slice_free (FileInParser, parser);

}
