

#include "st-system.h"
#include "st-utils.h"

#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static st_pointer
st_mmap_anon (st_pointer address, st_uint length,
	      int protect, int flags)
{
    /* Wrapper for mmap(), with the default flags as MAP_PRIVATE | MAP_ANONYMOUS.
     * Returns NULL on failure.
     */
    st_pointer result;

    result = mmap (address, length, protect,
		   flags | MAP_PRIVATE | MAP_ANONYMOUS,
		   0, 0);

    if (result == (st_pointer) -1) {
	fprintf (stderr, "panda: error: %s\n", strerror (errno));
	return NULL;
    }

    return result;
}

st_pointer
st_system_reserve_memory (st_pointer addr, st_uint size)
{
    /* Reserves a virtual memory region without actually allocating any 
     * storage in physical memory or swap space.
     */
    int flags = 0;

    flags = MAP_NORESERVE;
    if (addr)
	flags |= MAP_FIXED;

    return st_mmap_anon (addr, size, PROT_NONE, flags);
}

st_pointer
st_system_commit_memory  (st_pointer addr, st_uint size)
{
    /* Allocates storage in physical memory or swap space.
     */
    int flags = 0;

    if (munmap (addr, size) < 0) {
	fprintf (stderr, "panda: error: %s\n", strerror (errno));
	return false;
    }
    
    if (addr)
	flags |= MAP_FIXED;

    return st_mmap_anon (addr, size, PROT_READ | PROT_WRITE, flags);
}

st_pointer
st_system_decommit_memory  (st_pointer addr, st_uint size)
{
    /* Deallocates any storage but ensures that the given region is still reserved
     */
    int flags = 0;

    st_assert (addr != NULL);
    if (munmap (addr, size) < 0) {
	fprintf (stderr, "panda: error: %s\n", strerror (errno));
	return false;
    }

    flags = MAP_NORESERVE;
    if (addr)
	flags |= MAP_FIXED;

    return st_mmap_anon (addr, size, PROT_NONE, flags);
}

void
st_system_release_memory (st_pointer addr, st_uint size)
{
    /* destroys any virtual memory mappings within the given region
     */
    st_assert (addr != NULL);
    if (munmap (addr, size) < 0) {
	fprintf (stderr, "panda: error: %s\n", strerror (errno));
	st_assert_not_reached ();
    }
}

st_uint
st_system_pagesize (void)
{
    return getpagesize ();
}
