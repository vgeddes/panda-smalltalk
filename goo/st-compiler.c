
#include "st-compiler.h"
#include "st-symbol.h"
#include "st-universe.h"
#include "st-hashed-collection.h"
#include "st-class.h"
#include "st-input.h"
#include "st-node.h"

#include "st-lexer.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


typedef struct {
    
    const char *filename;
    STInput    *input;

    /* major, minor */
    int version[2];

    int line;

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
st_compile_string (st_oop klass, const char *string, STError **error)
{
    STNode *node;    
    st_oop method;
    STLexer *lexer;
    
    g_assert (klass != st_nil);
    
    lexer = st_lexer_new (string);
    
    node = st_parser_parse (lexer, error);

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
    fprintf (stderr, "%s: %i: %s\n", parser->filename, parser->line + st_token_line (token), message);
    exit (1);
}


static STToken *
next_token (FileInParser *parser, STLexer *lexer)
{
    STToken *token;

    token = st_lexer_next_token (lexer);
     
    if (st_token_type (token) == ST_TOKEN_COMMENT)
	return next_token (parser, lexer);
    else if (st_token_type (token) == ST_TOKEN_INVALID)
	filein_error (parser, token, st_lexer_error_message (lexer));
    
    return token;
}


static STLexer *
next_chunk (FileInParser *parser)
{
    char *chunk;

    parser->line = st_input_get_line (parser->input);

    chunk = st_input_next_chunk (parser->input);
    if (!chunk)
	return NULL;
    
    return st_lexer_new (chunk);
}

static void
parse_method (FileInParser *parser,
	      STLexer *lexer,
	      char *class_name,
	      bool class_method
    )
{
    STToken *token;
    st_oop   klass;
    STError *error = NULL;

    token = next_token (parser, lexer);
    if (st_token_type (token) != ST_TOKEN_STRING_CONST)
	filein_error (parser, token, "expected string literal")	;

    st_lexer_destroy (lexer);

    /* get class or metaclass */
    klass = st_global_get (class_name);
    if (klass == st_nil)
	filein_error (parser, token, "undefined class");

    if (class_method)
	klass = st_object_class (klass);
    

    /* parse method chunk */
    lexer = next_chunk (parser);
    if (!lexer)
	filein_error (parser, token, "expected method definition");	
    
    STNode *node;
    st_oop method;
    
    node = st_parser_parse (lexer, &error);
    if (node == NULL)
	goto error;
    
    method = st_generate_method (klass, node, &error);
    if (method == st_nil)
	goto error;
	
    st_dictionary_at_put (st_behavior_method_dictionary (klass),
			  node->selector,
			  method);

    st_node_destroy (node);
    st_lexer_destroy (lexer);
    g_free (class_name);

    return;
    
error:
    st_node_destroy (node);
    fprintf (stderr, "%s:%i: %s\n", parser->filename,
	     parser->line + ST_ERROR_LINE (error) - 1 ,
	     error->message);
    exit (1);   
}

static void
parse_class (FileInParser *parser, STLexer *lexer, char *name)
{


}

static void
parse_chunk (FileInParser *parser, STLexer *lexer)
{
    STToken *token;
    char *name;

    token = next_token (parser, lexer);

    if (st_token_type (token) == ST_TOKEN_IDENTIFIER) {

	name = g_strdup (st_token_text (token));

	token = next_token (parser, lexer);

	if (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR
	    && (streq (st_token_text (token), "methodFor:")))
	
	    parse_method (parser, lexer, name, false);
	
	else if (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR
		 || streq (st_token_text (token), "classMethodFor:"))

	    parse_method (parser, lexer, name, true);
	    
	else if (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR
		 && streq (st_token_text (token), "subclass:"))

	    parse_class (parser, lexer, name);

	else
	    goto error;

    } else
	goto error;

    return;
    
error:
    
    filein_error (parser, token, "unrecognised syntax");    
}

static void
parse_chunks (FileInParser *parser)
{
    STLexer *lexer;
    
    /* parse all chunks */
    while (st_input_look_ahead (parser->input, 1) != ST_INPUT_EOF) {
	
	lexer = next_chunk (parser);
	if (!lexer)
	    continue;

	parse_chunk (parser, lexer);
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

    parser->input    = st_input_new (contents);
    parser->filename = g_path_get_basename (filename);
    parser->line     = 1;

    parse_chunks (parser);

    g_slice_free (FileInParser, parser);

}

