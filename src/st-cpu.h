

#ifndef __ST_INTERPRETER_H__
#define __ST_INTERPRETER_H__

#include <st-types.h>
#include <setjmp.h>

/* cache size must be a power of 2 */
#define ST_METHOD_CACHE_SIZE      1024
#define ST_METHOD_CACHE_MASK      (ST_METHOD_CACHE_SIZE - 1)
#define ST_METHOD_CACHE_HASH(k,s) ((k) ^ (s))

typedef struct st_method_cache
{
    st_oop class;
    st_oop selector;
    st_oop method;

} st_method_cache;

struct st_cpu {

    st_oop  context;
    st_oop  receiver;
    st_oop  method;

    st_uchar *bytecode;

    st_oop *literals;
    st_oop *temps;

    st_oop *stack;

    st_oop  lookup_class;

    st_oop  message_receiver;
    st_oop  message_selector;
    int     message_argcount;
    
    st_oop  new_method;
    bool    success;

    st_uint ip;
    st_uint sp;

    jmp_buf main_loop;

    st_method_cache method_cache[ST_METHOD_CACHE_SIZE];

    st_oop globals[34];
    st_oop selectors[24];

} __cpu;

#define ST_STACK_POP(cpu)          (cpu->stack[--cpu->sp])
#define ST_STACK_PUSH(cpu, oop)    (cpu->stack[cpu->sp++] = oop)
#define ST_STACK_PEEK(cpu)         (cpu->stack[cpu->sp-1])
#define ST_STACK_UNPOP(cpu, count) (cpu->sp += count)

void   st_cpu_main               (void);
void   st_cpu_initialize         (void);
void   st_cpu_set_active_context (st_oop context);
void   st_cpu_execute_method     (void);
st_oop st_cpu_lookup_method      (st_oop class);

void   st_cpu_clear_caches (void);

#endif /* __ST_INTERPRETER_H__ */
