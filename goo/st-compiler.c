
#include "st-compiler.h"
#include "st-symbol.h"
#include "st-universe.h"
#include "st-hashed-collection.h"
#include "st-class.h"
#include "st-input.h"


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

/* currently limited to "evaluating" simple methodFor statements */

static st_oop
st_evaluate_string (const char *string)
{
    STNode *node;
    GError *error = NULL;
    
    if (!string)
	return st_nil;

    node = st_parse_expression (st_lexer_new (string), &error);
    if (error) {
	fprintf (stderr, "error: %s\n", error->message);
	g_error_free (error);
	return st_nil;
    }

    if (node->type != ST_MESSAGE_NODE
	|| node->selector != st_symbol_new ("methodsFor:")
	|| node->receiver->type != ST_VARIABLE_NODE) {
	fprintf (stderr, "error: only methodFor: statements can be evaluated\n");
	return st_nil;
    }

    /* ignore categories for the moment
     * 
     * return the class the #methodFor was sent to
     */
    return st_dictionary_at (st_smalltalk, node->receiver->name);
}

static void
st_file_in_for_class (st_oop klass, STInput *input)
{
    char   *chunk;
    GError *error = NULL;

    g_assert (st_object_is_class (klass));

    while (st_input_look_ahead (input, 1) != ST_INPUT_EOF) {
	
	chunk = st_input_next_chunk (input);
	if (!chunk || !strcmp (chunk, " "))
	    break;

	//	printf ("Chunk:\n%s\n", chunk);

	st_compile_string (klass, chunk, &error);
	if (error) {
	    fprintf (stderr, "error: %s\n", error->message);
	    g_error_free (error);
	    g_free (chunk);
	    break;
	}
	g_free (chunk);
    }
}

void
st_file_in (const char *filename)
{
    STInput *input;
    char *contents;
    GError *error = NULL;

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
    
    input = st_input_new (contents);

    while (st_input_look_ahead (input, 1) != ST_INPUT_EOF) {

	bool saw_bang;
	char *chunk;
	st_oop klass;

	saw_bang = st_input_look_ahead (input, 1) == '!';
	if (saw_bang)
	    st_input_consume (input);

	chunk = st_input_next_chunk (input);

	klass = st_evaluate_string (chunk);

	if (saw_bang) {
	    st_file_in_for_class (klass, input);
	    g_free (chunk);
	}
    }
   
}
