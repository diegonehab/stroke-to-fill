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
#ifndef RVG_LUA_IMAGE_H
#define RVG_LUA_IMAGE_H

#include "rvg-lua.h"

#include "rvg-image.h"

int rvg_lua_image_init(lua_State *L, int ctxidx);
int rvg_lua_image_export(lua_State *L, int ctxidx);
rvg::i_image::ptr rvg_lua_image_create(lua_State *L, int base);

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_image(lua_State *L);

#endif // RVG_LUA_IMAGE_H
