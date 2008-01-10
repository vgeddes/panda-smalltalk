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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

#define LA(self, k)   (st_string_stream_look_ahead (self->input, (k)))
#define CONSUME(self) (st_string_stream_consume (self->input))
#define MARK(self)    (st_string_stream_mark (self->input))
#define REWIND(self)  (st_string_stream_rewind (self->input))

struct _st_lexer_t
{
    GooStringStream *input;

    bool token_matched;

    /* data for next token */
    guint line;
    guint column;
    guint start;
    st_lexer_token_t *token;

    /* error control */
    bool failed;
    st_lexer_error_t *last_error;
    jmp_buf main_loop;

    /* token allocator */
    struct obstack allocator;
};

struct _st_lexer_token_t
{
    st_lexer_token_type_t type;
    char *text;
    int line;
    int column;
};

typedef enum
{
    ST_LEXER_ERROR_NONE,
    ST_LEXER_ERROR_MISMATCHED_CHAR,
    ST_LEXER_ERROR_NO_VIABLE_ALT_FOR_CHAR,
    ST_LEXER_ERROR_ILLEGAL_CHAR,
    ST_LEXER_ERROR_UNTERMINATED_COMMENT,
    ST_LEXER_ERROR_UNTERMINATED_STRING_LITERAL,
    ST_LEXER_ERROR_INVALID_CHAR_CONST,
    ST_LEXER_ERROR_NO_ALT_FOR_POUND,

} st_lexer_error_code_t;

struct _st_lexer_error_t
{
    st_lexer_error_code_t code;
    gunichar c;
    guint line;
    guint column;
};


static void
raise_error (st_lexer_t *lexer,
	     st_lexer_error_code_t error_code, gunichar c, guint line, guint column)
{
    lexer->failed = true;

    if (lexer->last_error == NULL)
	lexer->last_error = g_slice_new (st_lexer_error_t);

    lexer->last_error->code = error_code;
    lexer->last_error->c = c;
    lexer->last_error->line = line;
    lexer->last_error->column = column;

    /* hopefully recover after consuming char */
    CONSUME (lexer);

    longjmp (lexer->main_loop, 0);
}

st_lexer_error_t *
st_lexer_last_error (st_lexer_t *lexer)
{
    g_assert (lexer != NULL);

    return lexer->last_error;
}

static inline void
make_token (st_lexer_t *lexer, st_lexer_token_type_t type, char *text)
{
    g_assert (lexer->token != NULL);

    lexer->token_matched = true;

    lexer->token->text = text;
    lexer->token->type = type;
    lexer->token->line = lexer->line;
    lexer->token->column = lexer->column;
}

static void
match_range (st_lexer_t *lexer, gunichar a, gunichar b)
{
    if (LA (lexer, 1) < a || LA (lexer, 1) > b) {
	// mismatch error
	raise_error (lexer,
		     ST_LEXER_ERROR_MISMATCHED_CHAR, LA (lexer, 1), lexer->line, lexer->column);
    }
    CONSUME (lexer);
}

static void
match (st_lexer_t *lexer, gunichar c)
{
    if (LA (lexer, 1) != c) {
	// mismatch error
	raise_error (lexer,
		     ST_LEXER_ERROR_MISMATCHED_CHAR, LA (lexer, 1), lexer->line, lexer->column);
    }
    CONSUME (lexer);
}

static bool
is_special_char (gunichar c)
{
    switch (c) {

    case '+':
    case '/':
    case '\\':
    case '*':
    case '~':
    case '<':
    case '>':
    case '=':
    case '@':
    case '%':
    case '|':
    case '&':
    case '?':
    case '!':
	return true;

    default:
	return false;

    }
}


#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_XDIGIT(c) ((IS_DIGIT (c)) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))

/* Numbers. We do just do basic matching here. Actual parsing and conversion can
 * be done in the parser. 
 */
