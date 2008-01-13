/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-hashed-collection.h
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

#ifndef _ST_HASHED_COLLECTION_H__
#define _ST_HASHED_COLLECTION_H__

#include <st-types.h>
#include <st-array.h>

st_oop_t st_dictionary_new (void);
st_oop_t st_dictionary_new_with_capacity (st_smi_t capacity);
st_oop_t st_dictionary_at (st_oop_t dict, st_oop_t key);
void st_dictionary_at_put (st_oop_t dict, st_oop_t key, st_oop_t value);


st_oop_t st_set_new (void);
st_oop_t st_set_new_with_capacity (st_smi_t capacity);
bool st_set_includes (st_oop_t set, st_oop_t object);
st_oop_t st_set_like (st_oop_t set, st_oop_t object);
void st_set_add (st_oop_t set, st_oop_t object);


const st_vtable_t *st_dictionary_vtable (void);
const st_vtable_t *st_set_vtable (void);


#endif /* _ST_HASHED_COLLECTION_H__ */
