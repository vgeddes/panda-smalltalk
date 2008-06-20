
#include "st-compiler.h"
#include "st-symbol.h"
#include "st-universe.h"
#include "st-hashed-collection.h"
#include "st-behavior.h"
#include "st-input.h"
#include "st-node.h"

#include "st-lexer.h"
#include "st-array.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>


typedef struct {
    
    const char *filename;
    st_input    *input;

    int line;

} FileInParser;

/*
 * st_compile_string:
 * @class: The class for which the compiled method will be bound.
 * @string: Source code for the method
 * @error: return location for errors
 *
 * This function will compile a source string into a new CompiledMethod,
 * and place the method in the methodDictionary of the given class.
 */
bool
st_compile_string (st_oop class, const char *string, st_compiler_error *error)
{
    st_node  *node;    
    st_oop   method;
    st_lexer *lexer;
    
    st_assert (class != st_nil);
    
    lexer = st_lexer_new (string);
    if (!lexer)
	return false;
    
    node = st_parser_parse (lexer, error);
    st_lexer_destroy (lexer);

    if (!node)
	return false;

    method = st_generate_method (class, node, error);
    if (method == st_nil) {
	st_node_destroy (node);
	return false;
    }

    st_dictionary_at_put (ST_BEHAVIOR (class)->method_dictionary,
			  node->method.selector,
			  method);

    st_node_destroy (node);

    return true;
}

static void
filein_error (FileInParser *parser, st_token *token, const char *message)
{
    fprintf (stderr, "%s: %i: %s\n", parser->filename, parser->line + ((token) ? st_token_get_line (token) : -90), message);
    exit (1);
}


static st_token *
next_token (FileInParser *parser, st_lexer *lexer)
{
    st_token *token;

    token = st_lexer_next_token (lexer);
     
    if (st_token_get_type (token) == ST_TOKEN_COMMENT)
	return next_token (parser, lexer);
    else if (st_token_get_type (token) == ST_TOKEN_INVALID)
	filein_error (parser, token, st_lexer_error_message (lexer));

    return token;
}


static st_lexer *
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
	      st_lexer      *lexer,
	      char         *class_name,
	      bool          class_method)
{
    st_token *token = NULL;
    st_oop   class;
    st_compiler_error error;

    st_lexer_destroy (lexer);

    /* get class or metaclass */
    class = st_global_get (class_name);
    if (class == st_nil)
	filein_error (parser, token, "undefined class");

    if (class_method)
	class = st_object_class (class);

    /* parse method chunk */
    lexer = next_chunk (parser);
    if (!lexer)
	filein_error (parser, token, "expected method definition");	
    
    st_node *node;
    st_oop method;
    
    node = st_parser_parse (lexer, &error);
    if (node == NULL)
	goto error;
    if (node->type != ST_METHOD_NODE)
	printf ("%i\n", node->type);
    
    method = st_generate_method (class, node, &error);
    if (method == st_nil)
	goto error;
	
    st_dictionary_at_put (ST_BEHAVIOR (class)->method_dictionary,
			  node->method.selector,
			  method);

 
    st_node_destroy (node);
    st_lexer_destroy (lexer);
    st_free (class_name);

    return;
    
error:
    st_node_destroy (node);
    fprintf (stderr, "%s:%i: %s\n", parser->filename,
	     parser->line + error.line - 1 ,
	     error.message);
    exit (1);   
}

static void
parse_class (FileInParser *parser, st_lexer *lexer, char *name)
{


}

static void
parse_chunk (FileInParser *parser, st_lexer *lexer)
{
    st_token *token;
    char *name;

    token = next_token (parser, lexer);

    if (st_token_get_type (token) == ST_TOKEN_IDENTIFIER) {

	name = st_strdup (st_token_get_text (token));

	token = next_token (parser, lexer);

	if (st_token_get_type (token) == ST_TOKEN_IDENTIFIER
	    && (streq (st_token_get_text (token), "method")))
	
	    parse_method (parser, lexer, name, false);
	
	else if (st_token_get_type (token) == ST_TOKEN_IDENTIFIER
		 || streq (st_token_get_text (token), "classMethod"))

	    parse_method (parser, lexer, name, true);
	    
	else if (st_token_get_type (token) == ST_TOKEN_KEYWORD_SELECTOR
		 && streq (st_token_get_text (token), "subclass:"))

	    parse_class (parser, lexer, name);

	else if (streq (name, "Annotation") &&
		 st_token_get_type (token) == ST_TOKEN_KEYWORD_SELECTOR &&
		 streq (st_token_get_text (token), "key:")) {

	    return;
		 
	}
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
    st_lexer *lexer;
    
    while (st_input_look_ahead (parser->input, 1) != ST_INPUT_EOF) {
	
	lexer = next_chunk (parser);
	if (!lexer)
	    continue;

	parse_chunk (parser, lexer);
    }
}

void
st_compile_file_in (const char *filename)
{
    char *buffer;
    FileInParser *parser;

    st_assert (filename != NULL);

    if (!st_file_get_contents (filename, &buffer)) {
	return;
    }
    
    parser = st_new0 (FileInParser);

    parser->input = st_input_new (buffer);
    if (!parser->input) {
	fprintf (stderr, "could not validate input file '%s'", filename);
	return;
    }

    parser->filename = basename (filename);
    parser->line     = 1;

    parse_chunks (parser);

    st_free (parser);
}
