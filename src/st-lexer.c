/*
 * st-lexer.c
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

/* Notes:
 *
 * we expand utf8-encoded text to ucs4 format and then lex it. Yes it's a bit
 * inefficient, but more straightforward than munging around with
 * multi-byte characters.
 *
 * Character input is supplied by the st_input object. It keeps track of
 * line/column numbers and has the ability to mark() and rewind() on the
 * input stream.
 *
 */

#include <config.h>

#include "st-lexer.h"
#include "st-input.h"
#include "st-utils.h"

#include <stdbool.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include <limits.h>
#include <ctype.h>

#define lookahead(self, k)   ((gunichar) st_input_look_ahead (self->input, k))
#define consume(self)        (st_input_consume (self->input))
#define mark(self)           (st_input_mark (self->input))
#define rewind(self)         (st_input_rewind (self->input))

typedef enum
{
    ERROR_MISMATCHED_CHAR,
    ERROR_NO_VIABLE_ALT_FOR_CHAR,
    ERROR_ILLEGAL_CHAR,
    ERROR_UNTERMINATED_COMMENT,
    ERROR_UNTERMINATED_STRING_LITERAL,
    ERROR_INVALID_RADIX,
    ERROR_INVALID_CHAR_CONST,
    ERROR_NO_ALT_FOR_POUND,

} ErrorCode;

struct st_lexer
{
    st_input *input;

    bool filter_comments;

    bool token_matched;
    
    /* data for next token */
    st_uint line;
    st_uint column;
    st_uint start;
    st_token *token;

    /* error control */
    bool    failed;
    jmp_buf main_loop;
    
    /* last error information */
    ErrorCode    error_code;
    st_uint        error_line;
    st_uint        error_column;
    gunichar     error_char;

    /* delayed deallocation */
    st_list *allocated_tokens;
};

struct st_token
{
    st_token_type type;
    int   line;
    int   column;

    union {
	struct {
	    char *text;
	};
	/* Number Token */
	struct {
	    bool  negative;
	    char *number;
	    int   radix;
	    int   exponent;
	};
    };
};

static void
make_token (st_lexer      *lexer,
	    st_token_type  type,
	    char        *text)
{
    st_token *token;
 
    token = st_new0 (st_token);

    token->type   = type;
    token->text   = text ? text : st_strdup ("");
    token->type   = type;
    token->line   = lexer->line;
    token->column = lexer->column;

    lexer->token = token;
    lexer->token_matched = true;


    lexer->allocated_tokens = st_list_prepend (lexer->allocated_tokens, token);
}

static void
make_number_token (st_lexer *lexer, int radix, int exponent, char *number, bool negative)
{
    st_token *token;

    token = st_new0 (st_token);

    token->type   = ST_TOKEN_NUMBER_CONST;
    token->line   = lexer->line;
    token->column = lexer->column;

    token->negative = negative;
    token->number   = number;
    token->radix    = radix;
    token->exponent = exponent;

    lexer->token = token;
    lexer->token_matched = true;

    lexer->allocated_tokens = st_list_prepend (lexer->allocated_tokens, token);
}

static void
raise_error (st_lexer   *lexer,
	     ErrorCode  error_code,
	     gunichar      error_char)
{
    lexer->failed = true;

    lexer->error_code   = error_code;
    lexer->error_char   = error_char;
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
match_range (st_lexer *lexer, gunichar a, gunichar b)
{
    if (lookahead (lexer, 1) < a || lookahead (lexer, 1) > b) {
	// mismatch error
	raise_error (lexer, ERROR_MISMATCHED_CHAR, lookahead (lexer, 1));
    }
    consume (lexer);
}

static void
match (st_lexer *lexer, gunichar c)
{
    if (lookahead (lexer, 1) != c) {
	// mismatch error
	raise_error (lexer, ERROR_MISMATCHED_CHAR, lookahead (lexer, 1));
    }
    consume (lexer);
}

static bool
is_special_char (gunichar c)
{
    switch (c) {

    case '+': case '/': case '\\': case '*': case '~':
    case '<': case '>': case '=': case '@': case '%':
    case '|': case '&': case '?': case '!': case ',':
	return true;

    default:
	return false;

    }
}

/* check if a char is valid numeral identifier for a given radix
 * 
 * for example, 2r1010301 is an invalid number since the '3' is not within the radix.
 * 
 **/
static bool
is_radix_numeral (st_uint radix, gunichar c)
{
    st_assert (radix >= 2 && radix <= 36);
    
    if (radix > 10)
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= ('A' - 1 + (radix - 10)));
    else
	return c >= '0' && c <= ('0' - 1 + radix);
}

