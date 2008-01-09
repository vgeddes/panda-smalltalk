

#include "goo-mini.h"

#include <glib.h>

/* foo table */

typedef struct _GooFoo GooFoo;

struct _GooFoo
{
	GooMiniBase base_table;

	void (* foo) (GooFoo *table);
	
};

#define GOO_MINI_TYPE_FOO         (goo_foo_get_type ())
#define GOO_MINI_FOO_TABLE(table) ((GooFoo *) table)

GooMiniType goo_foo_get_type ();

GOO_MINI_DEFINE_TYPE (GooFoo, goo_foo, GOO_MINI_TYPE_BASE);


static void
foo_handler (GooFoo *table)
{
	g_debug (G_STRFUNC);
}

static void
goo_foo_table_init (GooFoo *table)
{
	table->foo = foo_handler;
}


/* bar table */

typedef struct _GooBar GooBar;

struct _GooBar
{
	GooFoo parent_table;

	void (* bar) (GooBar *table);
	
};

#define GOO_MINI_TYPE_BAR         (goo_bar_get_type ())
#define GOO_MINI_BAR_TABLE(table) ((GooBar *) table)

GooMiniType goo_bar_get_type ();

GOO_MINI_DEFINE_TYPE (GooBar, goo_bar, GOO_MINI_TYPE_FOO);

static void
bar_foo_handler (GooFoo *table)
{
	g_debug (G_STRFUNC);
	GOO_MINI_FOO_TABLE (goo_bar_parent_table)->foo (table);
}

static void
bar_handler (GooBar *table)
{
	g_debug (G_STRFUNC);
}

static void
goo_bar_table_init (GooBar *table)
{

	GooFoo *foo_table = GOO_MINI_FOO_TABLE (table);
		
	foo_table->foo = bar_foo_handler;
	
	table->bar =  bar_handler;
}



int
main (int argc, char *argv[])
{
	GooFoo *table = (GooFoo *) goo_mini_type_get_table (GOO_MINI_TYPE_BAR);

	table->foo (table);

	return 0;
}
