/*
 * st-universe.c
 *
 * Copyright (C) 2008 Vincent Geddes <vgeddes@gnome.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "st-types.h"
#include "st-utils.h"
#include "st-object.h"
#include "st-mark.h"
#include "st-float.h"
#include "st-association.h"
#include "st-compiled-code.h"
#include "st-array.h"
#include "st-byte-array.h"
#include "st-small-integer.h"
#include "st-hashed-collection.h"
#include "st-symbol.h"
#include "st-universe.h"
#include "st-heap-object.h"
#include "st-lexer.h"
#include "st-vtable.h"

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

st_oop
    st_nil		= ST_OOP (NULL),
    st_true		= ST_OOP (NULL),
    st_false		= ST_OOP (NULL),
    st_symbol_table	= ST_OOP (NULL),
    st_smalltalk	= ST_OOP (NULL),
    st_undefined_object_class = ST_OOP (NULL),
    st_metaclass_class  = ST_OOP (NULL),
    st_behavior_class   = ST_OOP (NULL),
    st_smi_class	= ST_OOP (NULL),
    st_float_class      = ST_OOP (NULL),
    st_character_class  = ST_OOP (NULL),
    st_true_class       = ST_OOP (NULL),
    st_false_class      = ST_OOP (NULL),
    st_array_class       = ST_OOP (NULL),
    st_byte_array_class  = ST_OOP (NULL),
    st_set_class	 = ST_OOP (NULL),
    st_tuple_class       = ST_OOP (NULL),
    st_dictionary_class  = ST_OOP (NULL),
    st_association_class = ST_OOP (NULL),
    st_string_class      = ST_OOP (NULL),
    st_symbol_class      = ST_OOP (NULL),
    st_compiled_method_class = ST_OOP (NULL),
    st_compiled_block_class  = ST_OOP (NULL);




st_oop
st_global_get (const char *name)
{
    return st_dictionary_at (st_smalltalk, st_symbol_new (name));
}


#include "st-types.h"
#include "st-utils.h"
#include "st-object.h"
#include "st-mark.h"
#include "st-float.h"
#include "st-association.h"
#include "st-compiled-code.h"
#include "st-array.h"
#include "st-byte-array.h"
#include "st-small-integer.h"
#include "st-hashed-collection.h"
#include "st-symbol.h"
#include "st-universe.h"
#include "st-heap-object.h"
#include "st-lexer.h"
#include "st-vtable.h"

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

enum
{
    INSTANCE_SIZE_OBJECT = 0,
    INSTANCE_SIZE_UNDEFINED_OBJECT = 0,
    INSTANCE_SIZE_BEHAVIOR = 0,
    INSTANCE_SIZE_CLASS = 6 + 1,	/* 6 instvars + 1 vtable */
    INSTANCE_SIZE_METACLASS = 6,	/* 5 instvars + 1 vtable */
    INSTANCE_SIZE_SMI = 0,
    INSTANCE_SIZE_CHARACTER = 1,
    INSTANCE_SIZE_BOOLEAN = 0,
    INSTANCE_SIZE_FLOAT = 1,
    INSTANCE_SIZE_ARRAY = 0,
    INSTANCE_SIZE_DICTIONARY = 2,
    INSTANCE_SIZE_SET = 2,
    INSTANCE_SIZE_BYTE_ARRAY = 0,
    INSTANCE_SIZE_SYMBOL = 0,
    INSTANCE_SIZE_ASSOCIATION = 2,
    INSTANCE_SIZE_STRING = 0,
    INSTANCE_SIZE_COMPILED_METHOD = 3,
    INSTANCE_SIZE_COMPILED_BLOCK = 4,

};

static GList *classes = NULL;

static st_oop
st_class_new (const STVTable * table)
{
    st_oop klass = st_allocate_object (ST_TYPE_SIZE (STClass));

    /* TODO refactor this initialising */

    st_heap_object_set_mark (klass, st_mark_new ());
    st_heap_object_set_class (klass, st_nil);

    st_heap_object_set_mark (klass, st_mark_set_nonpointer (st_heap_object_mark (klass), true));

    ST_CLASS_VTABLE (klass) = table;
    st_behavior_set_instance_size (klass, st_nil);
    st_behavior_set_superclass (klass, st_nil);
    st_behavior_set_method_dictionary (klass, st_nil);
    st_behavior_set_instance_variables (klass, st_nil);
    st_class_set_name (klass, st_nil);
    st_class_set_pool (klass, st_nil);

    classes = g_list_append (classes, (void *) klass);

    return klass;
}

static st_oop
declare_class (const char *name, st_oop klass)
{
    // sanity check     for symbol interning
    g_assert (st_symbol_new (name) == st_symbol_new (name));

    st_oop sym = st_symbol_new (name);

    st_dictionary_at_put (st_smalltalk, sym, klass);

    // sanity check for dictionary
    g_assert (st_dictionary_at (st_smalltalk, sym) == klass);

    return klass;
}


