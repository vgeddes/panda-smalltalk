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
#include <st-vtable.h>
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
    st_compiled_code_t parent;

} st_compiled_method_t;

typedef struct
{
    st_compiled_code_t parent;

    st_oop_t method;

} st_compiled_block_t;


typedef enum
{
    ST_COMPILED_CODE_NORMAL,
    ST_COMPILED_CODE_RETURN_RECEIVER,
    ST_COMPILED_CODE_RETURN_INSTVAR,
    ST_COMPILED_CODE_RETURN_LITERAL,
    ST_COMPILED_CODE_PRIMITIVE,

} st_compiled_code_flag_t;

typedef enum
{
    ST_COMPILED_CODE_LITERAL_NIL,
    ST_COMPILED_CODE_LITERAL_TRUE,
    ST_COMPILED_CODE_LITERAL_FALSE,
    ST_COMPILED_CODE_LITERAL_MINUS_ONE,
    ST_COMPILED_CODE_LITERAL_ZERO,
    ST_COMPILED_CODE_LITERAL_ONE,
    ST_COMPILED_CODE_LITERAL_TWO,

} st_compiled_code_literal_type_t;


INLINE st_oop_t st_compiled_code_header          (st_oop_t code);

INLINE void     st_compiled_code_set_header      (st_oop_t code, st_oop_t header);

INLINE int      st_compiled_code_temp_count      (st_oop_t code);

INLINE int      st_compiled_code_arg_count       (st_oop_t code);

INLINE int      st_compiled_code_stack_depth     (st_oop_t code);

INLINE int      st_compiled_code_primitive_index (st_oop_t code);

INLINE int      st_compiled_code_flags           (st_oop_t code);


INLINE void     st_compiled_code_set_flags           (st_oop_t code, int flags);

INLINE void     st_compiled_code_set_arg_count       (st_oop_t code, int count);

INLINE void     st_compiled_code_set_temp_count      (st_oop_t code, int count);

INLINE void     st_compiled_code_set_stack_depth     (st_oop_t code, int depth);

INLINE void     st_compiled_code_set_instvar_index   (st_oop_t code, int depth);

INLINE void     st_compiled_code_set_literal_type    (st_oop_t code, st_compiled_code_literal_type_t literal_type);

INLINE void     st_compiled_code_set_primitive_index (st_oop_t code, int index);

INLINE void     st_compiled_code_set_bytecodes       (st_oop_t code, st_oop_t bytecode);

INLINE void     st_compiled_code_set_literals        (st_oop_t code, st_oop_t literals);

INLINE void     st_compiled_block_set_method         (st_oop_t block, st_oop_t method);

INLINE st_oop_t    st_compiled_block_method          (st_oop_t block);

const st_vtable_t *st_compiled_block_vtable          (void);

const st_vtable_t *st_compiled_method_vtable         (void);



/*
 * CompiledCode Header:
 *
 * The CompiledCode header is a smi containing various bitfields.
 *
 * A flag bitfield in the header indicates how the method must be executed. Generally
 * it provides various optimization hints to the interpreter. The flag bitfield
 * is alway stored in bits 31-29. The format of the remaining bits in the header
 * varies according to the flag value.  
 * 
 * flag (3 bits):
 *   0 : Activate method and execute its bytecodes 
 *   1 : The method simply returns 'self'. Don't bother creating a new activation.
 *   2 : The method simply returns an instance variable. Ditto.
 *   3 : The method simply returns a literal. Ditto.
 *   4 : The method performs a primitive operation.
 *
 * Bitfield format
 * 
 * flag = 0:
 *   [ flag: 3 | arg_count: 5 | temp_count: 6 | stack_depth: 8 | primitive: 8 | tag: 2 ]
 *
 *   arg_count:      number of args
 *   temp_count:     number of temps
 *   stack_depth:    depth of stack
 *   primitive:      index of a primitive method
 *   tag:            The usual smi tag
 *
 * flag = 1:
 *   [ flag: 3 | unused: 27 | tag: 2 ]
 *
 * flag = 2:
 *   header: [ flag: 3 | unused: 11 | instvar: 16 | tag: 2 ]
 *
 *   instvar_index:  Index of instvar
 *
 * flag = 3:
 *   header: [ flag: 3 | unused: 23 | literal: 4 | tag: 2 ]
 *
 *   literal: 
 *      nil:             0
 *      true:            1
 *      false:           2
 *      -1:              3
 *       0:              4
 *       1:              5
 *       2:              6
 */

