INLINE oop
goo_meta_allocate (GooMeta *meta)
{
	return GOO_META_CLASS (meta)->allocate ();
}

INLINE bool
goo_meta_equal (GooMeta *meta, oop object, oop another)
{
	return GOO_META_CLASS (meta)->equal (object, another);
}

INLINE bool
goo_meta_is_association (GooMeta *meta)
{
	return GOO_META_CLASS (meta)->is_association ();
}

INLINE bool
goo_meta_is_class (GooMeta *meta)
{
	return GOO_META_CLASS (meta)->is_class ();
}

INLINE bool
goo_meta_is_symbol (GooMeta *meta)
{
	return GOO_META_CLASS (meta)->is_symbol ();
}

INLINE bool
goo_meta_is_compiled_method (GooMeta *meta)
{
	return GOO_META_CLASS (meta)->is_compiled_method ();
}

INLINE bool
goo_meta_is_compiled_block (GooMeta *meta)
{
	return GOO_META_CLASS (meta)->is_compiled_block ();
}

INLINE bool
goo_meta_is_block_closure (GooMeta *meta)
{
	return GOO_META_CLASS (meta)->is_block_closure ();
}

INLINE bool
goo_meta_is_indexable (GooMeta *meta)
{
	return GOO_META_CLASS (meta)->is_indexable ();
}

INLINE bool
goo_meta_is_variable (GooMeta *meta)
{
	return GOO_META_CLASS (meta)->is_variable ();
}

INLINE bool
goo_meta_is_variable_byte (GooMeta *meta);
{
	return GOO_META_CLASS (meta)->is_variable_byte ();
}