static void
match_number (st_lexer_t *lexer, bool create_token)
{
    /* We don't match any leading '-'. The parser will resolve whether a '-'
     * specifies a negative number or a binary selector
     */

    /* either match radix value or most significant digits of number */
    do {
	match_range (lexer, '0', '9');
    } while (IS_DIGIT (LA (lexer, 1)));

    /* radix specified, so accept hex values */
    if (LA (lexer, 1) == 'r') {

	CONSUME (lexer);

	if (LA (lexer, 1) == '-')
	    CONSUME (lexer);

	while (IS_XDIGIT (LA (lexer, 1)))
	    CONSUME (lexer);

	if (LA (lexer, 1) == '.' && IS_XDIGIT (LA (lexer, 2))) {
	    CONSUME (lexer);

	    do {
		CONSUME (lexer);
	    } while (IS_XDIGIT (LA (lexer, 1)));
	}

    } else if (LA (lexer, 1) == '.' && IS_DIGIT (LA (lexer, 2))) {

	CONSUME (lexer);

	do {
	    CONSUME (lexer);
	} while (IS_DIGIT (LA (lexer, 1)));

    }

    if (LA (lexer, 1) == 'e') {

	CONSUME (lexer);

	/* consume negative exponent value */
	if (LA (lexer, 1) == '-' && IS_DIGIT (LA (lexer, 2))) {

	    CONSUME (lexer);

	    do {
		CONSUME (lexer);
	    } while (IS_DIGIT (LA (lexer, 1)));

	} else if (IS_DIGIT (LA (lexer, 1))) {

	    /* or consume positive exponent value */

	    do {
		CONSUME (lexer);
	    } while (IS_DIGIT (LA (lexer, 1)));

	}

    }

    if (create_token) {
	make_token (lexer, ST_TOKEN_NUMBER_CONST,
		    st_string_stream_substring (lexer->input,
						lexer->start,
						st_string_stream_index (lexer->input)));
    }
}


static void
match_identifier (st_lexer_t *lexer, bool create_token)
{
    if (g_unichar_isalpha (LA (lexer, 1)))
	CONSUME (lexer);
    else {
	raise_error (lexer,
		     ST_LEXER_ERROR_NO_VIABLE_ALT_FOR_CHAR,
		     LA (lexer, 1), lexer->line, lexer->column);
    }

    while (true) {
	if (g_unichar_isalpha (LA (lexer, 1)))
	    CONSUME (lexer);
	else if (LA (lexer, 1) >= '0' && LA (lexer, 1) <= '9')
	    CONSUME (lexer);
	else if (LA (lexer, 1) == '_')
	    CONSUME (lexer);
	else
	    break;
    }

    if (create_token) {
	make_token (lexer, ST_TOKEN_IDENTIFIER,
		    st_string_stream_substring (lexer->input,
						lexer->start,
						st_string_stream_index (lexer->input)));
    }
}

static void
match_keyword_or_identifier (st_lexer_t *lexer, bool create_token)
{
    if (g_unichar_isalpha (LA (lexer, 1)))
	CONSUME (lexer);
    else {
	raise_error (lexer,
		     ST_LEXER_ERROR_NO_VIABLE_ALT_FOR_CHAR,
		     LA (lexer, 1), lexer->line, lexer->column);
    }

    while (true) {

	if (g_unichar_isalpha (LA (lexer, 1)))
	    CONSUME (lexer);
	else if (LA (lexer, 1) >= '0' && LA (lexer, 1) <= '9')
	    CONSUME (lexer);
	else if (LA (lexer, 1) == '_')
	    CONSUME (lexer);
	else
	    break;
    }

    st_lexer_token_type_t token_type;

    if (LA (lexer, 1) == ':') {
	CONSUME (lexer);
	token_type = ST_TOKEN_KEYWORD_SELECTOR;
    } else {
	token_type = ST_TOKEN_IDENTIFIER;
    }

    if (create_token) {
	char *text;

	if (token_type == ST_TOKEN_KEYWORD_SELECTOR)
	    text = st_string_stream_substring (lexer->input, lexer->start,
					       st_string_stream_index (lexer->input));
	else
	    text = st_string_stream_substring (lexer->input, lexer->start,
					       st_string_stream_index (lexer->input));

	make_token (lexer, token_type, text);
    }

}

static void
match_string_constant (st_lexer_t *lexer)
{
    MARK (lexer);

    match (lexer, '\'');

    while (LA (lexer, 1) != '\'') {
	CONSUME (lexer);

	if (LA (lexer, 1) == ST_STRING_STREAM_EOF) {
	    REWIND (lexer);
	    raise_error (lexer,
			 ST_LEXER_ERROR_UNTERMINATED_STRING_LITERAL,
			 0x0000, lexer->line, lexer->column);
	}
    }

    match (lexer, '\'');

    char *string;

    string = st_string_stream_substring (lexer->input,
					 lexer->start + 1,
					 st_string_stream_index (lexer->input) - 1);

    make_token (lexer, ST_TOKEN_STRING_CONST, string);
}

static inline void
match_comment (st_lexer_t *lexer)
{
    MARK (lexer);

    match (lexer, '"');

    while (LA (lexer, 1) != '"') {
	CONSUME (lexer);

	if (LA (lexer, 1) == ST_STRING_STREAM_EOF) {
	    REWIND (lexer);
	    raise_error (lexer,
			 ST_LEXER_ERROR_UNTERMINATED_COMMENT, 0x0000, lexer->line, lexer->column);
	}
    }

    match (lexer, '"');

    char *comment;

    comment = st_string_stream_substring (lexer->input,
					  lexer->start + 1,
					  st_string_stream_index (lexer->input) - 1);

    make_token (lexer, ST_TOKEN_COMMENT, comment);
}

