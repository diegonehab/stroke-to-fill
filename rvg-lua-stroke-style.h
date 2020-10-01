// Stroke-to-fill conversion program and test harness
// Copyright (C) 2020 Diego Nehab
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// Contact information: diego.nehab@gmail.com
//
#ifndef RVG_LUA_STROKE_STYLE_H
#define RVG_LUA_STROKE_STYLE_H

#include "rvg-lua.h"

#include "rvg-stroke-style.h"

int rvg_lua_stroke_style_init(lua_State *L, int ctxidx);
int rvg_lua_stroke_style_export(lua_State *L, int ctxidx);
rvg::stroke_style::ptr rvg_lua_stroke_style_create(lua_State *L, int base);

#endif
