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
#ifndef RVG_LUA_FACADE_DRIVER_H
#define RVG_LUA_FACADE_DRIVER_H

#include "rvg-lua.h"

#include "rvg-scene-data.h"

int rvg_lua_facade_init(lua_State *L, int ctxidx);
rvg::scene_data::ptr rvg_lua_facade_scene_data_create(lua_State *L, int base);
int rvg_lua_facade_new_driver(lua_State *L, const luaL_Reg *funcs);

#endif