/* Numbers. We do just do basic matching here. Actual parsing and conversion can
 * be done in the parser. 
 */
static void
match_number (st_lexer *lexer)
{
    /* We don't match any leading '-'. The parser will resolve whether a '-'
     * specifies a negative number or a binary selector
     */
    
    bool negative = false;
    long radix = 10;
    long exponent = 0;
    int k, j, l;
    char *string;
    
    if (lookahead (lexer, 1) == '-') {
	negative = true;
	consume (lexer);
    }

    k = st_input_index (lexer->input);
    
    do {
	match_range (lexer, '0', '9');
    } while (isdigit (lookahead (lexer, 1)));
   
    if (lookahead (lexer, 1) != 'r') {
    
	j = st_input_index (lexer->input);
	goto out1;
	    
    } else {
    
	string = st_input_range (lexer->input, k,
				 st_input_index (lexer->input));
				       			
	radix = strtol (string, NULL, 10);
	st_free (string);
	if (radix < 2 || radix > 36) {
	    raise_error (lexer, ERROR_INVALID_RADIX, lookahead (lexer, 1));
	}

	consume (lexer);
    
    }

    k = st_input_index (lexer->input);

    if (lookahead (lexer, 1) == '-')
	raise_error (lexer, ERROR_NO_VIABLE_ALT_FOR_CHAR, lookahead (lexer, 1));	
	
out1:

    while (is_radix_numeral (radix, lookahead (lexer, 1)))
	consume (lexer);

    if (lookahead (lexer, 1) == '.' && is_radix_numeral (radix, lookahead (lexer, 2))) {
	consume (lexer);

	do {
	    consume (lexer);
	} while (is_radix_numeral (radix, lookahead (lexer, 1)));
    }  

    j = st_input_index (lexer->input);

    if (lookahead (lexer, 1) == 'e') {

	consume (lexer);
	
	l = st_input_index (lexer->input);

	if (lookahead (lexer, 1) == '-' && isdigit (lookahead (lexer, 2)))
	    consume (lexer);

	while (isdigit (lookahead (lexer, 1)))
		consume (lexer);
		
	if (l == st_input_index (lexer->input))
	    goto out2;
	
	string = st_input_range (lexer->input, l,
				 st_input_index (lexer->input));				    
	exponent = strtol (string, NULL, 10);
    }
    
out2:
    
    make_number_token (lexer, radix, exponent,
		       st_input_range (lexer->input, k, j),
		       negative);
}


static void
match_identifier (st_lexer *lexer, bool create_token)
{
    if (g_unichar_isalpha (lookahead (lexer, 1)))
	consume (lexer);
    else {
	raise_error (lexer, ERROR_NO_VIABLE_ALT_FOR_CHAR, lookahead (lexer, 1));
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
		    st_input_range (lexer->input, lexer->start,
				    st_input_index (lexer->input)));
    }
}

