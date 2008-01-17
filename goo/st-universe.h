/*
 * st-universe.h
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
#ifndef __ST_UNIVERSE_H__
#define __ST_UNIVERSE_H__

#include <st-types.h>

extern st_oop_t st_nil,
    st_true,
    st_false,
    st_symbol_table,
    st_smalltalk,
    st_undefined_object_class,
    st_metaclass_class,
    st_behavior_class,
    st_smi_class,
    st_float_class,
    st_character_class,
    st_true_class,
    st_false_class,
    st_array_class,
    st_byte_array_class,
    st_tuple_class,
    st_set_class,
    st_dictionary_class,
    st_association_class,
    st_string_class,
    st_symbol_class,
    st_compiled_method_class,
    st_compiled_block_class;


st_oop_t st_global_get (const char *name);


#endif /* __ST_UNIVERSE_H__ */