static inline void
match_array_begin (st_lexer_t *lexer)
{
    match (lexer, '#');
    match (lexer, '(');

    make_token (lexer, ST_TOKEN_ARRAY_BEGIN, NULL);
}

static void
match_binary_selector (st_lexer_t *lexer, bool create_token)
{
    if (LA (lexer, 1) == '-') {
	match (lexer, '-');

    } else if (is_special_char (LA (lexer, 1))) {
	match (lexer, LA (lexer, 1));

	if (is_special_char (LA (lexer, 1)))
	    match (lexer, LA (lexer, 1));

    } else {
	raise_error (lexer,
		     ST_LEXER_ERROR_NO_VIABLE_ALT_FOR_CHAR,
		     LA (lexer, 1), lexer->line, lexer->column);
    }

    if (create_token) {
	make_token (lexer, ST_TOKEN_BINARY_SELECTOR,
		    st_string_stream_substring (lexer->input,
						lexer->start,
						st_string_stream_index (lexer->input)));
    }
}

static void
match_symbol_constant (st_lexer_t *lexer)
{
    match (lexer, '#');

    if (g_unichar_isalpha (LA (lexer, 1))) {

	do {
	    match_keyword_or_identifier (lexer, false);
	} while (g_unichar_isalpha (LA (lexer, 1)));

    } else if (LA (lexer, 1) == '-' || is_special_char (LA (lexer, 1))) {
	match_binary_selector (lexer, false);
    } else {
	raise_error (lexer,
		     ST_LEXER_ERROR_NO_ALT_FOR_POUND, LA (lexer, 1), lexer->line, lexer->column);
    }

    // discard #
    char *symbol_text = st_string_stream_substring (lexer->input,
						    lexer->start + 1,
						    st_string_stream_index (lexer->input));

    make_token (lexer, ST_TOKEN_SYMBOL_CONST, symbol_text);
}

static inline void
match_block_begin (st_lexer_t *lexer)
{
    match (lexer, '[');

    make_token (lexer, ST_TOKEN_BLOCK_BEGIN, NULL);
}

static inline void
match_block_end (st_lexer_t *lexer)
{
    match (lexer, ']');

    make_token (lexer, ST_TOKEN_BLOCK_END, NULL);
}

static inline void
match_lparen (st_lexer_t *lexer)
{
    match (lexer, '(');

    make_token (lexer, ST_TOKEN_LPAREN, NULL);
}

static inline void
match_rparen (st_lexer_t *lexer)
{
    match (lexer, ')');

    make_token (lexer, ST_TOKEN_RPAREN, NULL);
}

static inline void
match_char_constant (st_lexer_t *lexer)
{
    match (lexer, '$');

    if (g_unichar_isgraph (LA (lexer, 1))) {
	CONSUME (lexer);
    } else {
	raise_error (lexer,
		     ST_LEXER_ERROR_INVALID_CHAR_CONST,
		     LA (lexer, 1), lexer->line, lexer->column + 1);
    }

    make_token (lexer, ST_TOKEN_CHAR_CONST,
		st_string_stream_substring (lexer->input,
					    lexer->start + 1,
					    st_string_stream_index (lexer->input)));
}

static inline void
match_eof (st_lexer_t *lexer)
{
    match (lexer, ST_STRING_STREAM_EOF);

    make_token (lexer, ST_TOKEN_EOF, NULL);
}

static inline void
match_white_space (st_lexer_t *lexer)
{
    /* gobble up white space */
    while (true) {
	switch (LA (lexer, 1)) {
	case ' ':
	case '\r':
	case '\n':
	case '\t':
	case '\f':
	    CONSUME (lexer);
	    break;
	default:
	    return;
	}
    }
}

static inline void
match_colon (st_lexer_t *lexer)
{
    match (lexer, ':');
    make_token (lexer, ST_TOKEN_COLON, NULL);
}

static inline void
match_semicolon (st_lexer_t *lexer)
{
    match (lexer, ';');
    make_token (lexer, ST_TOKEN_SEMICOLON, NULL);
}

static inline void
match_assign (st_lexer_t *lexer)
{
    match (lexer, ':');
    match (lexer, '=');
    make_token (lexer, ST_TOKEN_ASSIGN, NULL);
}

static inline void
match_period (st_lexer_t *lexer)
{
    match (lexer, '.');
    make_token (lexer, ST_TOKEN_PERIOD, NULL);
}