static void
match_keyword_or_identifier (st_lexer *lexer, bool create_token)
{
    if (g_unichar_isalpha (lookahead (lexer, 1)))
	consume (lexer);
    else {
	raise_error (lexer, ERROR_NO_VIABLE_ALT_FOR_CHAR, lookahead (lexer, 1));
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

    st_token_type token_type;

    if (lookahead (lexer, 1) == ':' && lookahead (lexer, 2) != '=') {
	consume (lexer);
	token_type = ST_TOKEN_KEYWORD_SELECTOR;
    } else {
	token_type = ST_TOKEN_IDENTIFIER;
    }

    if (create_token) {
	char *text;

	if (token_type == ST_TOKEN_KEYWORD_SELECTOR)
	    text = st_input_range (lexer->input, lexer->start,
				   st_input_index (lexer->input));
	else
	    text = st_input_range (lexer->input, lexer->start,
				   st_input_index (lexer->input));

	make_token (lexer, token_type, text);
    }

}

static void
match_string_constant (st_lexer *lexer)
{
    mark (lexer);

    match (lexer, '\'');

    while (lookahead (lexer, 1) != '\'') {
	consume (lexer);

	if (lookahead (lexer, 1) == ST_INPUT_EOF) {
	    rewind (lexer);
	    raise_error (lexer, ERROR_UNTERMINATED_STRING_LITERAL, lookahead (lexer, 1));
	}
    }

    match (lexer, '\'');

    char *string;

    string = st_input_range (lexer->input,
			     lexer->start + 1,
			     st_input_index (lexer->input) - 1);

    make_token (lexer, ST_TOKEN_STRING_CONST, string);
}

static void
match_comment (st_lexer *lexer)
{
    mark (lexer);

    match (lexer, '"');

    while (lookahead (lexer, 1) != '"') {
	consume (lexer);

	if (lookahead (lexer, 1) == ST_INPUT_EOF) {
	    rewind (lexer);
	    raise_error (lexer, ERROR_UNTERMINATED_COMMENT, lookahead (lexer, 1));
	}
    }

    match (lexer, '"');
    
    if (!lexer->filter_comments) {
	
	char *comment;

	comment = st_input_range (lexer->input,
				  lexer->start + 1,
				  st_input_index (lexer->input) - 1);
    
	make_token (lexer, ST_TOKEN_COMMENT, comment);
    }
    
}

static void
match_tuple_begin (st_lexer *lexer)
{
    match (lexer, '#');
    match (lexer, '(');

    make_token (lexer, ST_TOKEN_TUPLE_BEGIN, st_strdup ("#("));
}

static void
match_binary_selector (st_lexer *lexer, bool create_token)
{
    if (lookahead (lexer, 1) == '-') {
	match (lexer, '-');

	if (is_special_char (lookahead (lexer, 1)))
	    match (lexer, lookahead (lexer, 1));

    } else if (is_special_char (lookahead (lexer, 1))) {
	match (lexer, lookahead (lexer, 1));

	if (is_special_char (lookahead (lexer, 1)))
	    match (lexer, lookahead (lexer, 1));

    } else {
	raise_error (lexer, ERROR_NO_VIABLE_ALT_FOR_CHAR, lookahead (lexer, 1));
    }

    if (create_token) {
	make_token (lexer, ST_TOKEN_BINARY_SELECTOR,
		    st_input_range (lexer->input,
				    lexer->start,
				    st_input_index (lexer->input)));
    }
}

static void
match_symbol_constant (st_lexer *lexer)
{
    match (lexer, '#');

    if (g_unichar_isalpha (lookahead (lexer, 1))) {

	do {
	    match_keyword_or_identifier (lexer, false);
	} while (g_unichar_isalpha (lookahead (lexer, 1)));

    } else if (lookahead (lexer, 1) == '-' || is_special_char (lookahead (lexer, 1))) {
	match_binary_selector (lexer, false);
    } else {
	raise_error (lexer, ERROR_NO_ALT_FOR_POUND, lookahead (lexer, 1));
    }

    // discard #
    char *symbol_text = st_input_range (lexer->input,
					lexer->start + 1,
					st_input_index (lexer->input));

    make_token (lexer, ST_TOKEN_SYMBOL_CONST, symbol_text);
}

static void
match_block_begin (st_lexer *lexer)
{
    match (lexer, '[');

    make_token (lexer, ST_TOKEN_BLOCK_BEGIN, NULL);
}

static void
match_block_end (st_lexer *lexer)
{
    match (lexer, ']');

    make_token (lexer, ST_TOKEN_BLOCK_END, NULL);
}

static void
match_lparen (st_lexer *lexer)
{
    match (lexer, '(');

    make_token (lexer, ST_TOKEN_LPAREN, NULL);
}

static void
match_rparen (st_lexer *lexer)
{
    match (lexer, ')');

    make_token (lexer, ST_TOKEN_RPAREN, NULL);
}

static void
match_char_constant (st_lexer *lexer)
{
    gunichar ch = 0;
    match (lexer, '$');
    
    if (lookahead (lexer, 1) == '\\') {
    
	if (lookahead (lexer, 2) == 't') {
	    ch = '\t';
	    consume (lexer);
	    consume (lexer);
	} else if (lookahead (lexer, 2) == 'f') {
	    ch = '\f';
	    consume (lexer);
	    consume (lexer);
	} else if (lookahead (lexer, 2) == 'n') {
	    ch = '\n';
	    consume (lexer);
	    consume (lexer);
	} else if (lookahead (lexer, 2) == 'r') {
	    ch = '\r';
	    consume (lexer);
	    consume (lexer);
	} else if (isxdigit (lookahead (lexer, 2))) {
	    consume (lexer);
	    int start = st_input_index (lexer->input);	
	
	    do {
		consume (lexer);
	    } while (isxdigit (lookahead (lexer, 1)));
	
	    char *string = st_input_range (lexer->input, start, st_input_index (lexer->input));
	    ch = strtol (string, NULL, 16);
	    st_free (string);

	    if (!g_unichar_validate (ch))
		raise_error (lexer, ERROR_ILLEGAL_CHAR, ch);
	   
	} else {
	    // just match the '\' char then
	    ch = '\\';
	    consume (lexer);
	}
	
    } else if (g_unichar_isgraph (lookahead (lexer, 1))) {
	ch = lookahead (lexer, 1);
	consume (lexer);    
    } else {
	raise_error (lexer, ERROR_INVALID_CHAR_CONST, lookahead (lexer, 1));
    }

    char outbuf[6] = {0};
    g_unichar_to_utf8 (ch, outbuf);
    make_token (lexer, ST_TOKEN_CHARACTER_CONST, st_strdup (outbuf));
}

static void
match_eof (st_lexer *lexer)
{
    match (lexer, ST_INPUT_EOF);

    make_token (lexer, ST_TOKEN_EOF, NULL);
}

static void
match_white_space (st_lexer *lexer)
{
    /* gobble up white space */
    while (true) {
	switch (lookahead (lexer, 1)) {
	case  ' ': case '\r':
	case '\n': case '\t': case '\f':
	    consume (lexer);
	    break;
	default:
	    return;
	}
    }
}

static void
match_colon (st_lexer *lexer)
{
    match (lexer, ':');
    make_token (lexer, ST_TOKEN_COLON, NULL);
}

static void
match_semicolon (st_lexer *lexer)
{
    match (lexer, ';');
    make_token (lexer, ST_TOKEN_SEMICOLON, NULL);
}

static void
match_assign (st_lexer *lexer)
{
    match (lexer, ':');
    match (lexer, '=');
    make_token (lexer, ST_TOKEN_ASSIGN, NULL);
}

static void
match_period (st_lexer *lexer)
{
    match (lexer, '.');
    make_token (lexer, ST_TOKEN_PERIOD, NULL);
}

static void
match_return (st_lexer *lexer)
{
    match (lexer, '^');
    make_token (lexer, ST_TOKEN_RETURN, NULL);
}

/* st_lexer_next_token:
 * lexer: a st_lexer
 *
 * Returns the next matched token from the input stream. Caller takes
 * ownership of returned token.
 *
 * If the end of the input stream is reached, tokens of type ST_TOKEN_EOF
 * will be returned. Similarly, if there are matching errors, then tokens
 * of type ST_TOKEN_INVALID will be returned;
 *
 */
st_token *
st_lexer_next_token (st_lexer *lexer)
{
    st_assert (lexer != NULL);

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

	case ' ': case '\n': case '\r': case '\t': case '\f':
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

	case '+': case '/': case '\\':
	case '*': case '<': case '>': case '=':
	case '@': case '%': case '|': case '&':
	case '?': case '!': case '~': case ',':
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

	    else if (lookahead (lexer, 1) == '-' && isdigit (lookahead (lexer, 2)))
		match_number (lexer);

	    else if (isdigit (lookahead (lexer, 1)))
		match_number (lexer);

	    else if (lookahead (lexer, 1) == '-')
		match_binary_selector (lexer, true);

	    else if (lookahead (lexer, 1) == '#' && lookahead (lexer, 2) == '(')
		match_tuple_begin (lexer);

	    else if (lookahead (lexer, 1) == '#')
		match_symbol_constant (lexer);

	    // match assign or colon
	    else if (lookahead (lexer, 1) == ':' && lookahead (lexer, 2) == '=')
		match_assign (lexer);

	    else if (lookahead (lexer, 1) == ':')
		match_colon (lexer);

	    else
		raise_error (lexer, ERROR_ILLEGAL_CHAR, lookahead (lexer, 1));
	}

      out:

	// we return the matched token or an invalid token on error
	if (lexer->token_matched || lexer->failed)
	    return lexer->token;
	else
	    continue;

    }
}

