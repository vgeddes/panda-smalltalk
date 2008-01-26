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

#define ST_NUMBER_TOKEN(token) ((STNumberToken *) token)

typedef struct STLexer        STLexer;
typedef struct STToken        STToken;
typedef struct STNumberToken  STNumberToken;

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


STLexer       *st_lexer_new           (const char *text);

STToken       *st_lexer_next_token    (STLexer *lexer);

STToken       *st_lexer_current_token (STLexer *lexer);

void              st_lexer_destroy       (STLexer *lexer);

gunichar          st_lexer_error_char    (STLexer *lexer);

guint             st_lexer_error_line    (STLexer *lexer);

guint             st_lexer_error_column  (STLexer *lexer);

char *            st_lexer_error_message (STLexer *lexer);


STTokenType       st_token_type   (STToken *token);

char             *st_token_text   (STToken *token);

guint             st_token_line   (STToken *token);

guint             st_token_column (STToken *token);


guint             st_number_token_radix    (STNumberToken *token);

int               st_number_token_exponent (STNumberToken *token);



#endif /* __ST_LEXER_H__ */
