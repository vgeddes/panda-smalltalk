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
#include "st-float.h"
#include "st-association.h"
#include "st-method.h"
#include "st-array.h"
#include "st-byte-array.h"
#include "st-small-integer.h"
#include "st-hashed-collection.h"
#include "st-symbol.h"
#include "st-universe.h"
#include "st-heap-object.h"
#include "st-lexer.h"
#include "st-descriptor.h"
#include "st-compiler.h"
#include "st-virtual-space.h"

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

st_oop
    st_nil		     = 0,
    st_true		     = 0,
    st_false		     = 0,
    st_symbol_table	     = 0,
    st_smalltalk	     = 0,
    st_undefined_object_class = 0,
    st_metaclass_class       = 0,
    st_behavior_class        = 0,
    st_smi_class	     = 0,
    st_large_integer_class   = 0,
    st_float_class           = 0,
    st_character_class       = 0,
    st_true_class            = 0,
    st_false_class           = 0,
    st_array_class           = 0,
    st_byte_array_class      = 0,
    st_set_class	     = 0,
    st_dictionary_class      = 0,
    st_association_class     = 0,
    st_string_class          = 0,
    st_symbol_class          = 0,
    st_compiled_method_class = 0,
    st_method_context_class  = 0,
    st_block_context_class   = 0;


st_oop st_specials[ST_NUM_SPECIALS];

STVirtualSpace *st_virtual_space = NULL;

st_oop
st_global_get (const char *name)
{
    return st_dictionary_at (st_smalltalk, st_symbol_new (name));
}

enum
{
    INSTANCE_SIZE_UNDEFINED = 0,
    INSTANCE_SIZE_CLASS = 7,
    INSTANCE_SIZE_METACLASS = 6,
    INSTANCE_SIZE_DICTIONARY = 2,
    INSTANCE_SIZE_SET = 2,
    INSTANCE_SIZE_ASSOCIATION = 2,
};

static st_oop
class_new (STFormat format, guint instance_size)
{
    st_oop klass;

    klass = st_allocate_object (ST_TYPE_SIZE (STClass));

    /* TODO refactor this initialising */
    st_heap_object_set_format (klass, ST_FORMAT_OBJECT);

    st_heap_object_set_readonly (klass, false);
    st_heap_object_set_nonpointer (klass, false);
    st_heap_object_set_hash (klass, st_current_hash++);			       
    st_heap_object_class (klass) = st_nil;

    st_behavior_format (klass)             = st_smi_new (format);
    st_behavior_instance_size (klass)      = st_smi_new (instance_size);
    st_behavior_superclass (klass)         = st_nil;
    st_behavior_method_dictionary (klass)  = st_nil;
    st_behavior_instance_variables (klass) = st_nil;

    st_class_name (klass) = st_nil;
    st_class_pool (klass) = st_nil;

    return klass;
}

static void
declare_class (const char *name, st_oop klass)
{
    st_oop symbol;

    // sanity check for symbol interning
    g_assert (st_symbol_new (name) == st_symbol_new (name));

    symbol = st_symbol_new (name);
    st_dictionary_at_put (st_smalltalk, symbol, klass);

    // sanity check for dictionary
    g_assert (st_dictionary_at (st_smalltalk, symbol) == klass);
}

static void
parse_error (char *message, STToken *token)
{
    g_warning ("error: %i: %i: %s",
	       st_token_line (token), st_token_column (token), message);
    exit (1);
}

