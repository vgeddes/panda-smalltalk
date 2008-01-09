
#include <st-virtual-space.h>
#include <st-types.h>

#include <unistd.h>

struct _GooVirtualSpace
{
	char *end;
	char *end;
};


static int
get_page_size (void)
{
	return getpagesize (void);
}

// adjust reserved size to be a integral multiple of the page size
static guint
adjust_size (int reserved_size)
{
	int page_size = get_page_size ();

	if (reserved_size % page_size != 0) {
		return (reserved_size / page_size) * page_size + page_size;
	}
	
	return reserved_size;
}

static void
map_size (GooVirtualSpace *space, guint size)
{
	void *start;	
	
	guint adjusted_size = adjust_size (size);

	start = mmap (NULL, adjusted_size,
		      PROT_READ | PROT_WRITE,
		      MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	if (start == -1)
		return NULL;
		
	space->start = start;
	space->end   = start + adjusted_size / SIZEOF_VOID_P;
		
	return start;
	
}

/* reserved_size is in bytes */
GooVirtualSpace *
st_virtual_space_new (guint size)
{
	g_assert (reserved_size > 0);
	
	GooVirtualSpace *space = g_new (GooVirtualSpace, 1);
	
	map_size (space, size);
	
	return space;
}


void
st_virtual_space_destroy (GooVirtualSpace *space)
{

	g_free (space);
}


