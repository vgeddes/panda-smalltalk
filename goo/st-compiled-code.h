/*
 * st-compiled-code.h
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

#ifndef __ST_COMPILED_CODE_H__
#define __ST_COMPILED_CODE_H__

#include <st-types.h>
#include <st-vtable.h>
#include <st-heap-object.h>
#include <glib.h>

typedef struct
{
    STHeader _header;

    st_oop header;
    st_oop bytecode;
    st_oop literals;

} STCompiledCode;

typedef struct
{
    STCompiledCode parent;

} STCompiledMethod;

typedef struct
{
    STCompiledCode parent;

    st_oop method;

} STCompiledBlock;


typedef enum
{
    ST_COMPILED_CODE_NORMAL,
    ST_COMPILED_CODE_RETURN_RECEIVER,
    ST_COMPILED_CODE_RETURN_INSTVAR,
    ST_COMPILED_CODE_RETURN_LITERAL,
    ST_COMPILED_CODE_PRIMITIVE,

} st_compiled_code_flag;

typedef enum
{
    ST_COMPILED_CODE_LITERAL_NIL,
    ST_COMPILED_CODE_LITERAL_TRUE,
    ST_COMPILED_CODE_LITERAL_FALSE,
    ST_COMPILED_CODE_LITERAL_MINUS_ONE,
    ST_COMPILED_CODE_LITERAL_ZERO,
    ST_COMPILED_CODE_LITERAL_ONE,
    ST_COMPILED_CODE_LITERAL_TWO,

} st_compiled_code_literal_type;


INLINE st_oop st_compiled_code_header          (st_oop code);

INLINE void     st_compiled_code_set_header      (st_oop code, st_oop header);

INLINE int      st_compiled_code_temp_count      (st_oop code);

INLINE int      st_compiled_code_arg_count       (st_oop code);

INLINE int      st_compiled_code_stack_depth     (st_oop code);

INLINE int      st_compiled_code_primitive_index (st_oop code);

INLINE int      st_compiled_code_flags           (st_oop code);


INLINE void     st_compiled_code_set_flags           (st_oop code, int flags);

INLINE void     st_compiled_code_set_arg_count       (st_oop code, int count);

INLINE void     st_compiled_code_set_temp_count      (st_oop code, int count);

INLINE void     st_compiled_code_set_stack_depth     (st_oop code, int depth);

INLINE void     st_compiled_code_set_instvar_index   (st_oop code, int depth);

INLINE void     st_compiled_code_set_literal_type    (st_oop code, st_compiled_code_literal_type literal_type);

INLINE void     st_compiled_code_set_primitive_index (st_oop code, int index);

INLINE void     st_compiled_code_set_bytecodes       (st_oop code, st_oop bytecode);

INLINE void     st_compiled_code_set_literals        (st_oop code, st_oop literals);

INLINE void     st_compiled_block_set_method         (st_oop block, st_oop method);

INLINE st_oop    st_compiled_block_method          (st_oop block);


const STVTable *st_compiled_block_vtable  (void);

const STVTable *st_compiled_method_vtable (void);



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

#define ST_COMPILED_CODE(oop)   ((STCompiledCode *)   (ST_POINTER (oop)))
#define ST_COMPILED_METHOD(oop) ((STCompiledMethod *) (ST_POINTER (oop)))
#define ST_COMPILED_BLOCK(oop)  ((STCompiledBlock *)  (ST_POINTER (oop)))

#define HEADER(code) (ST_COMPILED_CODE (code)->header)

INLINE st_oop
st_compiled_code_header (st_oop code)
{
    return ST_COMPILED_CODE (code)->header;
}

INLINE void
st_compiled_code_set_header (st_oop code, st_oop header)
{
    ST_COMPILED_CODE (code)->header = header;
}

INLINE int
st_compiled_code_temp_count (st_oop code)
{
    return GET_BITFIELD (HEADER (code), temp);
}

INLINE int
st_compiled_code_arg_count (st_oop code)
{
    return GET_BITFIELD (HEADER (code), arg);
}

INLINE int
st_compiled_code_stack_depth (st_oop code)
{
    return GET_BITFIELD (HEADER (code), stack);
}

INLINE int
st_compiled_code_primitive_index (st_oop code)
{
    return GET_BITFIELD (HEADER (code), primitive);
}

INLINE int
st_compiled_code_flags (st_oop code)
{   
    return GET_BITFIELD (HEADER (code), flag);
}

INLINE void
st_compiled_code_set_flags (st_oop code, int flags)
{
    HEADER (code) = SET_BITFIELD (HEADER (code), flag, flags);
}

INLINE void
st_compiled_code_set_arg_count (st_oop code, int count)
{	
    HEADER (code) = SET_BITFIELD (HEADER (code), arg, count);
}

INLINE void
st_compiled_code_set_temp_count (st_oop code, int count)
{
    HEADER (code) = SET_BITFIELD (HEADER (code), temp, count);
}

INLINE void
st_compiled_code_set_stack_depth (st_oop code, int depth)
{
    HEADER (code) = SET_BITFIELD (HEADER (code), stack, depth);
}

INLINE void
st_compiled_code_set_primitive_index (st_oop code, int index)
{
    HEADER (code) = SET_BITFIELD (HEADER (code), primitive, index);
}

INLINE void
st_compiled_code_set_instvar_index (st_oop code, int index)
{
    HEADER (code) = SET_BITFIELD (HEADER (code), instvar, index);
}

INLINE void
st_compiled_code_set_literal_type (st_oop code, st_compiled_code_literal_type literal_type)
{
    HEADER (code) = SET_BITFIELD (HEADER (code), literal, literal_type);
}

INLINE st_oop
st_compiled_code_literals (st_oop code)
{
    return ST_COMPILED_CODE (code)->literals;
}

INLINE st_oop
st_compiled_code_bytecodes (st_oop code)
{
    return ST_COMPILED_CODE (code)->bytecode;
}

INLINE void
st_compiled_code_set_bytecodes (st_oop code, st_oop bytecode)
{
    ST_COMPILED_CODE (code)->bytecode = bytecode;
}

INLINE void
st_compiled_code_set_literals (st_oop code, st_oop literals)
{
    ST_COMPILED_CODE (code)->literals = literals;
}

/* CompiledBlock */

INLINE void
st_compiled_block_set_method (st_oop block, st_oop method)
{
    ST_COMPILED_BLOCK (block)->method = method;
}

INLINE st_oop
st_compiled_block_method (st_oop block)
{
    return ST_COMPILED_BLOCK (block)->method;
}


#endif /* __ST_COMPILED_CODE_H__ */