static void
parse_error (char *message, STToken *token)
{
    g_warning ("error:%i:%i: %s",
	       st_token_line (token), st_token_column (token), message);
    exit (1);
}

static void
setup_class_final (const char *class_name,
		   const char *superclass_name, GList * ivarnames, GList * cvarnames)
{
    st_oop metaclass, klass, superclass;

    /* we have to special case the declaration for Object since
     * it derives from nil.
     */
    if (streq (class_name, "Object") && streq (superclass_name, "nil")) {

	// see if class has already been allocated
	klass = st_dictionary_at (st_smalltalk, st_symbol_new ("Object"));
	g_assert (klass != st_nil);

	st_behavior_set_superclass (klass, st_nil);

	// get metaclass or create a new one
	if (st_object_class (klass) == st_nil) {
	    metaclass = st_object_new (st_metaclass_class);
	    st_heap_object_set_class (klass, metaclass);
	} else {
	    metaclass = st_object_class (klass);
	}

	st_behavior_set_superclass (metaclass,
				    st_dictionary_at (st_smalltalk, st_symbol_new ("Class")));

	st_behavior_set_instance_size (klass, 0);

    } else {

	// superclass must have already been allocated
	superclass = st_global_get (superclass_name);
	g_assert (superclass != st_nil);

	// see if class has already been allocated
	klass = st_global_get (class_name);
	if (klass == st_nil) {
	    /* we allocate the class and set it's virtual table */
	    klass = st_class_new (ST_CLASS_VTABLE (superclass));
	}

	st_behavior_set_superclass (klass, superclass);

	// get metaclass or create a new one
	if (st_object_class (klass) == st_nil) {
	    metaclass = st_object_new (st_metaclass_class);
	    st_heap_object_set_class (klass, metaclass);
	} else {
	    metaclass = st_object_class (klass);
	}

	st_behavior_set_superclass (metaclass, st_object_class (superclass));

	// set the instance size
	st_smi n_ivars =
	    g_list_length (ivarnames) + st_smi_value (st_behavior_instance_size (superclass));
	st_behavior_set_instance_size (klass, n_ivars);
    }

    st_behavior_set_method_dictionary (metaclass, st_dictionary_new ());
    st_behavior_set_instance_variables (metaclass, st_nil);
    st_behavior_set_instance_size (metaclass, INSTANCE_SIZE_CLASS);
    st_metaclass_set_instance_class (metaclass, klass);

    // set the class name
    st_class_set_name (klass, st_symbol_new (class_name));

    // create instanceVariableNames array
    st_oop instance_variable_names =
	g_list_length (ivarnames) ? st_object_new_arrayed (st_array_class,
							   g_list_length (ivarnames))
	: st_nil;
    st_smi i = 1;
    for (GList * l = ivarnames; l; l = l->next) {
	st_array_at_put (instance_variable_names, i++, st_symbol_new (l->data));
    }
    st_behavior_set_instance_variables (klass, instance_variable_names);

    // create classPool dictionary
    st_oop class_pool = st_dictionary_new ();
    for (GList * l = cvarnames; l; l = l->next) {
	st_dictionary_at_put (class_pool, st_symbol_new (l->data), st_nil);
    }
    st_class_set_pool (klass, class_pool);

    // add methodDictionary
    st_behavior_set_method_dictionary (klass, st_dictionary_new ());

    // add class to SystemDictionary if it's not already there
    st_dictionary_at_put (st_smalltalk, st_symbol_new (class_name), klass);
}


static bool
parse_variable_names (STLexer *lexer, GList ** varnames)
{

    STToken *token;

    token = st_lexer_next_token (lexer);

    if (st_token_type (token) != ST_TOKEN_STRING_CONST)
	return false;

    char *names = st_token_text (token);

    STLexer *ivarlexer = st_lexer_new (names);

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

    // superclass name
    if (st_token_type (token) == ST_TOKEN_IDENTIFIER) {
        
	superclass_name = g_strdup (st_token_text (token));
	token = st_lexer_next_token (lexer);
    } else {
	parse_error ("expected identifier", token);
    }

    // 'subclass:' keyword selector
    if (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR &&
	streq (st_token_text (token), "subclass:")) {

	token = st_lexer_next_token (lexer);
    } else {
	parse_error ("expected 'subclass:' selector", token);
    }

    // class name
    if (st_token_type (token) == ST_TOKEN_SYMBOL_CONST) {

	class_name = g_strdup (st_token_text (token));

	token = st_lexer_next_token (lexer);
    } else {
	parse_error ("expected identifier", token);
    }

    GList *ivarnames = NULL, *cvarnames = NULL;;

    // 'instanceVariableNames:' keyword selector        
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

    setup_class_final (class_name, superclass_name, ivarnames, cvarnames);

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
    GError *error = NULL;

    bool success = g_file_get_contents (filename,
					&contents,
					&len,
					&error);
    if (!success) {
	g_critical (error->message);
	exit (1);
    }

    STLexer *lexer = st_lexer_new (contents);

    STToken *token = st_lexer_next_token (lexer);

    while (st_token_type (token) != ST_TOKEN_EOF) {

	// ignore comments
	while (st_token_type (token) == ST_TOKEN_COMMENT)
	    token = st_lexer_next_token (lexer);

	parse_class (lexer, token);

	token = st_lexer_next_token (lexer);
    }

}