static inline void
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
st_lexer_token_t *
st_lexer_next_token (st_lexer_t *lexer)
{
    g_assert (lexer != NULL);

    lexer->token = obstack_alloc (&lexer->allocator, sizeof (st_lexer_token_t));
    lexer->token->type = ST_TOKEN_INVALID;

    while (true) {

	/* reset token and error state */
	lexer->failed = false;
	lexer->token_matched = false;
	lexer->line = st_string_stream_get_line (lexer->input);
	lexer->column = st_string_stream_get_column (lexer->input);
	lexer->start = st_string_stream_index (lexer->input);

	/* we return here on match errors and then goto out */
	if (setjmp (lexer->main_loop))
	    goto out;

	switch (LA (lexer, 1)) {

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

	case '+':
	case '-':
	case '/':
	case '\\':
	case '*':
	case '<':
	case '>':
	case '=':
	case '@':
	case '%':
	case '|':
	case '&':
	case '?':
	case '!':
	case '~':
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

	case ST_STRING_STREAM_EOF:
	    match_eof (lexer);
	    break;

	default:

	    if (g_unichar_isalpha (LA (lexer, 1)))
		match_keyword_or_identifier (lexer, true);

	    else if (IS_DIGIT (LA (lexer, 1)))
		match_number (lexer, true);

	    else if (LA (lexer, 1) == '#' && LA (lexer, 2) == '(')
		match_array_begin (lexer);

	    else if (LA (lexer, 1) == '#')
		match_symbol_constant (lexer);

	    else if (LA (lexer, 1) == ':' && LA (lexer, 2) == '=')
		match_assign (lexer);

	    else if (LA (lexer, 1) == ':')
		match_colon (lexer);

	    else
		raise_error (lexer,
			     ST_LEXER_ERROR_ILLEGAL_CHAR,
			     LA (lexer, 1), lexer->line, lexer->column);
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

    lexer = g_slice_new (st_lexer_t);

    lexer->input = st_string_stream_new (text);

    lexer->token = NULL;
    lexer->line = 1;
    lexer->column = 1;
    lexer->start = -1;

    lexer->failed = FALSE;
    lexer->last_error = NULL;

    obstack_init (&lexer->allocator);

    return lexer;
}

void
st_lexer_destroy (st_lexer_t *lexer)
{
    g_assert (lexer != NULL);

    st_string_stream_destroy (lexer->input);

    // destroy all allocated tokens;
    obstack_free (&lexer->allocator, NULL);

    if (lexer->last_error)
	g_slice_free (st_lexer_error_t, lexer->last_error);

    g_slice_free (st_lexer_t, lexer);
}

st_lexer_token_type_t
st_lexer_token_type (st_lexer_token_t *token)
{
    g_assert (token != NULL);

    return token->type;
}

char *
st_lexer_token_text (st_lexer_token_t *token)
{
    g_assert (token != NULL);

    return token->text;
}

guint
st_lexer_token_line (st_lexer_token_t *token)
{
    g_assert (token != NULL);

    return token->line;
}

guint
st_lexer_token_column (st_lexer_token_t *token)
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
st_lexer_error_message (st_lexer_error_t *error)
{
    g_assert (error != NULL);

    static const char *const message_formats[] = {
	N_("error:%i: ST_LEXER_ERROR_NONE"),
	N_("error:%i: ST_LEXER_ERROR_MISMATCHED_CHAR"),
	N_("error:%i: ST_LEXER_ERROR_NO_VIABLE_ALT_FOR_CHAR"),
	N_("error:%i: illegal character \\u%04X"),
	N_("error:%i: unterminated comment"),
	N_("error:%i: unterminated string literal"),
	N_("error:%i: non-whitespace character expected after '$'"),
	N_("error:%i: expected '(' after '#'"),
    };

    switch (error->code) {
    case ST_LEXER_ERROR_NONE:
    case ST_LEXER_ERROR_MISMATCHED_CHAR:
    case ST_LEXER_ERROR_NO_VIABLE_ALT_FOR_CHAR:
    case ST_LEXER_ERROR_UNTERMINATED_COMMENT:
    case ST_LEXER_ERROR_UNTERMINATED_STRING_LITERAL:
    case ST_LEXER_ERROR_INVALID_CHAR_CONST:
    case ST_LEXER_ERROR_NO_ALT_FOR_POUND:

	return g_strdup_printf (message_formats[error->code], error->line);

    case ST_LEXER_ERROR_ILLEGAL_CHAR:

	return g_strdup_printf (message_formats[error->code], error->line, error->c);
    default:
	return NULL;
    }
}
