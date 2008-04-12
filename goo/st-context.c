

#include "st-context.h"
#include "st-object.h"
#include "st-universe.h"

ST_DEFINE_VTABLE (st_method_context, st_heap_object_vtable ());

st_oop
st_method_context_new (guint stack_size)
{
    st_oop context;

    context = st_allocate_object (ST_TYPE_SIZE (STMethodContext) + stack_size);
    st_object_initialize_header (context, st_method_context_class);

    ST_CONTEXT_PART_SENDER (context) = st_nil;
    ST_CONTEXT_PART_METHOD (context) = st_nil;
    
    ST_CONTEXT_PART_IP (context) = st_nil;
    ST_CONTEXT_PART_SP (context) = ST_METHOD_CONTEXT_STACK (context);

    ST_METHOD_CONTEXT_RECEIVER (context) = st_nil;

    return context;
}

static void
st_method_context_vtable_init (STVTable *table)
{


}
