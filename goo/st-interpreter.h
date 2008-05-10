

#ifndef __ST_INTERPRETER_H__
#define __ST_INTERPRETER_H__

#include <st-types.h>
#include <st-context.h>

typedef struct {

    st_oop  context;
    st_oop  receiver;
    st_oop  method;

    guchar *bytecode;

    st_oop *literals;
    st_oop *temps;

    st_oop *stack;
    guint   sp;
    guint   ip;

    /* information on last message send */
    st_oop  msg_receiver;
    st_oop  msg_selector;
    guint   msg_argcount;
    
    /* primitives error control */
    bool    success;

} STExecutionState;


#define ST_STACK_POP(es)          (es->stack[--es->sp])
#define ST_STACK_PUSH(es, oop)    (es->stack[es->sp++] = oop)
#define ST_STACK_PEEK(es)         (es->stack[es->sp-1])
#define ST_STACK_UNPOP(es, count) (es->sp += count)

void st_interpreter_main (void);

void st_interpreter_set_active_context (STExecutionState *es, st_oop context);

INLINE void
st_interpreter_set_success (STExecutionState *es, bool success)
{
    es->success = es->success && success;
}


#endif //* __ST_INTERPRETER_H__ */
