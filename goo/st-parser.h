
#ifndef __ST_PARSER_H__
#define __ST_PARSER_H__

#include <st-lexer.h>

#include <st-ast.h>


st_node_method_t *st_parser_parse (st_lexer_t *lexer);


#endif // __ST_PARSER_H__
