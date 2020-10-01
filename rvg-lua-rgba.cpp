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
#include "rvg-unorm.h"
#include "rvg-rgba.h"
#include "rvg-named-colors.h"

using namespace rvg;

template <>
int rvg_lua_tostring<RGBA8>(lua_State *L) {
    auto c = *reinterpret_cast<RGBA8 *>(lua_touserdata(L, 1));
    lua_pushfstring(L, "rgba8{%d,%d,%d,%d}", c[0], c[1], c[2], c[3]);
    return 1;
}

static int RGBA8__index(lua_State *L) {
    int n = static_cast<int>(luaL_checkinteger(L, 2));
    if (n >= 1 && n <= 4) {
        lua_pushnumber(L, unorm<double>{rvg_lua_check<RGBA8>(L, 1)[n-1]});
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int RGBA8__len(lua_State *L) {
    lua_pushinteger(L, 4);
    return 1;
}

static luaL_Reg RGBA8_meta[] = {
    {"__index", RGBA8__index},
    {"__len", RGBA8__len},
    {nullptr, nullptr}
};

static int create_rgba(lua_State *L) {
    RGBA<unorm<float>> c{
        unorm<float>::clamped(rvg_lua_checkfloat(L, 1)),
        unorm<float>::clamped(rvg_lua_checkfloat(L, 2)),
        unorm<float>::clamped(rvg_lua_checkfloat(L, 3)),
        unorm<float>::clamped(rvg_lua_optfloat(L, 4, 1.f))
    };
    rvg_lua_push<RGBA8>(L, RGBA8{c});
    return 1;
}

static int create_rgba8(lua_State *L) {
    RGBA<unorm8> c{
        unorm8::clamped(luaL_checkinteger(L, 1)),
        unorm8::clamped(luaL_checkinteger(L, 2)),
        unorm8::clamped(luaL_checkinteger(L, 3)),
        unorm8::clamped(luaL_optinteger(L, 4, 255))
    };
    rvg_lua_push<RGBA8>(L, RGBA8{c});
    return 1;
}

static int color_name(lua_State *L) {
    const auto found = named_colors::RGBA8_to_string.find(
        rvg_lua_check<RGBA8>(L, 1));
    if (found != named_colors::RGBA8_to_string.end()) {
        lua_pushlstring(L, found->second.data(), found->second.size());
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static const luaL_Reg mod_rgba[] = {
    {"rgb", create_rgba},
    {"rgba", create_rgba},
    {"rgb8", create_rgba8},
    {"rgba8", create_rgba8},
    {"color_name", color_name},
    {NULL, NULL}
};

static void clonenamedcolors(lua_State *L, int ctxidx) {
    ctxidx = rvg_lua_abs_index(L, ctxidx);
    lua_newtable(L);
    for (const auto &color: named_colors::string_to_RGBA8) {
        rvg_lua_push<RGBA8>(L, color.second, ctxidx);
        lua_setfield(L, -2, color.first.c_str());
    }
}

int rvg_lua_rgba_init(lua_State *L, int ctxidx) {
    if (!rvg_lua_typeexists<RGBA8>(L, ctxidx)) {
        rvg_lua_createtype<RGBA8>(L, "color", ctxidx, RGBA8_meta);
    }
    return 0;
}

int rvg_lua_rgba_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_rgba_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_rgba, 1); // mod
    clonenamedcolors(L, ctxidx); // mod named_color
    lua_setfield(L, -2, "color");
    return 0;
}