static void
initialize_class  (const char *name,
		   const char *super_name,
		   GList      *ivarnames,
		   GList      *cvarnames)
{
    st_oop metaclass, klass, superclass;

    if (streq (name, "Object") && streq (super_name, "nil")) {

	klass = st_dictionary_at (st_smalltalk, st_symbol_new ("Object"));
	g_assert (klass != st_nil);

	metaclass = st_object_class (klass);
	if (metaclass == st_nil) {
	    metaclass = st_object_new (st_metaclass_class);
	    st_heap_object_class (klass) =  metaclass;
	}

	st_behavior_superclass (klass)     = st_nil;
	st_behavior_instance_size (klass)  = st_smi_new (0);
	st_behavior_superclass (metaclass) = st_dictionary_at (st_smalltalk, st_symbol_new ("Class"));

    } else {

	superclass = st_global_get (super_name);
	g_assert (superclass != st_nil);

	klass = st_global_get (name);
	if (klass == st_nil)
	    klass = class_new (st_smi_value (st_behavior_format (superclass)), 0);

	metaclass = st_object_class (klass);
	if (metaclass == st_nil) {
	    metaclass = st_object_new (st_metaclass_class);
	    st_heap_object_class (klass) = metaclass;
	}

	st_behavior_superclass (klass)     = superclass;
	st_behavior_superclass (metaclass) = st_object_class (superclass);

	st_behavior_instance_size (klass) = st_smi_new (g_list_length (ivarnames) + st_smi_value (st_behavior_instance_size (superclass)));
    }

    st_behavior_format (metaclass)             = st_smi_new (ST_FORMAT_OBJECT);
    st_behavior_method_dictionary (metaclass)  = st_dictionary_new ();
    st_behavior_instance_variables (metaclass) = st_nil;
    st_behavior_instance_size (metaclass)      = st_smi_new (INSTANCE_SIZE_CLASS);
    st_metaclass_instance_class (metaclass)    = klass;

    if (g_list_length (ivarnames) != 0) {
	st_oop names;
	guint i = 1;
	names = st_object_new_arrayed (st_array_class, g_list_length (ivarnames));
	for (GList *l = ivarnames; l; l = l->next)
	    st_array_at_put (names, i++, st_symbol_new (l->data));
	st_behavior_instance_variables (klass) = names;
	
    } else {
	st_behavior_instance_variables (klass) = st_nil;
    }

    st_oop pool = st_dictionary_new ();
    for (GList * l = cvarnames; l; l = l->next) {
	st_dictionary_at_put (pool, st_symbol_new (l->data), st_nil);
    }

    st_class_name (klass) = st_symbol_new (name);
    st_class_pool (klass) = pool;
    st_behavior_method_dictionary (klass) = st_dictionary_new ();

    st_dictionary_at_put (st_smalltalk, st_symbol_new (name), klass);
}


static bool
parse_variable_names (STLexer *lexer, GList **varnames)
{
    STLexer *ivarlexer;
    STToken *token;
    char *names;

    token = st_lexer_next_token (lexer);

    if (st_token_type (token) != ST_TOKEN_STRING_CONST)
	return false;

    names = st_token_text (token);
    ivarlexer = st_lexer_new (names);
    token = st_lexer_next_token (ivarlexer);

    while (st_token_type (token) != ST_TOKEN_EOF) {

	if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
	    parse_error (NULL, token);

	*varnames = g_list_append (*varnames, g_strdup (st_token_text (token)));
	token = st_lexer_next_token (ivarlexer);
    }

    st_lexer_destroy (ivarlexer);

    return true;
}


static void
parse_class (STLexer *lexer, STToken *token)
{
    char *class_name = NULL;
    char *superclass_name = NULL;

    // 'Class' token
    if (st_token_type (token) != ST_TOKEN_IDENTIFIER
	|| !streq (st_token_text (token), "Class"))
	parse_error ("expected class definition", token);	

    // `named:' token
    token = st_lexer_next_token (lexer);
    if (st_token_type (token) != ST_TOKEN_KEYWORD_SELECTOR
	|| !streq (st_token_text (token), "named:"))
	parse_error ("expected 'name:'", token);

    // class name
    token = st_lexer_next_token (lexer);
    if (st_token_type (token) == ST_TOKEN_STRING_CONST) {
	class_name = g_strdup (st_token_text (token));
    } else {
	parse_error ("expected string literal", token);
    }

    // `superclass:' token
    token = st_lexer_next_token (lexer);	
    if (st_token_type (token) != ST_TOKEN_KEYWORD_SELECTOR
	|| !streq (st_token_text (token), "superclass:"))
	parse_error ("expected 'superclass:'", token);

    // superclass name
    token = st_lexer_next_token (lexer);
    if (st_token_type (token) == ST_TOKEN_STRING_CONST) {
        
	superclass_name = g_strdup (st_token_text (token));

    } else {
	parse_error ("expected string literal", token);
    }

    GList *ivarnames = NULL, *cvarnames = NULL;;

    // 'instanceVariableNames:' keyword selector        
    token = st_lexer_next_token (lexer);
    if (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR &&
	streq (st_token_text (token), "instanceVariableNames:")) {

	parse_variable_names (lexer, &ivarnames);

    } else {
	parse_error (NULL, token);
    }

    token = st_lexer_next_token (lexer);

    // 'classVariableNames:' keyword selector   
    if (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR &&
	streq (st_token_text (token), "classVariableNames:")) {

	parse_variable_names (lexer, &cvarnames);

    } else {
	parse_error (NULL, token);
    }

    initialize_class (class_name, superclass_name, ivarnames, cvarnames);

    g_list_free (ivarnames);
    g_list_free (cvarnames);

    token = st_lexer_next_token (lexer);
    
    return;
}

