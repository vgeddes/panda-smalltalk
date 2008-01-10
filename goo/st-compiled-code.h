/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-compiled-code.h
 *
 * Copyright (C) 2008 Vincent Geddes <vgeddes@gnome.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __ST_COMPILED_CODE_H__
#define __ST_COMPILED_CODE_H__

#include <st-types.h>
#include <st-mini.h>
#include <st-heap-object.h>
#include <glib.h>

typedef struct
{
    st_header_t _header;

    st_oop_t header;
    st_oop_t bytecode;
    st_oop_t literals;

} st_compiled_code_t;

typedef struct
{
    st_compiled_code_t header;

} st_compiled_method_t;

typedef struct
{
    st_compiled_code_t header;

    st_oop_t method;

} st_compiled_block_t;


INLINE st_oop_t st_compiled_code_header (st_oop_t code);

INLINE void st_compiled_code_set_header (st_oop_t code, st_oop_t header);

INLINE int st_compiled_code_temp_count (st_oop_t code);

INLINE int st_compiled_code_arg_count (st_oop_t code);

INLINE int st_compiled_code_primitive_index (st_oop_t code);

INLINE int st_compiled_code_flags (st_oop_t code);

INLINE void st_compiled_code_set_flags (st_oop_t code, int flags);

INLINE void st_compiled_code_set_primitive_index (st_oop_t code, int index);

INLINE void st_compiled_code_set_temp_count (st_oop_t code, int count);

INLINE void st_compiled_code_set_arg_count (st_oop_t code, int count);

INLINE void st_compiled_code_set_bytecodes (st_oop_t code, st_oop_t bytecode);

INLINE void st_compiled_code_set_literals (st_oop_t code, st_oop_t literals);


INLINE void st_compiled_block_set_method (st_oop_t block, st_oop_t method);

INLINE st_oop_t st_compiled_block_method (st_oop_t block);

st_vtable_t *st_compiled_block_vtable (void);


st_vtable_t *st_compiled_method_vtable (void);





/* inline definitions */

enum
{
    st_flag_bits = 3,
    st_arg_count_bits = 8,
    st_temp_count_bits = 8,
    st_primitive_bits = 8,
};

enum
{
    st_primitive_shift = ST_TAG_SIZE,
    st_temp_count_shift = st_primitive_bits + st_primitive_shift,
    st_arg_count_shift = st_temp_count_bits + st_temp_count_shift,
    st_flag_shift = st_arg_count_bits + st_arg_count_shift,
};

enum
{
    st_primitive_mask = ST_NTH_MASK (st_primitive_bits),
    st_primitive_mask_in_place = st_primitive_bits << st_primitive_shift,
    st_temp_count_mask = ST_NTH_MASK (st_temp_count_bits),
    st_temp_count_mask_in_place = st_temp_count_mask << st_temp_count_shift,
    st_arg_count_mask = ST_NTH_MASK (st_arg_count_bits),
    st_arg_count_mask_in_place = st_arg_count_mask << st_arg_count_shift,
    st_flag_mask = ST_NTH_MASK (st_flag_bits),
    st_flag_mask_in_place = st_flag_mask << st_flag_shift,
};

#define _ST_COMPILED_CODE(oop)   ((st_compiled_code_t *)   (ST_POINTER (oop)))
#define _ST_COMPILED_METHOD(oop) ((st_compiled_method_t *) (ST_POINTER (oop)))
#define _ST_COMPILED_BLOCK(oop)  ((st_compiled_block_t *)  (ST_POINTER (oop)))


INLINE st_oop_t
st_compiled_code_header (st_oop_t code)
{
    return _ST_COMPILED_CODE (code)->header;
}

INLINE void
st_compiled_code_set_header (st_oop_t code, st_oop_t header)
{
    _ST_COMPILED_CODE (code)->header = header;
}

INLINE int
st_compiled_code_temp_count (st_oop_t code)
{
    return (st_compiled_code_header (code) >> st_temp_count_shift) & st_temp_count_mask;
}

INLINE int
st_compiled_code_arg_count (st_oop_t code)
{
    return (st_compiled_code_header (code) >> st_arg_count_shift) & st_arg_count_mask;
}

INLINE int
st_compiled_code_primitive_index (st_oop_t code)
{
    return (st_compiled_code_header (code) >> st_primitive_shift) & st_primitive_mask;
}

INLINE int
st_compiled_code_flags (st_oop_t code)
{
    return (st_compiled_code_header (code) >> st_flag_shift) & st_flag_mask;
}

INLINE void
st_compiled_code_set_flags (st_oop_t code, int flags)
{
    _ST_COMPILED_CODE (code)->header = (st_compiled_code_header (code) & ~st_flag_mask_in_place) |
	((flags & st_flag_mask) << st_flag_shift);
}

INLINE void
st_compiled_code_set_primitive_index (st_oop_t code, int index)
{
    _ST_COMPILED_CODE (code)->header =
	(st_compiled_code_header (code) & ~st_primitive_mask_in_place) |
	((index & st_primitive_mask) << st_primitive_shift);
}

INLINE void
st_compiled_code_set_temp_count (st_oop_t code, int count)
{
    _ST_COMPILED_CODE (code)->header =
	(st_compiled_code_header (code) & ~st_temp_count_mask_in_place) |
	((count & st_temp_count_mask) << st_temp_count_shift);
}

INLINE void
st_compiled_code_set_arg_count (st_oop_t code, int count)
{
    _ST_COMPILED_CODE (code)->header =
	(st_compiled_code_header (code) & ~st_arg_count_mask_in_place) |
	((count & st_arg_count_mask) << st_arg_count_shift);
}

INLINE st_oop_t
st_compiled_code_literals (st_oop_t code)
{
    return _ST_COMPILED_CODE (code)->literals;
}

INLINE st_oop_t
st_compiled_code_bytecodes (st_oop_t code)
{
    return _ST_COMPILED_CODE (code)->bytecode;
}

INLINE void
st_compiled_code_set_bytecodes (st_oop_t code, st_oop_t bytecode)
{
    _ST_COMPILED_CODE (code)->bytecode = bytecode;
}

INLINE void
st_compiled_code_set_literals (st_oop_t code, st_oop_t literals)
{
    _ST_COMPILED_CODE (code)->literals = literals;
}

/* CompiledBlock */

INLINE void
st_compiled_block_set_method (st_oop_t block, st_oop_t method)
{
    _ST_COMPILED_BLOCK (block)->method = method;
}

INLINE st_oop_t
st_compiled_block_method (st_oop_t block)
{
    return _ST_COMPILED_BLOCK (block)->method;
}


#endif /* __ST_COMPILED_CODE_H__ */
