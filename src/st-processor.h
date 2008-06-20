

#ifndef __ST_INTERPRETER_H__
#define __ST_INTERPRETER_H__

#include <st-types.h>
#include <st-context.h>
#include <setjmp.h>

/* cache size must be a power of 2 */
#define ST_METHOD_CACHE_SIZE      1024
#define ST_METHOD_CACHE_MASK      (ST_METHOD_CACHE_SIZE - 1)
#define ST_METHOD_CACHE_HASH(k,s) ((k ^ s) & ST_METHOD_CACHE_MASK)

#define ST_METHOD_IS_CACHED

typedef struct st_method_cache
{
    st_oop class;
    st_oop selector;
    st_oop method;

} st_method_cache;

typedef struct st_processor {

    st_oop  context;
    st_oop  receiver;
    st_oop  method;

    st_uchar *bytecode;

    st_oop *literals;
    st_oop *temps;

    st_oop *stack;
    st_uint   sp;
    st_uint   ip;

    st_oop  message_receiver;
    st_oop  message_selector;
    int     message_argcount;
    
    bool    success;

    jmp_buf main_loop;

    st_method_cache method_cache[ST_METHOD_CACHE_SIZE];

} st_processor;


#define ST_STACK_POP(processor)          ((processor)->stack[--(processor)->sp])
#define ST_STACK_PUSH(processor, oop)    ((processor)->stack[(processor)->sp++] = oop)
#define ST_STACK_PEEK(processor)         ((processor)->stack[(processor)->sp-1])
#define ST_STACK_UNPOP(processor, count) ((processor)->sp += count)

void   st_processor_main               (st_processor *processor);
void   st_processor_initialize         (st_processor *processor);
void   st_processor_set_active_context (st_processor *processor, st_oop context);
void   st_processor_send_selector      (st_processor *processor, st_oop selector, st_uint argcount);
void   st_processor_execute_method     (st_processor *processor, st_oop method);
st_oop st_processor_lookup_method      (st_processor *processor, st_oop class);

void   st_processor_clear_caches (st_processor *processor);

#endif /* __ST_INTERPRETER_H__ */
