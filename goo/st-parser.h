
#ifndef __ST_PARSER_H__
#define __ST_PARSER_H__


#include <st-ast.h>

typedef struct st_parser_t st_parser_t;

st_node_method_t *st_parser_parse (const char *source);




#endif // __ST_PARSER_H__
