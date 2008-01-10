/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-types.h
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

#ifndef __ST_TYPES_H__
#define __ST_TYPES_H__

#include <glib.h>
#include <stdbool.h>
#include <stdint.h>
#include <config.h>

#define ST_TAG_SIZE 2

/* Check host data model
 */
#if (SIZEOF_VOID_P == 4 && SIZEOF_INT == 4)
#  define ST_HOST32         1
#  define ST_HOST64         0
#  define ST_BITS_PER_WORD  32
#  define ST_SMALL_INTEGER_MIN  (-1 << (31 - GOO_TAG_SIZE))
#  define ST_SMALL_INTEGER_MAX  ( 1 << (31 - GOO_TAG_SIZE))
#elif (SIZEOF_VOID_P == 8)
#  define ST_HOST32         0
#  define ST_HOST64         1
#  define ST_BITS_PER_WORD  64
#  define ST_SMALL_INTEGER_MIN  (-1 << (63 - GOO_TAG_SIZE))
#  define ST_SMALL_INTEGER_MAX  ( 1 << (63 - GOO_TAG_SIZE))
#else
#  error platform not supported
#endif

typedef enum
{
    ST_SMI_TAG,
    ST_POINTER_TAG,
    ST_MARK_TAG,
    ST_TAG_UNUSED,

} st_tag_type_t;

/* basic oop pointer:
 * integral type wide enough to hold a C pointer.
 * Can either point to a heap object or contain a smi or mark immediate.
 */
typedef uintptr_t st_oop_t;

/* signed integral type which can hold the value of a smi immediate
 */
#if (ST_HOST32 == 1)
typedef int32_t st_smi_t;
#else
typedef int64_t st_smi_t;
#endif

/* glib already defines `inline' in a portable manner
 */
#ifdef INLINE
#  undef INLINE
#endif
#define INLINE static inline

#define ST_POINTER(oop)          ((st_header_t *) ((oop) - ST_POINTER_TAG))
#define ST_OOP(ptr)              (((st_oop_t) (ptr)) + ST_POINTER_TAG)

#endif /* __ST_TYPES_H__ */