#define SET_BITFIELD(bits, field, value) \
    (((bits) & ~st_##field##_mask_aligned) | ((value & st_##field##_mask) << st_##field##_shift))

#define GET_BITFIELD(bits, field) \
    (((bits) >> st_##field##_shift) & st_##field##_mask)

enum
{
    st_flag_bits         = 3,
    st_arg_bits          = 5,
    st_temp_bits         = 6,
    st_stack_bits        = 8,
    st_instvar_bits      = 16,  
    st_literal_bits      = 4,
    st_primitive_bits    = 8,

    st_primitive_shift   = ST_TAG_SIZE,
    st_instvar_shift     = ST_TAG_SIZE,
    st_literal_shift     = ST_TAG_SIZE,
    st_stack_shift       = st_primitive_bits + st_primitive_shift,
    st_temp_shift        = st_stack_bits + st_stack_shift,
    st_arg_shift         = st_temp_bits + st_temp_shift,
    st_flag_shift        = st_arg_bits + st_arg_shift,

    st_primitive_mask           = ST_NTH_MASK (st_primitive_bits),
    st_primitive_mask_aligned   = st_primitive_bits << st_primitive_shift,

    st_stack_mask               = ST_NTH_MASK (st_stack_bits),
    st_stack_mask_aligned       = st_stack_bits << st_stack_shift,
    
    st_instvar_mask             = ST_NTH_MASK (st_instvar_bits),
    st_instvar_mask_aligned     = st_instvar_mask << st_instvar_shift,

    st_literal_mask             = ST_NTH_MASK (st_literal_bits),
    st_literal_mask_aligned     = st_literal_mask << st_literal_shift,
    
    st_temp_mask                = ST_NTH_MASK (st_temp_bits),
    st_temp_mask_aligned        = st_temp_mask << st_temp_shift,
    
    st_arg_mask                 = ST_NTH_MASK (st_arg_bits),
    st_arg_mask_aligned         = st_arg_mask << st_arg_shift,
    
    st_flag_mask                = ST_NTH_MASK (st_flag_bits),
    st_flag_mask_aligned        = st_flag_mask << st_flag_shift,
};

#define ST_COMPILED_CODE(oop)   ((st_compiled_code_t *)   (ST_POINTER (oop)))
#define ST_COMPILED_METHOD(oop) ((st_compiled_method_t *) (ST_POINTER (oop)))
#define ST_COMPILED_BLOCK(oop)  ((st_compiled_block_t *)  (ST_POINTER (oop)))

#define HEADER(code) (ST_COMPILED_CODE (code)->header)

INLINE st_oop_t
st_compiled_code_header (st_oop_t code)
{
    return ST_COMPILED_CODE (code)->header;
}

INLINE void
st_compiled_code_set_header (st_oop_t code, st_oop_t header)
{
    ST_COMPILED_CODE (code)->header = header;
}

INLINE int
st_compiled_code_temp_count (st_oop_t code)
{
    return GET_BITFIELD (HEADER (code), temp);
}

INLINE int
st_compiled_code_arg_count (st_oop_t code)
{
    return GET_BITFIELD (HEADER (code), arg);
}

INLINE int
st_compiled_code_stack_depth (st_oop_t code)
{
    return GET_BITFIELD (HEADER (code), stack);
}

INLINE int
st_compiled_code_primitive_index (st_oop_t code)
{
    return GET_BITFIELD (HEADER (code), primitive);
}

INLINE int
st_compiled_code_flags (st_oop_t code)
{   
    return GET_BITFIELD (HEADER (code), flag);
}

INLINE void
st_compiled_code_set_flags (st_oop_t code, int flags)
{
    HEADER (code) = SET_BITFIELD (HEADER (code), flag, flags);
}

INLINE void
st_compiled_code_set_arg_count (st_oop_t code, int count)
{	
    HEADER (code) = SET_BITFIELD (HEADER (code), arg, count);
}

INLINE void
st_compiled_code_set_temp_count (st_oop_t code, int count)
{
    HEADER (code) = SET_BITFIELD (HEADER (code), temp, count);
}

INLINE void
st_compiled_code_set_stack_depth (st_oop_t code, int depth)
{
    HEADER (code) = SET_BITFIELD (HEADER (code), stack, depth);
}

INLINE void
st_compiled_code_set_primitive_index (st_oop_t code, int index)
{
    HEADER (code) = SET_BITFIELD (HEADER (code), primitive, index);
}

INLINE void
st_compiled_code_set_instvar_index (st_oop_t code, int index)
{
    HEADER (code) = SET_BITFIELD (HEADER (code), instvar, index);
}

INLINE void
st_compiled_code_set_literal_type (st_oop_t code, st_compiled_code_literal_type_t literal_type)
{
    HEADER (code) = SET_BITFIELD (HEADER (code), literal, literal_type);
}

INLINE st_oop_t
st_compiled_code_literals (st_oop_t code)
{
    return ST_COMPILED_CODE (code)->literals;
}

INLINE st_oop_t
st_compiled_code_bytecodes (st_oop_t code)
{
    return ST_COMPILED_CODE (code)->bytecode;
}

INLINE void
st_compiled_code_set_bytecodes (st_oop_t code, st_oop_t bytecode)
{
    ST_COMPILED_CODE (code)->bytecode = bytecode;
}

INLINE void
st_compiled_code_set_literals (st_oop_t code, st_oop_t literals)
{
    ST_COMPILED_CODE (code)->literals = literals;
}

/* CompiledBlock */

INLINE void
st_compiled_block_set_method (st_oop_t block, st_oop_t method)
{
    ST_COMPILED_BLOCK (block)->method = method;
}

INLINE st_oop_t
st_compiled_block_method (st_oop_t block)
{
    return ST_COMPILED_BLOCK (block)->method;
}


#endif /* __ST_COMPILED_CODE_H__ */
