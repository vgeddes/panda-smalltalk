/*
 * st-lexer.h
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

#ifndef __ST_LEXER_H__
#define __ST_LEXER_H__

#include <glib.h>
#include <stdbool.h>
#include <st-types.h>

typedef struct STLexer    STLexer;
typedef struct STToken    STToken;

typedef enum
{
    ST_TOKEN_INVALID,
    ST_TOKEN_LPAREN,
    ST_TOKEN_RPAREN,
    ST_TOKEN_BLOCK_BEGIN,
    ST_TOKEN_BLOCK_END,
    ST_TOKEN_COMMA,
    ST_TOKEN_SEMICOLON,
    ST_TOKEN_PERIOD,
    ST_TOKEN_RETURN,
    ST_TOKEN_COLON,
    ST_TOKEN_ASSIGN,
    ST_TOKEN_TUPLE_BEGIN,
    ST_TOKEN_IDENTIFIER,
    ST_TOKEN_CHARACTER_CONST,
    ST_TOKEN_STRING_CONST,
    ST_TOKEN_NUMBER_CONST,
    ST_TOKEN_SYMBOL_CONST,
    ST_TOKEN_COMMENT,
    ST_TOKEN_BINARY_SELECTOR,
    ST_TOKEN_KEYWORD_SELECTOR,
    ST_TOKEN_EOF
    
} STTokenType;

 
STLexer       *st_lexer_new           (const char *string);

STLexer       *st_lexer_new_ucs4      (const wchar_t *string);

STToken       *st_lexer_next_token    (STLexer *lexer);

STToken       *st_lexer_current_token (STLexer *lexer);

void           st_lexer_destroy       (STLexer *lexer);

gunichar       st_lexer_error_char    (STLexer *lexer);

st_uint        st_lexer_error_line    (STLexer *lexer);

st_uint         st_lexer_error_column  (STLexer *lexer);

char          *st_lexer_error_message (STLexer *lexer);

void           st_lexer_filter_comments (STLexer *lexer, bool filter);


STTokenType    st_token_type   (STToken *token);

char          *st_token_text   (STToken *token);

st_uint          st_token_line   (STToken *token);

st_uint          st_token_column (STToken *token);

bool           st_number_token_negative (STToken *token);

char          *st_number_token_number   (STToken *token);

st_uint          st_number_token_radix    (STToken *token);

int            st_number_token_exponent (STToken *token);

#endif /* __ST_LEXER_H__ */
