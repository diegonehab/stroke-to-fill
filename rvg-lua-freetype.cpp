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
#include "rvg-lua.h"
#include "rvg-freetype.h"
#include "rvg-lua-freetype.h"
#include "rvg-freetype-typeface.h"
#include "rvg-lua-freetype-typeface.h"

using namespace rvg;

static int get_num_faces(lua_State *L) {
    lua_pushinteger(L, rvg_lua_to<freetype>(L, lua_upvalueindex(2)).
        get_num_faces(luaL_checkstring(L, 1)));
    return 1;
}

static int get_memory_num_faces(lua_State *L) {
    size_t size = 0;
    const uint8_t *base = reinterpret_cast<const uint8_t *>(
        luaL_checklstring(L, 1, &size));
    lua_pushinteger(L, rvg_lua_to<freetype>(L, lua_upvalueindex(2)).
        get_memory_num_faces(base, size));
    return 1;
}

static int create_face(lua_State *L) {
    auto ptr = rvg_lua_to<freetype>(L, lua_upvalueindex(2)).new_face(
        luaL_checkstring(L, 1));
    if (ptr) {
        rvg_lua_push<freetype_typeface::ptr>(L, ptr);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int create_memory_face(lua_State *L) {
    size_t size = 0;
    const uint8_t *base = reinterpret_cast<const uint8_t *>(
        luaL_checklstring(L, 1, &size));
    auto ptr = rvg_lua_to<freetype>(L, lua_upvalueindex(2)).new_memory_face(
        base, size);
    if (ptr) {
        rvg_lua_push<freetype_typeface::ptr>(L, ptr);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static const luaL_Reg mod[] = {
    {"get_num_faces", get_num_faces},
    {"get_memory_num_faces", get_memory_num_faces},
    {"face", create_face},
    {"memory_face", create_memory_face},
    {NULL, NULL}
};

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_freetype(lua_State *L) {
    rvg_lua_init(L); // ctxtab
    int ctxidx = lua_gettop(L);
    rvg_lua_freetype_typeface_init(L, ctxidx); // ctxtab
    lua_newtable(L); // ctxtab mod
    lua_pushvalue(L, -2); // ctxtab mod ctxtab
    rvg_lua_createtype<freetype>(L, "freetype", ctxidx); // ctxtab mod ctxtab
    rvg_lua_push<freetype>(L, freetype{}, ctxidx); // ctxtab mod ctxtab lib
    rvg_lua_setfuncs(L, mod, 2); // ctxtab mod
    return 1;
}
