/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-lexer.h
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

#ifndef __ST_LEXER_H__
#define __ST_LEXER_H__

#include <glib.h>

typedef struct _st_lexer_t st_lexer_t;
typedef struct _st_lexer_token_t st_lexer_token_t;
typedef struct _st_lexer_error_t st_lexer_error_t;

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

st_lexer_t *st_lexer_new (const char *text);

st_lexer_token_t *st_lexer_next_token (st_lexer_t *lexer);

st_lexer_error_t *st_lexer_last_error (st_lexer_t *lexer);

void st_lexer_destroy (st_lexer_t *lexer);



st_lexer_token_type_t st_lexer_token_type (st_lexer_token_t *token);

char *st_lexer_token_text (st_lexer_token_t *token);

guint st_lexer_token_line (st_lexer_token_t *token);

guint st_lexer_token_column (st_lexer_token_t *token);


gunichar st_lexer_error_char (st_lexer_error_t *error);

guint st_lexer_error_line (st_lexer_error_t *error);

guint st_lexer_error_column (st_lexer_error_t *error);

char *st_lexer_error_message (st_lexer_error_t *error);

#endif /* __ST_LEXER_H__ */