void
st_bootstrap_universe (void)
{
    st_oop _st_object_class, _st_class_class;

    /* must create nil first, since the fields of all newly created objects
     * are initialized to it's st_oop.
     */
    st_nil = st_allocate_object (sizeof (STHeader) / sizeof (st_oop));
    st_heap_object_set_mark (st_nil, st_mark_new ());
    st_heap_object_set_class (st_nil, st_nil);

    _st_object_class = st_class_new (st_object_vtable ());
    st_undefined_object_class = st_class_new (st_heap_object_vtable ());
    st_metaclass_class = st_class_new (st_metaclass_vtable ());
    st_behavior_class = st_class_new (st_heap_object_vtable ());
    _st_class_class = st_class_new (st_class_vtable ());
    st_smi_class = st_class_new (st_smi_vtable ());
    st_character_class = st_class_new (st_heap_object_vtable ());
    st_true_class = st_class_new (st_heap_object_vtable ());
    st_false_class = st_class_new (st_heap_object_vtable ());
    st_float_class = st_class_new (st_float_vtable ());
    st_array_class = st_class_new (st_array_vtable ());
    st_byte_array_class = st_class_new (st_byte_array_vtable ());
    st_dictionary_class = st_class_new (st_dictionary_vtable ());
    st_set_class = st_class_new (st_set_vtable ());
    st_tuple_class = st_class_new (st_array_vtable ());
    st_string_class = st_class_new (st_byte_array_vtable ());
    st_symbol_class = st_class_new (st_symbol_vtable ());
    st_association_class = st_class_new (st_association_vtable ());
    st_compiled_method_class = st_class_new (st_compiled_method_vtable ());
    st_compiled_block_class = st_class_new (st_compiled_block_vtable ());

    st_heap_object_set_class (st_nil, st_undefined_object_class);

    /* setup some initial instance sizes so that we can start instantiating
     * critical objects:
     *   Association, String, Symbol, Set, Dictionary, True, False, Array, ByteArray
     */
    st_behavior_set_instance_size (_st_object_class, 0);
    st_behavior_set_instance_size (st_association_class, 2);
    st_behavior_set_instance_size (st_undefined_object_class, 0);
    st_behavior_set_instance_size (st_symbol_class, 0);
    st_behavior_set_instance_size (st_string_class, 0);
    st_behavior_set_instance_size (st_byte_array_class, 0);
    st_behavior_set_instance_size (st_array_class, 0);
    st_behavior_set_instance_size (st_tuple_class, 0);
    st_behavior_set_instance_size (st_true_class, 0);
    st_behavior_set_instance_size (st_false_class, 0);
    st_behavior_set_instance_size (st_set_class, 2);
    st_behavior_set_instance_size (st_dictionary_class, 2);

    /* create special object instances */
    st_true = st_object_new (st_true_class);
    st_false = st_object_new (st_false_class);
    st_symbol_table = st_set_new_with_capacity (75);
    st_smalltalk = st_dictionary_new_with_capacity (75);

    /* add class names to symbol table */
    declare_class ("Object", _st_object_class);
    declare_class ("UndefinedObject", st_undefined_object_class);
    declare_class ("Behavior", st_behavior_class);
    declare_class ("Class", _st_class_class);
    declare_class ("Metaclass", st_metaclass_class);
    declare_class ("SmallInteger", st_smi_class);
    declare_class ("Character", st_character_class);
    declare_class ("True", st_true_class);
    declare_class ("False", st_false_class);
    declare_class ("Float", st_float_class);
    declare_class ("Array", st_array_class);
    declare_class ("ByteArray", st_byte_array_class);
    declare_class ("String", st_string_class);
    declare_class ("Symbol", st_symbol_class);
    declare_class ("Set", st_set_class);
    declare_class ("Tuple", st_tuple_class);
    declare_class ("Dictionary", st_dictionary_class);
    declare_class ("Association", st_association_class);
    declare_class ("CompiledMethod", st_compiled_method_class);
    declare_class ("CompiledBlock", st_compiled_block_class);

    /* parse class declarations */
    parse_classes ("../smalltalk/class-declarations.st");

    /* verify object graph */
    /*    for (GList * l = objects; l; l = l->next) {

	printf ("verified: %i\n", st_object_verify ((st_oop) l->data));

	if (!st_object_verify ((st_oop) l->data)) {
	    printf ("%s\n", st_object_describe ((st_oop) l->data));
	}

    }
    */

}
