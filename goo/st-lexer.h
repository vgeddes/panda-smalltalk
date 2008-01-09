/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-lexer.h - Lexical Analyzer 
 *
 * Copyright (C) 2008 Vincent Geddes
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __ST_LEXER_H__
#define __ST_LEXER_H__

#include <glib.h>

typedef struct _st_lexer_t        st_lexer_t;
typedef struct _st_lexer_token_t  st_lexer_token_t;
typedef struct _st_lexer_error_t  st_lexer_error_t;

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
    ST_TOKEN_ARRAY_BEGIN,
    ST_TOKEN_IDENTIFIER,
    ST_TOKEN_CHAR_CONST, 
    ST_TOKEN_STRING_CONST,
    ST_TOKEN_NUMBER_CONST,
    ST_TOKEN_SYMBOL_CONST,
    ST_TOKEN_COMMENT, 
    ST_TOKEN_BINARY_SELECTOR,
    ST_TOKEN_KEYWORD_SELECTOR,   
    ST_TOKEN_EOF

} st_lexer_token_type_t;

st_lexer_t        *st_lexer_new         (const char *text);

st_lexer_token_t  *st_lexer_next_token  (st_lexer_t *lexer);

st_lexer_error_t  *st_lexer_last_error  (st_lexer_t *lexer);

void               st_lexer_destroy     (st_lexer_t *lexer);



st_lexer_token_type_t    st_lexer_token_type   (st_lexer_token_t *token);

char           *st_lexer_token_text   (st_lexer_token_t *token);

guint           st_lexer_token_line   (st_lexer_token_t *token);

guint           st_lexer_token_column (st_lexer_token_t *token);


gunichar     st_lexer_error_char     (st_lexer_error_t *error); 

guint        st_lexer_error_line     (st_lexer_error_t *error);

guint        st_lexer_error_column   (st_lexer_error_t *error);

char        *st_lexer_error_message  (st_lexer_error_t *error);

#endif /* __ST_LEXER_H__ */
