/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-lexer.c
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
 * MERCHANTABILITY or FITNESS FOR A PARTICUlookaheadR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * we expand utf8 encoded text to ucs4 encoding and then lex it. Yes it's inefficient,
 * but much cleaner and straightforward than munging around with utf8-encoded multi-byte
 * characters.
 *
 */

#include <config.h>

#include <st-lexer.h>
#include <st-string-stream.h>

#include <glib/gi18n-lib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include <obstack.h>

#define obstack_chunk_alloc g_malloc
#define obstack_chunk_free  g_free


#define lookahead(self, k)   (st_input_look_ahead (self->input, (k)))
#define consume(self)        (st_input_consume (self->input))
#define mark(self)           (st_input_mark (self->input))
#define rewind(self)         (st_input_rewind (self->input))

struct st_lexer_t
{
    st_input_t *input;

    bool token_matched;

    /* data for next token */
    guint line;
    guint column;
    guint start;
    st_token_t *token;

    /* error control */
    bool failed;
    jmp_buf main_loop;
    
    /* last error information */
    error_code_t error_code;
    guint        error_line;
    guint        error_column;
    gunichar     error_char;

    /* token allocator */
    struct obstack allocator;
};

struct st_token_t
{
    st_token_type_t type;
    char *text;
    int   line;
    int   column;
};

struct st_number_token_t
{
    st_token_t parent;
    
    int   radix;
    int   exponent;
    char *number;
};

typedef enum
{
    LEXER_ERROR_MISMATCHED_CHAR,
    LEXER_ERROR_NO_VIABLE_ALT_FOR_CHAR,
    LEXER_ERROR_ILLEGAL_CHAR,
    LEXER_ERROR_UNTERMINATED_COMMENT,
    LEXER_ERROR_UNTERMINATED_STRING_LITERAL,
    LEXER_ERROR_INVALID_RADIX,
    LEXER_ERROR_INVALID_CHAR_CONST,
    LEXER_ERROR_NO_ALT_FOR_POUND,

} error_code_t;

static void
raise_error (st_lexer_t   *lexer,
	     error_code_t  error_code)
{
    lexer->failed = true;

    lexer->error_code   = error_code;
    lexer->error_char   = c;
    lexer->error_line   = lexer->line;
    lexer->error_column = lexer->column;

    /* create an token of type invalid */
    make_token (lexer, ST_TOKEN_INVALID, NULL);    

    /* hopefully recover after consuming char */
    consume (lexer);

    /* go back to main loop */
    longjmp (lexer->main_loop, 0);
}

static void
make_number_token (int radix, int exponent, char* number)
{
    st_token_t *token;

    token = obstack_alloc (&lexer->allocator, sizeof (st_number_token_t));
    
    token->type   = type;
    token->text   = number;
    token->type   = type;
    token->line   = lexer->line;
    token->column = lexer->column;

    st_number_token_t *number_token = ST_NUMBER_TOKEN (token);
    number_token->radix    = radix;
    number_token->exponent = exponent;

    lexer->token = token;
    lexer->token_matched = true;
    
    return token;

}

static void
make_token (st_lexer_t      *lexer,
	    st_token_type_t  type,
	    char            *text)
{
    st_token_t *token;

    token = obstack_alloc (&lexer->allocator, sizeof (st_token_t));
    
    token->type   = type;
    token->text   = text;
    token->type   = type;
    token->line   = lexer->line;
    token->column = lexer->column;

    lexer->token = token;
    lexer->token_matched = true;
    
    return token;
}

static void
match_range (st_lexer_t *lexer, gunichar a, gunichar b)
{
    if (lookahead (lexer, 1) < a || lookahead (lexer, 1) > b) {
	// mismatch error
	raise_error (lexer, LEXER_ERROR_MISMATCHED_CHAR);
    }
    consume (lexer);
}

static void
match (st_lexer_t *lexer, gunichar c)
{
    if (lookahead (lexer, 1) != c) {
	// mismatch error
	raise_error (lexer, LEXER_ERROR_MISMATCHED_CHAR);
    }
    consume (lexer);
}