static void
parse_classes (const char *filename)
{
    char *contents;
    gsize len;
    STLexer *lexer;
    STToken *token;
    GError *error = NULL;

    bool success = g_file_get_contents (filename,
					&contents,
					&len,
					&error);
    if (!success) {
	g_critical (error->message);
	exit (1);
    }

    lexer = st_lexer_new (contents);
    token = st_lexer_next_token (lexer);

    while (st_token_type (token) != ST_TOKEN_EOF) {

	while (st_token_type (token) == ST_TOKEN_COMMENT)
	    token = st_lexer_next_token (lexer);

	parse_class (lexer, token);
	token = st_lexer_next_token (lexer);
    }
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
	    "Array.st",
	    "Association.st",
	    "SmallInteger.st",
	    "Object.st",
	    "UndefinedObject.st",
	    "String.st",
	    "ByteString.st",
	    "Character.st",
	    "Behavior.st",
	    "Boolean.st",
	    "True.st",
	    "False.st",
	    "Behavior.st",
	    "ContextPart.st",
	    "BlockContext.st",
	    "Message.st"
	};

    for (guint i = 0; i < G_N_ELEMENTS (files); i++) {
	filename = g_build_filename ("..", "st", files[i], NULL);
	st_file_in (filename);
	g_free (filename);
    }
}

static st_oop
create_nil_object (void)
{
    st_oop nil;

    nil = st_allocate_object (sizeof (STHeader) / sizeof (st_oop));

    st_heap_object_set_readonly   (nil, false);
    st_heap_object_set_nonpointer (nil, false);
    st_heap_object_set_format     (nil, ST_FORMAT_OBJECT);
    st_heap_object_set_hash       (nil, st_current_hash++);
    st_heap_object_class          (nil) = nil;

    return nil;
}

static void
init_specials (void)
{
    st_specials[ST_SPECIAL_PLUS]      = st_symbol_new ("+");
    st_specials[ST_SPECIAL_MINUS]     = st_symbol_new ("-");
    st_specials[ST_SPECIAL_LT]        = st_symbol_new ("<");
    st_specials[ST_SPECIAL_GT]        = st_symbol_new (">");
    st_specials[ST_SPECIAL_LE]        = st_symbol_new ("<=");
    st_specials[ST_SPECIAL_GE]        = st_symbol_new (">=");
    st_specials[ST_SPECIAL_EQ]        = st_symbol_new ("=");
    st_specials[ST_SPECIAL_NE]        = st_symbol_new ("~=");
    st_specials[ST_SPECIAL_MUL]       = st_symbol_new ("*");
    st_specials[ST_SPECIAL_DIV]       = st_symbol_new ("/");
    st_specials[ST_SPECIAL_MOD]       = st_symbol_new ("\\");
    st_specials[ST_SPECIAL_BITSHIFT]  = st_symbol_new ("bitShift:");
    st_specials[ST_SPECIAL_BITAND]    = st_symbol_new ("bitAnd:");
    st_specials[ST_SPECIAL_BITOR]     = st_symbol_new ("bitOr:");
    st_specials[ST_SPECIAL_BITXOR]    = st_symbol_new ("bitXor:");

    st_specials[ST_SPECIAL_AT]        = st_symbol_new ("at:");
    st_specials[ST_SPECIAL_ATPUT]     = st_symbol_new ("at:put:");
    st_specials[ST_SPECIAL_SIZE]      = st_symbol_new ("size");
    st_specials[ST_SPECIAL_VALUE]     = st_symbol_new ("value");
    st_specials[ST_SPECIAL_VALUE_ARG] = st_symbol_new ("value:");
    st_specials[ST_SPECIAL_IDEQ]      = st_symbol_new ("==");
    st_specials[ST_SPECIAL_CLASS]     = st_symbol_new ("class");
    st_specials[ST_SPECIAL_NEW]       = st_symbol_new ("new");
    st_specials[ST_SPECIAL_NEW_ARG]   = st_symbol_new ("new:");
}