static void
lexer_initialize (st_lexer *lexer, st_input *input)
{
    lexer->input = input;
    lexer->token = NULL;
    lexer->line = 1;
    lexer->column = 1;
    lexer->start = -1;
    lexer->error_code = 0;
    lexer->failed = FALSE;
    lexer->filter_comments = true;

    lexer->allocated_tokens = NULL;
}

st_lexer *
st_lexer_new (const char *string)
{
    st_lexer *lexer;
    st_input *input;

    st_assert (string != NULL);

    lexer = st_new0 (st_lexer);
    input = st_input_new (string);
    if (!input)
	return NULL;

    lexer_initialize (lexer, input);

    return lexer;
}

st_lexer *
st_lexer_new_ucs4 (const wchar_t *string)
{
    st_lexer *lexer;

    st_assert (string != NULL);

    lexer = st_new0 (st_lexer);
    lexer_initialize (lexer, st_input_new_ucs4 (string)); 

    return lexer;
}

void
destroy_token (st_token *token)
{
    if (token->type != ST_TOKEN_NUMBER_CONST) {
	st_free (token->text);

    }

    st_free (token);
}

void
st_lexer_destroy (st_lexer *lexer)
{
    st_assert (lexer != NULL);

    st_input_destroy (lexer->input);

    st_list_foreach (lexer->allocated_tokens, (st_list_foreach_func) destroy_token);
    st_list_destroy (lexer->allocated_tokens);

    st_free (lexer);
}

