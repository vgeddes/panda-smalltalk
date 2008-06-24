/*
 * st-method.h
 *
 * Copyright (C) 2008 Vincent Geddes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

#ifndef __ST_METHOD_H__
#define __ST_METHOD_H__

#include <st-types.h>
#include <st-heap-object.h>
#include <st-byte-array.h>

#define ST_METHOD(oop) ((struct st_method *) (ST_POINTER (oop)))

struct st_method
{
    struct st_header object_header;

    st_oop header;
    st_oop bytecode;
    st_oop literals;
    st_oop selector;

};

typedef enum
{
    ST_METHOD_NORMAL,
    ST_METHOD_RETURN_RECEIVER,
    ST_METHOD_RETURN_INSTVAR,
    ST_METHOD_RETURN_LITERAL,
    ST_METHOD_PRIMITIVE,

} st_method_flags;

typedef enum
{
    ST_METHOD_LITERAL_NIL,
    ST_METHOD_LITERAL_TRUE,
    ST_METHOD_LITERAL_FALSE,
    ST_METHOD_LITERAL_MINUS_ONE,
    ST_METHOD_LITERAL_ZERO,
    ST_METHOD_LITERAL_ONE,
    ST_METHOD_LITERAL_TWO,

} st_method_literal_type;

/*
 * CompiledMethod Header:
 *
 * The header is a smi containing various bitfields.
 *
 * A flag bitfield in the header indicates how the method must be executed. Generally
 * it provides various optimization hints to the interpreter. The flag bitfield
 * is alway stored in bits 31-29. The format of the remaining bits in the header
 * varies according to the flag value.  
 * 
 * flag (3 bits):
 *   0 : Activate method and execute its bytecodes 
 *   1 : The method simply returns 'self'. Has no side-effects. Don't bother creating a new activation.
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
    st_flag_bits        = 3,
    st_arg_bits         = 5,
    st_temp_bits        = 6,
    st_stack_bits       = 8,
    st_instvar_bits     = 16,  
    st_literal_bits     = 4,
    st_primitive_bits   = 8,
    
    st_primitive_shift  = ST_TAG_SIZE,
    st_instvar_shift    = ST_TAG_SIZE,
    st_literal_shift    = ST_TAG_SIZE,
    st_stack_shift      = st_primitive_bits + st_primitive_shift,
    st_temp_shift       = st_stack_bits + st_stack_shift,
    st_arg_shift        = st_temp_bits + st_temp_shift,
    st_flag_shift       = st_arg_bits + st_arg_shift,
    
    st_primitive_mask           = ST_NTH_MASK (st_primitive_bits),
    st_primitive_mask_aligned   = st_primitive_mask << st_primitive_shift,
    
    st_stack_mask               = ST_NTH_MASK (st_stack_bits),
    st_stack_mask_aligned       = st_stack_mask << st_stack_shift,
    
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


static inline int
st_method_get_temp_count (st_oop method)
{
    return GET_BITFIELD (ST_METHOD (method)->header, temp);
}

static inline int
st_method_get_arg_count (st_oop method)
{
    return GET_BITFIELD (ST_METHOD (method)->header, arg);
}

static inline int
st_method_get_stack_depth (st_oop method)
{
    return GET_BITFIELD (ST_METHOD (method)->header, stack);
}

static inline int
st_method_get_primitive_index (st_oop method)
{
    return GET_BITFIELD (ST_METHOD (method)->header, primitive);
}

static inline st_method_flags
st_method_get_flags (st_oop method)
{   
    return GET_BITFIELD (ST_METHOD (method)->header, flag);
}

static inline void
st_method_set_flags (st_oop method, st_method_flags flags)
{
    ST_METHOD (method)->header = SET_BITFIELD (ST_METHOD (method)->header, flag, flags);
}

static inline void
st_method_set_arg_count (st_oop method, int count)
{	
    ST_METHOD (method)->header = SET_BITFIELD (ST_METHOD (method)->header, arg, count);
}

static inline void
st_method_set_temp_count (st_oop method, int count)
{
    ST_METHOD (method)->header = SET_BITFIELD (ST_METHOD (method)->header, temp, count);
}

static inline void
st_method_set_stack_depth (st_oop method, int depth)
{
    ST_METHOD (method)->header = SET_BITFIELD (ST_METHOD (method)->header, stack, depth);
}

static inline void
st_method_set_primitive_index (st_oop method, int index)
{
    ST_METHOD (method)->header = SET_BITFIELD (ST_METHOD (method)->header, primitive, index);
}

static inline void
st_method_set_instvar_index (st_oop method, int index)
{
    ST_METHOD (method)->header = SET_BITFIELD (ST_METHOD (method)->header, instvar, index);
}

static inline void
st_method_set_literal_type (st_oop method, st_method_literal_type literal_type)
{
    ST_METHOD (method)->header = SET_BITFIELD (ST_METHOD (method)->header, literal, literal_type);
}


static inline st_uchar *
st_method_bytecode_bytes (st_oop method)
{
    return st_byte_array_bytes (ST_METHOD (method)->bytecode);    
}

#endif /* __ST_METHOD_H__ */