// RESERVE 100 MB worth of virtual address space
#define HEAP_SIZE (100 * 1024  * 1024)

static void
allocate_virtual_space (void)
{
    st_virtual_space = st_virtual_space_new ();

    if (!st_virtual_space_reserve (st_virtual_space, HEAP_SIZE)) {
	abort ();
    }
}

void
st_bootstrap_universe (void)
{
    st_oop st_object_class_, st_class_class_;

    allocate_virtual_space ();

    /* setup format descriptors */
    st_descriptors[ST_FORMAT_OBJECT]     = st_heap_object_descriptor ();
    st_descriptors[ST_FORMAT_ARRAY]      = st_array_descriptor       ();
    st_descriptors[ST_FORMAT_BYTE_ARRAY] = st_byte_array_descriptor  ();
    st_descriptors[ST_FORMAT_FLOAT]      = st_float_descriptor       ();

    st_nil = create_nil_object ();

    st_object_class_          = class_new (ST_FORMAT_OBJECT, 0); 
    st_undefined_object_class = class_new (ST_FORMAT_OBJECT, 0);
    st_metaclass_class        = class_new (ST_FORMAT_OBJECT, 0);
    st_behavior_class         = class_new (ST_FORMAT_OBJECT, 0);
    st_class_class_           = class_new (ST_FORMAT_OBJECT, 0);
    st_smi_class              = class_new (ST_FORMAT_OBJECT, 0);
    st_large_integer_class    = class_new (ST_FORMAT_LARGE_INTEGER, 0);
    st_character_class        = class_new (ST_FORMAT_OBJECT, 0);
    st_true_class             = class_new (ST_FORMAT_OBJECT, 0);
    st_false_class            = class_new (ST_FORMAT_OBJECT, 0);
    st_float_class            = class_new (ST_FORMAT_FLOAT, 0);
    st_array_class            = class_new (ST_FORMAT_ARRAY, 0);
    st_dictionary_class       = class_new (ST_FORMAT_OBJECT, INSTANCE_SIZE_DICTIONARY);
    st_set_class              = class_new (ST_FORMAT_OBJECT, INSTANCE_SIZE_SET);
    st_byte_array_class       = class_new (ST_FORMAT_BYTE_ARRAY, 0);
    st_string_class           = class_new (ST_FORMAT_BYTE_ARRAY, 0);
    st_symbol_class           = class_new (ST_FORMAT_BYTE_ARRAY, 0);
    st_association_class      = class_new (ST_FORMAT_OBJECT, INSTANCE_SIZE_ASSOCIATION);
    st_compiled_method_class  = class_new (ST_FORMAT_OBJECT, 0);

    st_heap_object_class (st_nil) = st_undefined_object_class;

    /* special objects */
    st_true         = st_object_new (st_true_class);
    st_false        = st_object_new (st_false_class);
    st_symbol_table = st_set_new_with_capacity (75);
    st_smalltalk    = st_dictionary_new_with_capacity (75);

    /* add class names to symbol table */
    declare_class ("Object", st_object_class_);
    declare_class ("UndefinedObject", st_undefined_object_class);
    declare_class ("Behavior", st_behavior_class);
    declare_class ("Class", st_class_class_);
    declare_class ("Metaclass", st_metaclass_class);
    declare_class ("SmallInteger", st_smi_class);
    declare_class ("LargeInteger", st_large_integer_class);
    declare_class ("Character", st_character_class);
    declare_class ("True", st_true_class);
    declare_class ("False", st_false_class);
    declare_class ("Float", st_float_class);
    declare_class ("Array", st_array_class);
    declare_class ("ByteArray", st_byte_array_class);
//    declare_class ("String", st_string_class);
    declare_class ("ByteString", st_string_class);
    declare_class ("ByteSymbol", st_symbol_class);
    declare_class ("Set", st_set_class);
    declare_class ("Dictionary", st_dictionary_class);
    declare_class ("Association", st_association_class);
    declare_class ("CompiledMethod", st_compiled_method_class);

    init_specials ();

    file_in_classes ();
    
    st_method_context_class = st_global_get ("MethodContext");
    st_block_context_class = st_global_get ("BlockContext");
    g_assert (st_method_context_class != st_nil);
    g_assert (st_block_context_class != st_nil);
}