st_token_type
st_token_get_type (st_token *token)
{
    st_assert (token != NULL);

    return token->type;
}

char *
st_token_get_text (st_token *token)
{
    st_assert (token != NULL);

    return token->text;
}

st_uint
st_token_get_line (st_token *token)
{
    st_assert (token != NULL);

    return token->line;
}

st_uint
st_token_get_column (st_token *token)
{
    st_assert (token != NULL);

    return token->column;
}

st_uint
st_lexer_error_line (st_lexer *lexer)
{
    st_assert (lexer != NULL);

    return lexer->error_line;
}

st_uint
st_lexer_error_column (st_lexer *lexer)
{
    st_assert (lexer != NULL);

    return lexer->error_column;
}

gunichar
st_lexer_error_char (st_lexer *lexer)
{
    st_assert (lexer != NULL);

    return lexer->error_char;
}

char *
st_lexer_error_message (st_lexer *lexer)
{
    st_assert (lexer != NULL);

    static const char *msgformats[] = {
	"mismatched character \\%04X",
	"no viable alternative for character \\%04X",
	"illegal character \\%04X",
	"unterminated comment",
	"unterminated string literal",
        "invalid radix for number",
	"non-whitespace character expected after '$'",
	"expected '(' after '#'",
    };

    switch (lexer->error_code) {
    
    case ERROR_UNTERMINATED_COMMENT:
    case ERROR_UNTERMINATED_STRING_LITERAL:
    case ERROR_INVALID_RADIX:
    case ERROR_INVALID_CHAR_CONST:
    case ERROR_NO_ALT_FOR_POUND:

	return st_strdup_printf (msgformats[lexer->error_code]);

    case ERROR_MISMATCHED_CHAR:
    case ERROR_NO_VIABLE_ALT_FOR_CHAR:
    case ERROR_ILLEGAL_CHAR:

	return st_strdup_printf (msgformats[lexer->error_code], lexer->error_char);
	
    default:
	return NULL;
    }
}

st_token *
st_lexer_current_token (st_lexer *lexer)
{
    return lexer->token;
}

void
st_lexer_filter_comments (st_lexer *lexer, bool filter)
{
    lexer->filter_comments = filter;
}

bool
st_number_token_negative (st_token *token)
{
    return token->negative;
}

char *
st_number_token_number (st_token *token)
{
    return token->number;
}

st_uint
st_number_token_radix (st_token *token)
{
    return token->radix;
}

int
st_number_token_exponent (st_token *token)
{
    return token->exponent;
}
