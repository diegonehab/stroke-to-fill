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
#include "rvg-lua-patch-data.h"
#include "rvg-lua-rgba.h"

namespace rvg {
    template <typename PATCH_DATA>
    struct patch_data_points {
        typename PATCH_DATA::const_ptr ptr;
    };

    template <typename PATCH_DATA>
    struct patch_data_colors {
        typename PATCH_DATA::const_ptr ptr;
    };
}

using namespace rvg;

template <typename PATCH_DATA>
static int patch_data_points__index(lua_State *L) {
    const auto &pdp = rvg_lua_check<patch_data_points<PATCH_DATA>>(L, 1);
    int n = static_cast<int>(luaL_checkinteger(L, 2));
    const auto &p = pdp.ptr->get_points();
    if (n >= 1 && n <= 2*static_cast<int>(p.size())) {
        lua_pushnumber(L, p[(n-1)/2][(n-1)%2]);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

template <typename PATCH_DATA>
static int patch_data_points__len(lua_State *L) {
    const auto &pdp = rvg_lua_check<patch_data_points<PATCH_DATA>>(L, 1);
    const auto &p = pdp.ptr->get_points();
    lua_pushinteger(L, static_cast<lua_Integer>(2*p.size()));
    return 1;
}

template <typename PATCH_DATA>
static int patch_data_colors__index(lua_State *L) {
    const auto &pdc = rvg_lua_check<patch_data_colors<PATCH_DATA>>(L, 1);
    int n = static_cast<int>(luaL_checkinteger(L, 2));
    const auto &c = pdc.ptr->get_colors();
    if (n >= 1 && n <= static_cast<int>(c.size())) {
        rvg_lua_push<RGBA8>(L, c[n-1]);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

template <typename PATCH_DATA>
static int patch_data_colors__len(lua_State *L) {
    const auto &pdp = rvg_lua_check<patch_data_points<PATCH_DATA>>(L, 1);
    const auto &c = pdp.ptr->get_colors();
    lua_pushinteger(L, static_cast<lua_Integer>(c.size()));
    return 1;
}

template <typename PATCH_DATA>
static int patch_data_get_points(lua_State *L) {
    using PTR = typename PATCH_DATA::const_ptr;
    auto p = rvg_lua_check<PTR>(L, 1);
    using PDP = patch_data_points<PATCH_DATA>;
    rvg_lua_push<PDP>(L, PDP{p});
    return 1;
}

template <typename PATCH_DATA>
static int patch_data_get_colors(lua_State *L) {
    using PTR = typename PATCH_DATA::const_ptr;
    auto p = rvg_lua_check<PTR>(L, 1);
    using PDC = patch_data_colors<PATCH_DATA>;
    rvg_lua_push<PDC>(L, PDC{p});
    return 1;
}

template <size_t P>
static void copy_points(lua_State *L, int tab, R2 (&points)[P]) {
    luaL_checktype(L, tab, LUA_TTABLE);
    int n = rvg_lua_len(L, tab);
    if (n != 2*P) luaL_error(L, "need %d coordinates", 2*P);
    for (int item = 1; item <= static_cast<int>(P); ++item) {
        lua_rawgeti(L, tab, 2*item-1);
        if (!lua_isnumber(L, -1))
            luaL_error(L, "entry %d is not a number", 2*item-1);
        points[item-1][0] = static_cast<rvgf>(lua_tonumber(L, -1));
        lua_rawgeti(L, tab, 2*item);
        if (!lua_isnumber(L, -1))
            luaL_error(L, "entry %d is not a number", 2*item);
        points[item-1][1] = static_cast<rvgf>(lua_tonumber(L, -1));
        lua_pop(L, 2);
    }
}

template <size_t C>
static void copy_colors(lua_State *L, int tab, RGBA8 (&colors)[C]) {
    luaL_checktype(L, tab, LUA_TTABLE);
    int n = rvg_lua_len(L, tab);
    if (n != C) luaL_error(L, "need %d colors", C);
    for (int item = 1; item <= static_cast<int>(C); ++item) {
        lua_rawgeti(L, tab, item);
        if (!rvg_lua_is<RGBA8>(L, -1))
            luaL_error(L, "entry %d is not a color", item);
        colors[item-1] = rvg_lua_to<RGBA8>(L, -1);
        lua_pop(L, 1);
    }
}

template <typename PATCH_DATA>
static int patch_data_init(lua_State *L, const char *patch_data_type_name,
    int ctxidx) {
    char name[1024];
    static luaL_Reg patch_data__index[] = {
        {"get_points", &patch_data_get_points<PATCH_DATA> },
        {"get_colors", &patch_data_get_colors<PATCH_DATA> },
        { nullptr, nullptr }
    };
    using PTR = typename PATCH_DATA::const_ptr;
    if (!rvg_lua_typeexists<PTR>(L, ctxidx)) {
        sprintf(name, "const %s", patch_data_type_name);
        rvg_lua_createtype<PTR>(L, name, ctxidx);
        rvg_lua_setmethods<PTR>(L, patch_data__index, 0, ctxidx);
    }
    static luaL_Reg patch_data_points_meta[] = {
        {"__index", &patch_data_points__index<PATCH_DATA> },
        {"__len",   &patch_data_points__len<PATCH_DATA> },
        {nullptr, nullptr}
    };
    if (!rvg_lua_typeexists<patch_data_points<PATCH_DATA>>(L, ctxidx)) {
        sprintf(name, "const %s_points", patch_data_type_name);
        rvg_lua_createtype<patch_data_points<PATCH_DATA>>(L, name, ctxidx,
            patch_data_points_meta);
    }
    static luaL_Reg patch_data_colors_meta[] = {
        {"__index", &patch_data_colors__index<PATCH_DATA> },
        {"__len",   &patch_data_colors__len<PATCH_DATA> },
        {nullptr, nullptr}
    };
    if (!rvg_lua_typeexists<patch_data_colors<PATCH_DATA>>(L, ctxidx)) {
        sprintf(name, "const %s_colors", patch_data_type_name);
        rvg_lua_createtype<patch_data_colors<PATCH_DATA>>(L, name, ctxidx,
            patch_data_colors_meta);
    }
    return 1;
}

template <typename PATCH_DATA>
typename PATCH_DATA::const_ptr
rvg_lua_patch_data_create(lua_State *L, int base) {
    R2 points[PATCH_DATA::points_size];
    copy_points(L, base, points);
    RGBA8 colors[PATCH_DATA::colors_size];
    copy_colors(L, base+1, colors);
    return make_intrusive<const PATCH_DATA>(points, colors);
}

template
rvg::patch_data<16,4>::const_ptr
rvg_lua_patch_data_create<rvg::patch_data<16,4>>(lua_State *L, int base);

template
rvg::patch_data<12,4>::const_ptr
rvg_lua_patch_data_create<rvg::patch_data<12,4>>(lua_State *L, int base);

template
rvg::patch_data<3,3>::const_ptr
rvg_lua_patch_data_create<rvg::patch_data<3,3>>(lua_State *L, int base);

template <typename PATCH_DATA>
static int create_patch_data(lua_State *L) {
    using PTR = typename PATCH_DATA::const_ptr;
    rvg_lua_push<PTR>(L, rvg_lua_patch_data_create<PATCH_DATA>(L, 1));
    return 1;
}

static const luaL_Reg mod_patch_data[] = {
    {"tensor_product_patch_data", &create_patch_data<patch_data<16,4>>},
    {"coons_patch_data", &create_patch_data<patch_data<12,4>>},
    {"gouraud_triangle_data", &create_patch_data<patch_data<3,3>>},
    {nullptr, nullptr}
};

int rvg_lua_patch_data_init(lua_State *L, int ctxidx) {
    rvg_lua_rgba_init(L, ctxidx);
    patch_data_init<patch_data<16,4>>(L, "const tensor_product_patch_data",
        ctxidx);
    patch_data_init<patch_data<12,4>>(L, "const coons_patch_data", ctxidx);
    patch_data_init<patch_data<3,3>>(L, "const gouraud_triangle_data", ctxidx);
    return 1;
}

int rvg_lua_patch_data_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_patch_data_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_patch_data, 1); // mod
    return 0;
}
