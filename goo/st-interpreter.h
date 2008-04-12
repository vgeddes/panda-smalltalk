

#ifndef __ST_INTERPRETER_H__
#define __ST_INTERPRETER_H__

#include <st-types.h>
#include <st-context.h>

typedef struct {

    st_oop  context;

    st_oop  receiver;
    st_oop  method;  
    st_oop *literals;
    guchar *bytecodes;
    guint   sp;
    guint   ip;
    
    st_oop *stack;

    bool success;

} STInterpreter;

#define ST_STACK_POP(state)      (state->stack[state->sp--])
#define ST_STACK_PUSH(state, oop) (state->stack[++state->sp] = (oop))
#define ST_STACK_PEEK(state)      (state->stack[state->sp])


void st_interpreter_main (void);

#endif //* __ST_INTERPRETER_H__ */
