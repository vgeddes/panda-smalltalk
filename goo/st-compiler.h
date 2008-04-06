


#ifndef __ST_COMPILER_H__
#define __ST_COMPILER_H__

#include <st-types.h>
#include <st-lexer.h>
#include <st-node.h>
#include <glib.h>

#define ST_COMPILATION_ERROR st_compilation_error_quark ()
GQuark st_compilation_error_quark (void);

typedef enum
{
    ST_COMPILATION_ERROR_FAILED,
} STCompilationError;


bool st_compile_string (st_oop klass, const char *string, GError **error);



void st_file_in (const char *filename);


/* internal methods exposed for unit testing */
STNode *st_parser_parse     (STLexer *lexer, bool is_filein, GError **error);
STNode *st_parse_expression (STLexer *lexer, GError **error);
st_oop  st_generate_method  (st_oop klass, STNode *node, GError **error);
void    st_print_method     (st_oop method);


#endif /* __ST_COMPILER_H__ */
