
#ifndef __ST_PARSER_H__
#define __ST_PARSER_H__

#include <st-lexer.h>

#include <st-ast.h>


STNode *st_parser_parse (STLexer *lexer);


#endif // __ST_PARSER_H__
