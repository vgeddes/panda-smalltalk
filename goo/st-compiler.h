


#ifndef __ST_COMPILER_H__
#define __ST_COMPILER_H__

#include <st-types.h>
#include <st-utils.h>
#include <st-lexer.h>
#include <st-node.h>
#include <glib.h>

#define ST_COMPILER_ERROR   4

bool    st_compile_string   (st_oop      klass,
			     const char *string,
			     STError   **error);

void    st_compile_file_in  (const char *filename);

STNode *st_parser_parse     (STLexer *lexer,
			     STError **error);

st_oop  st_generate_method  (st_oop    klass,
			     STNode   *node,
			     STError **error);

void    st_print_method     (st_oop method);



/* bytecodes */ 
typedef enum
{
    PUSH_TEMP = 1,
    PUSH_INSTVAR,
    PUSH_LITERAL_CONST,
    PUSH_LITERAL_VAR,

    STORE_LITERAL_VAR,
    STORE_TEMP,
    STORE_INSTVAR,

    STORE_POP_LITERAL_VAR,
    STORE_POP_TEMP,
    STORE_POP_INSTVAR,

    PUSH_SELF,
    PUSH_NIL,
    PUSH_TRUE,
    PUSH_FALSE,

    RETURN_STACK_TOP,
    BLOCK_RETURN,

    POP_STACK_TOP,
    DUPLICATE_STACK_TOP,

    PUSH_ACTIVE_CONTEXT,

    BLOCK_COPY,

    JUMP_TRUE,
    JUMP_FALSE,
    JUMP,

    SEND,        /* B, B (arg count), B (selector index) */ 
    SEND_SUPER,

    SEND_PLUS,
    SEND_MINUS,
    SEND_LT,
    SEND_GT,
    SEND_LE,
    SEND_GE,
    SEND_EQ,
    SEND_NE,
    SEND_MUL,
    SEND_DIV,
    SEND_MOD,
    SEND_BITSHIFT,
    SEND_BITAND,
    SEND_BITOR,
    SEND_BITXOR,

    SEND_AT,
    SEND_AT_PUT,
    SEND_SIZE,
    SEND_VALUE,
    SEND_VALUE_ARG,
    SEND_IDENTITY_EQ,
    SEND_CLASS,
    SEND_NEW,
    SEND_NEW_ARG,

} Code;

#endif /* __ST_COMPILER_H__ */