static bool
is_special_char (gunichar c)
{
    switch (c) {

    case '+': case '/': case '\\': case '*': case '~':
    case '<': case '>': case '=': case '@': case '%':
    case '|': case '&': case '?': case '!':
	return true;

    default:
	return false;

    }
}

/* check if a char is valid numeral identifier for a given radix */
bool
is_radix_numeral (guint radix, gunichar c)
{
    g_assert (radix >= 2 && radix <= 36);
    
    if (radix > 10)
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= ('A' - 1 + (radix - 10)));
    else
	return c >= '0' && c <= ('0' - 1 + radix);
}

/* Numbers. We do just do basic matching here. Actual parsing and conversion can
 * be done in the parser. 
 */
static void
match_number (st_lexer_t *lexer)
{
    /* We don't match any leading '-'. The parser will resolve whether a '-'
     * specifies a negative number or a binary selector
     */

    int radix = 10;
    int exponent = 0;
    int start, end, exponent_start;
    char *string;
    
    do {
    
	match_range (lexer, '0', '9');
	
    } while (isdigit (lookahead (lexer, 1)));
   
    if (lookahead (lexer, 1) == 'r') {	
	
	char *string = st_input_substring (lexer->input, lexer->start,
					   st_input_index (lexer->input));				
	radix = strtol (string, NULL, 10);
	g_free (string);
	if (radix < 2 || radix > 36) {
	    raise_error (lexer, LEXER_ERROR_INVALID_RADIX);
	}

	consume (lexer);
	    
    } else {
	/* matched an integer so go make token */
    	goto out;
    }    

    start = st_input_index (lexer->input);

    if (lookahead (lexer, 1) == '-') {
	consume (lexer);

    while (is_radix_numeral (radix, lookahead (lexer, 1)))
	consume (lexer);

    if (lookahead (lexer, 1) == '.' && is_radix_numeral (radix, lookahead (lexer, 2))) {
	consume (lexer);

	do {
	    consume (lexer);
	} while (is_radix_numeral (radix, lookahead (lexer, 1)));
    }  
       

    if (lookahead (lexer, 1) == 'e') {

	consume (lexer);
	
	exponent_start = st_input_index (lexer->input);

	if (lookahead (lexer, 1) == '-' && isdigit (lookahead (lexer, 2)))
	    consume (lexer);

	while (isdigit (lookahead (lexer, 1)))
		consume (lexer);
		
	if (exponent_start == st_input_index (lexer->input))
	    goto out;
	
	string = st_input_substring (lexer->input, exponent_start,
				     st_input_index (lexer->input));				    
	exponent = strtol (string, NULL, 10);
    }
    
out:

    make_number_token (lexer, radix, exponent,
		       st_input_substring (lexer->input,
					   start,
					   st_input_index (lexer->input)));
}


static void
match_identifier (st_lexer_t *lexer, bool create_token)
{
    if (g_unichar_isalpha (lookahead (lexer, 1)))
	consume (lexer);
    else {
	raise_error (lexer, LEXER_ERROR_NO_VIABLE_ALT_FOR_CHAR);
    }

    while (true) {
	if (g_unichar_isalpha (lookahead (lexer, 1)))
	    consume (lexer);
	else if (lookahead (lexer, 1) >= '0' && lookahead (lexer, 1) <= '9')
	    consume (lexer);
	else if (lookahead (lexer, 1) == '_')
	    consume (lexer);
	else
	    break;
    }

    if (create_token) {
	make_token (lexer, ST_TOKEN_IDENTIFIER,
		    st_input_substring (lexer->input,
						lexer->start,
						st_input_index (lexer->input)));
    }
}

static void
match_keyword_or_identifier (st_lexer_t *lexer, bool create_token)
{
    if (g_unichar_isalpha (lookahead (lexer, 1)))
	consume (lexer);
    else {
	raise_error (lexer, LEXER_ERROR_NO_VIABLE_ALT_FOR_CHAR);
    }

    while (true) {

	if (g_unichar_isalpha (lookahead (lexer, 1)))
	    consume (lexer);
	else if (lookahead (lexer, 1) >= '0' && lookahead (lexer, 1) <= '9')
	    consume (lexer);
	else if (lookahead (lexer, 1) == '_')
	    consume (lexer);
	else
	    break;
    }

    st_token_type_t token_type;

    if (lookahead (lexer, 1) == ':') {
	consume (lexer);
	token_type = ST_TOKEN_KEYWORD_SELECTOR;
    } else {
	token_type = ST_TOKEN_IDENTIFIER;
    }

    if (create_token) {
	char *text;

	if (token_type == ST_TOKEN_KEYWORD_SELECTOR)
	    text = st_input_substring (lexer->input, lexer->start,
					       st_input_index (lexer->input));
	else
	    text = st_input_substring (lexer->input, lexer->start,
					       st_input_index (lexer->input));

	make_token (lexer, token_type, text);
    }

}

static void
match_string_constant (st_lexer_t *lexer)
{
    mark (lexer);

    match (lexer, '\'');

    while (lookahead (lexer, 1) != '\'') {
	consume (lexer);

	if (lookahead (lexer, 1) == ST_INPUT_EOF) {
	    rewind (lexer);
	    raise_error (lexer, LEXER_ERROR_UNTERMINATED_STRING_LITERAL);
	}
    }

    match (lexer, '\'');

    char *string;

    string = st_input_substring (lexer->input,
					 lexer->start + 1,
					 st_input_index (lexer->input) - 1);

    make_token (lexer, ST_TOKEN_STRING_CONST, string);
}

static void
match_comment (st_lexer_t *lexer)
{
    mark (lexer);

    match (lexer, '"');

    while (lookahead (lexer, 1) != '"') {
	consume (lexer);

	if (lookahead (lexer, 1) == ST_INPUT_EOF) {
	    rewind (lexer);
	    raise_error (lexer, LEXER_ERROR_UNTERMINATED_COMMENT);
	}
    }

    match (lexer, '"');

    char *comment;

    comment = st_input_substring (lexer->input,
					  lexer->start + 1,
					  st_input_index (lexer->input) - 1);

    make_token (lexer, ST_TOKEN_COMMENT, comment);
}

static void
match_tuple_begin (st_lexer_t *lexer)
{
    match (lexer, '#');
    match (lexer, '(');

    make_token (lexer, ST_TOKEN_TUPLE_BEGIN, NULL);
}

static void
match_binary_selector (st_lexer_t *lexer, bool create_token)
{
    if (lookahead (lexer, 1) == '-') {
	match (lexer, '-');

    } else if (is_special_char (lookahead (lexer, 1))) {
	match (lexer, lookahead (lexer, 1));

	if (is_special_char (lookahead (lexer, 1)))
	    match (lexer, lookahead (lexer, 1));

    } else {
	raise_error (lexer, LEXER_ERROR_NO_VIABLE_ALT_FOR_CHAR);
    }

    if (create_token) {
	make_token (lexer, ST_TOKEN_BINARY_SELECTOR,
		    st_input_substring (lexer->input,
						lexer->start,
						st_input_index (lexer->input)));
    }
}

static void
match_symbol_constant (st_lexer_t *lexer)
{
    match (lexer, '#');

    if (g_unichar_isalpha (lookahead (lexer, 1))) {

	do {
	    match_keyword_or_identifier (lexer, false);
	} while (g_unichar_isalpha (lookahead (lexer, 1)));

    } else if (lookahead (lexer, 1) == '-' || is_special_char (lookahead (lexer, 1))) {
	match_binary_selector (lexer, false);
    } else {
	raise_error (lexer, LEXER_ERROR_NO_ALT_FOR_POUND);
    }

    // discard #
    char *symbol_text = st_input_substring (lexer->input,
						    lexer->start + 1,
						    st_input_index (lexer->input));

    make_token (lexer, ST_TOKEN_SYMBOL_CONST, symbol_text);
}

static void
match_block_begin (st_lexer_t *lexer)
{
    match (lexer, '[');

    make_token (lexer, ST_TOKEN_BLOCK_BEGIN, NULL);
}

static void
match_block_end (st_lexer_t *lexer)
{
    match (lexer, ']');

    make_token (lexer, ST_TOKEN_BLOCK_END, NULL);
}

static void
match_lparen (st_lexer_t *lexer)
{
    match (lexer, '(');

    make_token (lexer, ST_TOKEN_LPAREN, NULL);
}

static void
match_rparen (st_lexer_t *lexer)
{
    match (lexer, ')');

    make_token (lexer, ST_TOKEN_RPAREN, NULL);
}

static void
match_char_constant (st_lexer_t *lexer)
{
    match (lexer, '$');

    if (g_unichar_isgraph (lookahead (lexer, 1))) {
	consume (lexer);
    } else {
	raise_error (lexer, LEXER_ERROR_INVALID_CHAR_CONST);
    }

    make_token (lexer, ST_TOKEN_CHAR_CONST,
		st_input_substring (lexer->input,
					    lexer->start + 1,
					    st_input_index (lexer->input)));
}

static void
match_eof (st_lexer_t *lexer)
{
    match (lexer, ST_INPUT_EOF);

    make_token (lexer, ST_TOKEN_EOF, NULL);
}

static void
match_white_space (st_lexer_t *lexer)
{
    /* gobble up white space */
    while (true) {
	switch (lookahead (lexer, 1)) {
	case ' ':
	case '\r':
	case '\n':
	case '\t':
	case '\f':
	    consume (lexer);
	    break;
	default:
	    return;
	}
    }
}

static void
match_colon (st_lexer_t *lexer)
{
    match (lexer, ':');
    make_token (lexer, ST_TOKEN_COLON, NULL);
}

static void
match_semicolon (st_lexer_t *lexer)
{
    match (lexer, ';');
    make_token (lexer, ST_TOKEN_SEMICOLON, NULL);
}

static void
match_assign (st_lexer_t *lexer)
{
    match (lexer, ':');
    match (lexer, '=');
    make_token (lexer, ST_TOKEN_ASSIGN, NULL);
}

static void
match_period (st_lexer_t *lexer)
{
    match (lexer, '.');
    make_token (lexer, ST_TOKEN_PERIOD, NULL);
}

static void
match_return (st_lexer_t *lexer)
{
    match (lexer, '^');
    make_token (lexer, ST_TOKEN_RETURN, NULL);
}

/* st_lexer_next_token:
 * lexer: a st_lexer_t
 *
 * Returns the next matched token from the input stream. Caller takes
 * ownership of returned token.
 *
 * If the end of the input stream is reached, tokens of type ST_TOKEN_EOF
 * will be returned. Similarly, if there are matching errors, then tokens
 * of type ST_TOKEN_INVALID will be returned;
 *
 */
st_token_t *
st_lexer_next_token (st_lexer_t *lexer)
{
    g_assert (lexer != NULL);

    while (true) {

	/* reset token and error state */
	lexer->failed = false;
	lexer->token_matched = false;
	lexer->line   = st_input_get_line (lexer->input);
	lexer->column = st_input_get_column (lexer->input);
	lexer->start  = st_input_index (lexer->input);

	/* we return here on match errors and then goto out */
	if (setjmp (lexer->main_loop))
	    goto out;

	switch (lookahead (lexer, 1)) {

	case ' ':
	case '\n':
	case '\r':
	case '\t':
	case '\f':
	    match_white_space (lexer);
	    break;

	case '(':
	    match_lparen (lexer);
	    break;

	case ')':
	    match_rparen (lexer);
	    break;

	case '[':
	    match_block_begin (lexer);
	    break;

	case ']':
	    match_block_end (lexer);
	    break;

	case '^':
	    match_return (lexer);
	    break;

	case '.':
	    match_period (lexer);
	    break;

	case ';':
	    match_semicolon (lexer);
	    break;

	case '+': case '-': case '/': case '\\':
	case '*': case '<': case '>': case '=':
	case '@': case '%': case '|': case '&':
	case '?': case '!': case '~':
	    match_binary_selector (lexer, true);
	    break;

	case '$':
	    match_char_constant (lexer);
	    break;

	case '"':
	    match_comment (lexer);
	    break;

	case '\'':
	    match_string_constant (lexer);
	    break;

	case ST_INPUT_EOF:
	    match_eof (lexer);
	    break;

	default:

	    if (g_unichar_isalpha (lookahead (lexer, 1)))
		match_keyword_or_identifier (lexer, true);

	    else if (IS_DIGIT (lookahead (lexer, 1)))
		match_number (lexer);

	    else if (lookahead (lexer, 1) == '#' && lookahead (lexer, 2) == '(')
		match_tuple_begin (lexer);

	    else if (lookahead (lexer, 1) == '#')
		match_symbol_constant (lexer);

	    else if (lookahead (lexer, 1) == ':' && lookahead (lexer, 2) == '=')
		match_assign (lexer);

	    else if (lookahead (lexer, 1) == ':')
		match_colon (lexer);

	    else
		raise_error (lexer, LEXER_ERROR_ILLEGAL_CHAR);
	}

      out:

	// we return the matched token or an invalid token on error
	if (lexer->token_matched || lexer->failed)
	    return lexer->token;
	else
	    continue;

    }
}

st_lexer_t *
st_lexer_new (const char *text)
{
    st_lexer_t *lexer;

    g_assert (text != NULL);

    lexer = g_slice_new0 (st_lexer_t);

    lexer->input = st_input_new (text);

    lexer->token = NULL;
    lexer->line = 1;
    lexer->column = 1;
    lexer->start = -1;
    
    lexer->error_code = NULL;


    lexer->failed = FALSE;
    lexer->last_error = NULL;

    obstack_init (&lexer->allocator);

    return lexer;
}

void
st_lexer_destroy (st_lexer_t *lexer)
{
    g_assert (lexer != NULL);

    st_input_destroy (lexer->input);

    // destroy all allocated tokens;
    obstack_free (&lexer->allocator, NULL);

    if (lexer->last_error)
	g_slice_free (st_lexer_error_t, lexer->last_error);

    g_slice_free (st_lexer_t, lexer);
}

st_token_type_t
st_token_type (st_token_t *token)
{
    g_assert (token != NULL);

    return token->type;
}

char *
st_token_text (st_token_t *token)
{
    g_assert (token != NULL);

    return token->text;
}

guint
st_lexer_token_line (st_token_t *token)
{
    g_assert (token != NULL);

    return token->line;
}

guint
st_lexer_token_column (st_token_t *token)
{
    g_assert (token != NULL);

    return token->column;
}

guint
st_lexer_error_line (st_lexer_error_t *error)
{
    g_assert (error != NULL);

    return error->line;
}

guint
st_lexer_error_column (st_lexer_error_t *error)
{
    g_assert (error != NULL);

    return error->column;
}

gunichar
st_lexer_error_char (st_lexer_error_t *error)
{
    g_assert (error != NULL);

    return error->c;
}

char *
st_lexer_error_message (st_lexer_t *lexer)
{
    g_assert (error != NULL);

    static const char *msgformats[] = {
	N_("mismatched character \\u%04X"),
	N_("no viable alternative for character \\u%04X"),
	N_("illegal character \\u%04X"),
	N_("unterminated comment"),
	N_("unterminated string literal"),
	N_("radix invalid for number"),
	N_("non-whitespace character expected after '$'"),
	N_("expected '(' after '#'"),
    };

    switch (lexer->error_code) {
    
    case LEXER_ERROR_UNTERMINATED_COMMENT:
    case LEXER_ERROR_UNTERMINATED_STRING_LITERAL:
    case LEXER_ERROR_INVALID_RADIX:
    case LEXER_ERROR_INVALID_CHAR_CONST:
    case LEXER_ERROR_NO_ALT_FOR_POUND:

	return g_strdup_printf (msgformats[lexer->error_code]);

    case LEXER_ERROR_MISMATCHED_CHAR:
    case LEXER_ERROR_NO_VIABLE_ALT_FOR_CHAR:
    case LEXER_ERROR_ILLEGAL_CHAR:

	return g_strdup_printf (msgformats[lexer->error_code], lexer->error_char);
	
    default:
	return NULL;
    }
}
