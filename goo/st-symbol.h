/*
 * st-symbol.h
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

#ifndef __ST_SYMBOL_H__
#define __ST_SYMBOL_H__

#include <st-types.h>
#include <st-vtable.h>
#include <glib.h>

st_oop st_string_new (const char *bytes);

st_oop st_symbol_new (const char *bytes);

st_oop st_character_new (gunichar unichar);

gunichar st_character_value (st_oop character);


const STVTable *st_symbol_vtable (void);

#endif /* __ST_SYMBOL_H__ */
