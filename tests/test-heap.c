

#include <st-compiler.h>
#include <st-universe.h>
#include <st-system.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

int
main (int argc, char *argv[])
{
    st_pointer start;
    st_uint size;

    st_heap *heap;

    heap = st_heap_new (1024 * 1024 * 1024);
    if (!heap)
	abort ();

    st_assert (st_heap_grow (heap, 512 * 1024 * 1024));
    st_assert (st_heap_grow (heap, 256 * 1024 * 1024));
    st_assert (st_heap_shrink (heap, 512 * 1024 * 1024));
   
    while (true)
	sleep (1);
    
    return 0;
}

