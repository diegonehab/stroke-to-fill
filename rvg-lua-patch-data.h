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
#ifndef RVG_LUA_PATCH_DATA_H
#define RVG_LUA_PATCH_DATA_H

#include "rvg-patch-data.h"

int rvg_lua_patch_data_init(lua_State *L, int ctxidx);
int rvg_lua_patch_data_export(lua_State *L, int ctxidx);

template <typename PATCH_DATA>
typename PATCH_DATA::const_ptr
rvg_lua_patch_data_create(lua_State *L, int base);

extern template
rvg::patch_data<16,4>::const_ptr
rvg_lua_patch_data_create<rvg::patch_data<16,4>>(lua_State *L, int base);

extern template
rvg::patch_data<12,4>::const_ptr
rvg_lua_patch_data_create<rvg::patch_data<12,4>>(lua_State *L, int base);

extern template
rvg::patch_data<3,3>::const_ptr
rvg_lua_patch_data_create<rvg::patch_data<3,3>>(lua_State *L, int base);

#endif
