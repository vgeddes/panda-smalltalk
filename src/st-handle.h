

#ifndef __ST_HANDLE_H__
#define __ST_HANDLE_H__

#include <st-types.h>

struct st_handle
{
    struct st_header header;
    
    uintptr_t value;
};

#define  ST_HANDLE(oop)       ((struct st_handle *) st_detag_pointer (oop))
#define  ST_HANDLE_VALUE(oop) (ST_HANDLE (oop)->value)

st_oop   st_handle_allocate (st_oop class);

#endif /* __ST_HANDLE_H__ */
