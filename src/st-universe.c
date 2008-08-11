/*
 * st-universe.c
 *
 * Copyright (C) 2008 Vincent Geddes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

#include "st-types.h"
#include "st-utils.h"
#include "st-object.h"
#include "st-behavior.h"
#include "st-float.h"
#include "st-association.h"
#include "st-method.h"
#include "st-array.h"
#include "st-small-integer.h"
#include "st-dictionary.h"
#include "st-large-integer.h"
#include "st-symbol.h"
#include "st-universe.h"
#include "st-object.h"
#include "st-lexer.h"
#include "st-compiler.h"
#include "st-memory.h"
#include "st-context.h"
#include "st-cpu.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

st_oop
st_global_get (const char *name)
{
    st_oop sym;

    sym = st_symbol_new (name);

    return st_dictionary_at (ST_GLOBALS, sym);
}

enum
{
    INSTANCE_SIZE_UNDEFINED = 0,
    INSTANCE_SIZE_CLASS = 6,
    INSTANCE_SIZE_METACLASS = 6,
    INSTANCE_SIZE_DICTIONARY = 3,
    INSTANCE_SIZE_SET = 3,
    INSTANCE_SIZE_ASSOCIATION = 2,
    INSTANCE_SIZE_SYSTEM = 2,
};

static st_oop
class_new (st_format format, st_uint instance_size)
{
    st_oop class;

    class = st_memory_allocate (ST_SIZE_OOPS (struct st_class));

    ST_OBJECT_MARK (class)  = 0 | ST_MARK_TAG;
    ST_OBJECT_CLASS (class) = ST_NIL;
    st_object_set_format (class, ST_FORMAT_OBJECT);
    st_object_set_instance_size (class, INSTANCE_SIZE_CLASS);
    
    ST_BEHAVIOR_FORMAT (class)             = st_smi_new (format);
    ST_BEHAVIOR_INSTANCE_SIZE (class)      = st_smi_new (instance_size);
    ST_BEHAVIOR_SUPERCLASS (class)         = ST_NIL;
    ST_BEHAVIOR_METHOD_DICTIONARY (class)  = ST_NIL;
    ST_BEHAVIOR_INSTANCE_VARIABLES (class) = ST_NIL;

    ST_CLASS (class)->name = ST_NIL;

    return class;
}

static void
add_global (const char *name, st_oop object)
{
    st_oop symbol;

    // sanity check for symbol interning
    st_assert (st_symbol_new (name) == st_symbol_new (name));

    symbol = st_symbol_new (name);
    st_dictionary_at_put (ST_GLOBALS, symbol, object);

    // sanity check for dictionary
    st_assert (st_dictionary_at (ST_GLOBALS, symbol) == object);
}

static void
parse_error (char *message, st_token *token)
{
    fprintf (stderr, "error: %i: %i: %s",
	     st_token_get_line (token), st_token_get_column (token), message);
    exit (1);
}

static void
initialize_class  (const char *name,
		   const char *super_name,
		   st_list    *ivarnames)
{
    st_oop metaclass, class, superclass;
    st_oop names;
    st_uint i = 1;

    if (streq (name, "Object") && streq (super_name, "nil")) {

	class = st_dictionary_at (ST_GLOBALS, st_symbol_new ("Object"));
	st_assert (class != ST_NIL);

	metaclass = st_object_class (class);
	if (metaclass == ST_NIL) {
	    metaclass = st_object_new (ST_METACLASS_CLASS);
	    ST_OBJECT_CLASS (class) = metaclass;
	}

	ST_BEHAVIOR_SUPERCLASS (class)     = ST_NIL;
	ST_BEHAVIOR_INSTANCE_SIZE (class)  = st_smi_new (0);
	ST_BEHAVIOR_SUPERCLASS (metaclass) = st_dictionary_at (ST_GLOBALS, st_symbol_new ("Class"));

    } else {
       	superclass = st_global_get (super_name);
	if (superclass == ST_NIL)
	    st_assert (superclass != ST_NIL);

	class = st_global_get (name);
	if (class == ST_NIL)
	    class = class_new (st_smi_value (ST_BEHAVIOR_FORMAT (superclass)), 0);

	metaclass = ST_HEADER (class)->class;
	if (metaclass == ST_NIL) {
	    metaclass = st_object_new (ST_METACLASS_CLASS);
	    ST_OBJECT_CLASS (class) = metaclass;
	}

	ST_BEHAVIOR_SUPERCLASS (class)     = superclass;
	ST_BEHAVIOR_SUPERCLASS (metaclass) = ST_HEADER (superclass)->class;
	ST_BEHAVIOR_INSTANCE_SIZE (class) = st_smi_new (st_list_length (ivarnames) +
							st_smi_value (ST_BEHAVIOR_INSTANCE_SIZE (superclass)));	
    }

    names = ST_NIL;
    if (st_list_length (ivarnames) != 0) {
	names = st_object_new_arrayed (ST_ARRAY_CLASS, st_list_length (ivarnames));
	for (st_list *l = ivarnames; l; l = l->next)
	    st_array_at_put (names, i++, st_symbol_new (l->data));
	ST_BEHAVIOR_INSTANCE_VARIABLES (class) = names;
    }

    ST_BEHAVIOR_FORMAT (metaclass)             = st_smi_new (ST_FORMAT_OBJECT);
    ST_BEHAVIOR_METHOD_DICTIONARY (metaclass)  = st_dictionary_new ();
    ST_BEHAVIOR_INSTANCE_VARIABLES (metaclass) = ST_NIL;
    ST_BEHAVIOR_INSTANCE_SIZE (metaclass)      = st_smi_new (INSTANCE_SIZE_CLASS);
    ST_METACLASS_INSTANCE_CLASS (metaclass)    = class;
    ST_BEHAVIOR_INSTANCE_VARIABLES (class)     = names;
    ST_BEHAVIOR_METHOD_DICTIONARY (class)      = st_dictionary_new ();
    ST_CLASS_NAME (class)                      = st_symbol_new (name);

    st_dictionary_at_put (ST_GLOBALS, st_symbol_new (name), class);
}


static bool
parse_variable_names (st_lexer *lexer, st_list **varnames)
{
    st_lexer *ivarlexer;
    st_token *token;
    char *names;

    token = st_lexer_next_token (lexer);

    if (st_token_get_type (token) != ST_TOKEN_STRING_CONST)
	return false;

    names = st_strdup (st_token_get_text (token));
    ivarlexer = st_lexer_new (names);
    token = st_lexer_next_token (ivarlexer);
    
    while (st_token_get_type (token) != ST_TOKEN_EOF) {
	
	if (st_token_get_type (token) != ST_TOKEN_IDENTIFIER)
	    parse_error (NULL, token);

	*varnames = st_list_append (*varnames, st_strdup (st_token_get_text (token)));
	token = st_lexer_next_token (ivarlexer);
    }

    st_free (names);
    st_lexer_destroy (ivarlexer);

    return true;
}


static void
parse_class (st_lexer *lexer, st_token *token)
{
    char *class_name = NULL;
    char *superclass_name = NULL;
    st_list *ivarnames = NULL;

    // 'Class' token
    if (st_token_get_type (token) != ST_TOKEN_IDENTIFIER
	|| !streq (st_token_get_text (token), "Class"))
	parse_error ("expected class definition", token);	

    // `named:' token
    token = st_lexer_next_token (lexer);
    if (st_token_get_type (token) != ST_TOKEN_KEYWORD_SELECTOR
	|| !streq (st_token_get_text (token), "named:"))
	parse_error ("expected 'name:'", token);

    // class name
    token = st_lexer_next_token (lexer);
    if (st_token_get_type (token) == ST_TOKEN_STRING_CONST) {
	class_name = st_strdup (st_token_get_text (token));
    } else {
	parse_error ("expected string literal", token);
    }

    // `superclass:' token
    token = st_lexer_next_token (lexer);	
    if (st_token_get_type (token) != ST_TOKEN_KEYWORD_SELECTOR
	|| !streq (st_token_get_text (token), "superclass:"))
	parse_error ("expected 'superclass:'", token);

    // superclass name
    token = st_lexer_next_token (lexer);
    if (st_token_get_type (token) == ST_TOKEN_STRING_CONST) {
        
	superclass_name = st_strdup (st_token_get_text (token));

    } else {
	parse_error ("expected string literal", token);
    }

    // 'instanceVariableNames:' keyword selector        
    token = st_lexer_next_token (lexer);
    if (st_token_get_type (token) == ST_TOKEN_KEYWORD_SELECTOR &&
	streq (st_token_get_text (token), "instanceVariableNames:")) {

	parse_variable_names (lexer, &ivarnames);
    } else {
	parse_error (NULL, token);
    }

    token = st_lexer_next_token (lexer);
    initialize_class (class_name, superclass_name, ivarnames);

    st_list_foreach (ivarnames, st_free);
    st_list_destroy (ivarnames);
    st_free (class_name);
    st_free (superclass_name);
    
    return;
}

static void
parse_classes (const char *filename)
{
    char *contents;
    st_lexer *lexer;
    st_token *token;

    if (!st_file_get_contents (filename, &contents)) {
	exit (1);
    }

    lexer = st_lexer_new (contents);
    st_assert (lexer != NULL);
    token = st_lexer_next_token (lexer);

    while (st_token_get_type (token) != ST_TOKEN_EOF) {

	while (st_token_get_type (token) == ST_TOKEN_COMMENT)
	    token = st_lexer_next_token (lexer);

	parse_class (lexer, token);
	token = st_lexer_next_token (lexer);
    }

    st_free (contents);
    st_lexer_destroy (lexer);
}

static void
file_in_classes (void)
{
    char *filename;

    parse_classes ("../st/class-defs.st");

    static const char * files[] = 
	{
	    "Stream.st",
	    "PositionableStream.st",
	    "WriteStream.st",
	    "Collection.st",
	    "SequenceableCollection.st",
	    "ArrayedCollection.st",
	    "HashedCollection.st",
	    "Set.st",
	    "Dictionary.st",
	    "IdentitySet.st",
	    "IdentityDictionary.st",
	    "Bag.st",
	    "Array.st",
	    "ByteArray.st",
	    "WordArray.st",
	    "FloatArray.st",
	    "Association.st",
	    "Magnitude.st",
	    "Number.st",
	    "Integer.st",
	    "SmallInteger.st",
	    "LargeInteger.st",
	    "Fraction.st",
	    "Float.st",
	    "Object.st",
	    "UndefinedObject.st",
	    "String.st",
	    "Symbol.st",
	    "ByteString.st",
	    "WideString.st",
	    "Character.st",
	    "UnicodeTables.st",
	    "Behavior.st",
	    "Boolean.st",
	    "True.st",
	    "False.st",
	    "Behavior.st",
	    "ContextPart.st",
	    "BlockContext.st",
	    "Message.st",
	    "OrderedCollection.st",
	    "List.st",
	    "System.st",
	    "CompiledMethod.st",
	    "FileStream.st",
	    "pidigits.st"
	};

    for (st_uint i = 0; i < ST_N_ELEMENTS (files); i++) {
	filename = st_strconcat ("..", ST_DIR_SEPARATOR_S, "st", ST_DIR_SEPARATOR_S, files[i], NULL);
	st_compile_file_in (filename);
	st_free (filename);
    }
}

#define NIL_SIZE_OOPS (sizeof (struct st_header) / sizeof (st_oop))

static st_oop
create_nil_object (void)
{
    st_oop nil;

    nil = st_memory_allocate (NIL_SIZE_OOPS);

    ST_OBJECT_MARK (nil)  = 0 | ST_MARK_TAG;
    ST_OBJECT_CLASS (nil) = nil;
    st_object_set_format (nil, ST_FORMAT_OBJECT);
    st_object_set_instance_size (nil, 0);

    return nil;
}

static void
init_specials (void)
{
    ST_SELECTOR_PLUS      = st_symbol_new ("+");
    ST_SELECTOR_MINUS     = st_symbol_new ("-");
    ST_SELECTOR_LT        = st_symbol_new ("<");
    ST_SELECTOR_GT        = st_symbol_new (">");
    ST_SELECTOR_LE        = st_symbol_new ("<=");
    ST_SELECTOR_GE        = st_symbol_new (">=");
    ST_SELECTOR_EQ        = st_symbol_new ("=");
    ST_SELECTOR_NE        = st_symbol_new ("~=");
    ST_SELECTOR_MUL       = st_symbol_new ("*");
    ST_SELECTOR_DIV       = st_symbol_new ("/");
    ST_SELECTOR_MOD       = st_symbol_new ("\\");
    ST_SELECTOR_BITSHIFT  = st_symbol_new ("bitShift:");
    ST_SELECTOR_BITAND    = st_symbol_new ("bitAnd:");
    ST_SELECTOR_BITOR     = st_symbol_new ("bitOr:");
    ST_SELECTOR_BITXOR    = st_symbol_new ("bitXor:");

    ST_SELECTOR_AT        = st_symbol_new ("at:");
    ST_SELECTOR_ATPUT     = st_symbol_new ("at:put:");
    ST_SELECTOR_SIZE      = st_symbol_new ("size");
    ST_SELECTOR_VALUE     = st_symbol_new ("value");
    ST_SELECTOR_VALUE_ARG = st_symbol_new ("value:");
    ST_SELECTOR_IDEQ      = st_symbol_new ("==");
    ST_SELECTOR_CLASS     = st_symbol_new ("class");
    ST_SELECTOR_NEW       = st_symbol_new ("new");
    ST_SELECTOR_NEW_ARG   = st_symbol_new ("new:");

    ST_SELECTOR_DOESNOTUNDERSTAND   = st_symbol_new ("doesNotUnderstand:");
    ST_SELECTOR_MUSTBEBOOLEAN       = st_symbol_new ("mustBeBoolean");
    ST_SELECTOR_STARTUPSYSTEM       = st_symbol_new ("startupSystem");
    ST_SELECTOR_CANNOTRETURN        = st_symbol_new ("cannotReturn");
    ST_SELECTOR_OUTOFMEMORY         = st_symbol_new ("outOfMemory");
}

st_memory *memory;

void
st_bootstrap_universe (void)
{
    st_oop smalltalk;
    st_oop st_object_class_, st_class_class_;

    st_memory_new ();

    ST_NIL = create_nil_object ();

    st_object_class_          = class_new (ST_FORMAT_OBJECT, 0); 
    ST_UNDEFINED_OBJECT_CLASS = class_new (ST_FORMAT_OBJECT, 0);
    ST_METACLASS_CLASS        = class_new (ST_FORMAT_OBJECT, INSTANCE_SIZE_METACLASS);
    ST_BEHAVIOR_CLASS         = class_new (ST_FORMAT_OBJECT, 0);
    st_class_class_           = class_new (ST_FORMAT_OBJECT, INSTANCE_SIZE_CLASS);
    ST_SMI_CLASS              = class_new (ST_FORMAT_OBJECT, 0);
    ST_LARGE_INTEGER_CLASS    = class_new (ST_FORMAT_LARGE_INTEGER, 0);
    ST_CHARACTER_CLASS        = class_new (ST_FORMAT_OBJECT, 0);
    ST_TRUE_CLASS             = class_new (ST_FORMAT_OBJECT, 0);
    ST_FALSE_CLASS            = class_new (ST_FORMAT_OBJECT, 0);
    ST_FLOAT_CLASS            = class_new (ST_FORMAT_FLOAT, 0);
    ST_ARRAY_CLASS            = class_new (ST_FORMAT_ARRAY, 0);
    ST_WORD_ARRAY_CLASS       = class_new (ST_FORMAT_WORD_ARRAY, 0);
    ST_FLOAT_ARRAY_CLASS      = class_new (ST_FORMAT_FLOAT_ARRAY, 0);
    ST_DICTIONARY_CLASS       = class_new (ST_FORMAT_OBJECT, INSTANCE_SIZE_DICTIONARY);
    ST_SET_CLASS              = class_new (ST_FORMAT_OBJECT, INSTANCE_SIZE_SET);
    ST_BYTE_ARRAY_CLASS       = class_new (ST_FORMAT_BYTE_ARRAY, 0);
    ST_SYMBOL_CLASS           = class_new (ST_FORMAT_BYTE_ARRAY, 0);
    ST_STRING_CLASS           = class_new (ST_FORMAT_BYTE_ARRAY, 0);
    ST_WIDE_STRING_CLASS      = class_new (ST_FORMAT_WORD_ARRAY, 0);
    ST_ASSOCIATION_CLASS      = class_new (ST_FORMAT_OBJECT, INSTANCE_SIZE_ASSOCIATION);
    ST_COMPILED_METHOD_CLASS  = class_new (ST_FORMAT_OBJECT, 0);
    ST_METHOD_CONTEXT_CLASS   = class_new (ST_FORMAT_CONTEXT, 5);
    ST_BLOCK_CONTEXT_CLASS    = class_new (ST_FORMAT_CONTEXT, 7);
    ST_SYSTEM_CLASS           = class_new (ST_FORMAT_OBJECT, INSTANCE_SIZE_SYSTEM);
    ST_HANDLE_CLASS           = class_new (ST_FORMAT_HANDLE, 0);

    ST_OBJECT_CLASS (ST_NIL)  = ST_UNDEFINED_OBJECT_CLASS;

    /* special objects */
    ST_TRUE         = st_object_new (ST_TRUE_CLASS);
    ST_FALSE        = st_object_new (ST_FALSE_CLASS);
    ST_SYMBOLS      = st_set_new_with_capacity (256);
    ST_GLOBALS      = st_dictionary_new_with_capacity (256);
    ST_SMALLTALK    = st_object_new (ST_SYSTEM_CLASS);
    ST_OBJECT_FIELDS (ST_SMALLTALK)[0] = ST_GLOBALS;
    ST_OBJECT_FIELDS (ST_SMALLTALK)[1] = ST_SYMBOLS;

    /* add class names to symbol table */
    add_global ("Object", st_object_class_);
    add_global ("UndefinedObject", ST_UNDEFINED_OBJECT_CLASS);
    add_global ("Behavior", ST_BEHAVIOR_CLASS);
    add_global ("Class", st_class_class_);
    add_global ("Metaclass", ST_METACLASS_CLASS);
    add_global ("SmallInteger", ST_SMI_CLASS);
    add_global ("LargeInteger", ST_LARGE_INTEGER_CLASS);
    add_global ("Character", ST_CHARACTER_CLASS);
    add_global ("True", ST_TRUE_CLASS);
    add_global ("False", ST_FALSE_CLASS);
    add_global ("Float", ST_FLOAT_CLASS);
    add_global ("Array", ST_ARRAY_CLASS);
    add_global ("ByteArray", ST_BYTE_ARRAY_CLASS);
    add_global ("WordArray", ST_WORD_ARRAY_CLASS);
    add_global ("FloatArray", ST_FLOAT_ARRAY_CLASS);
    add_global ("ByteString", ST_STRING_CLASS);
    add_global ("ByteSymbol", ST_SYMBOL_CLASS);
    add_global ("WideString", ST_WIDE_STRING_CLASS);
    add_global ("IdentitySet", ST_SET_CLASS);
    add_global ("IdentityDictionary", ST_DICTIONARY_CLASS);
    add_global ("Association", ST_ASSOCIATION_CLASS);
    add_global ("CompiledMethod", ST_COMPILED_METHOD_CLASS);
    add_global ("MethodContext", ST_METHOD_CONTEXT_CLASS);
    add_global ("BlockContext", ST_BLOCK_CONTEXT_CLASS);
    add_global ("Handle", ST_HANDLE_CLASS);
    add_global ("System", ST_SYSTEM_CLASS);
    add_global ("Smalltalk", ST_SMALLTALK);

    init_specials ();
    file_in_classes ();

    st_memory_add_root (ST_NIL);
    st_memory_add_root (ST_TRUE);
    st_memory_add_root (ST_FALSE);
    st_memory_add_root (ST_SMALLTALK);
}

static bool verbosity;

void
st_set_verbosity (bool verbose)
{
    verbosity = verbose;
}

bool
st_verbose_mode (void)
{
    return verbosity;
}


void
st_message (const char *format, ...)
{
    char *new_format;
    va_list args;

    if (!st_verbose_mode ())
	return;

    new_format = st_strconcat ("** ", format, NULL);

    va_start (args, format);
    vfprintf (stderr, new_format, args);
    fputc ('\n', stderr);
    va_end (args);
    st_free (new_format);
}
