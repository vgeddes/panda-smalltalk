

#ifndef __ST_INTERPRETER_H__
#define __ST_INTERPRETER_H__

#include <st-types.h>
#include <st-context.h>
#include <setjmp.h>

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

    st_oop  message_receiver;
    st_oop  message_selector;
    guint   message_argcount;
    
    bool    success;

    jmp_buf main_loop;

} STExecutionState;


#define ST_STACK_POP(es)          ((es)->stack[--(es)->sp])
#define ST_STACK_PUSH(es, oop)    ((es)->stack[(es)->sp++] = oop)
#define ST_STACK_PEEK(es)         ((es)->stack[(es)->sp-1])
#define ST_STACK_UNPOP(es, count) ((es)->sp += count)


void st_interpreter_main               (STExecutionState *es);

void st_interpreter_initialize         (STExecutionState *es);

void st_interpreter_set_active_context (STExecutionState *es, st_oop context);

void st_interpreter_send_selector      (STExecutionState *es, st_oop selector, guint argcount);

void st_interpreter_execute_method     (STExecutionState *es, st_oop method);

st_oop st_interpreter_lookup_method (STExecutionState *es, st_oop klass);

INLINE void
st_interpreter_set_success (STExecutionState *es, bool success)
{
    es->success = es->success && success;
}


#endif //* __ST_INTERPRETER_H__ */
