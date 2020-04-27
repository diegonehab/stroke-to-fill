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
#include "rvg-lua-xform.h"
#include "rvg-lua-xformable.h"
#include "rvg-lua-patch-data.h"
#include "rvg-lua-patch.h"
#include "rvg-patch.h"

using namespace rvg;

template <>
int rvg_lua_tostring<patch<16,4>>(lua_State *L) {
    lua_pushliteral(L, "tensor-product patch");
    return 1;
}

template <>
int rvg_lua_tostring<patch<12,4>>(lua_State *L) {
    lua_pushliteral(L, "coons patch");
    return 1;
}

template <>
int rvg_lua_tostring<patch<3,3>>(lua_State *L) {
    lua_pushliteral(L, "gouraud triangle");
    return 1;
}

template <typename PATCH>
static int patch_get_patch_data(lua_State *L) {
    const auto &p = rvg_lua_check<PATCH>(L, 1);
    rvg_lua_push<typename PATCH::patch_data_type::const_ptr>(L,
        p.get_patch_data_ptr());
    return 1;
}

template <typename PATCH>
static int patch_get_opacity(lua_State *L) {
    lua_pushnumber(L, unorm<double>{
        rvg_lua_check<PATCH>(L, 1).get_opacity()});
    return 1;
}

template <typename PATCH>
static int create_patch(lua_State *L) {
    unorm8 opacity = unorm<float>::clamped(rvg_lua_optfloat(L, 3, 1.f));
    rvg_lua_push<PATCH>(L, PATCH{
        rvg_lua_patch_data_create<typename PATCH::patch_data_type>(L, 1),
        opacity
    });
    return 1;
}

template <typename PATCH>
static int patch_init(lua_State *L, const char *patch_type_name, int ctxidx) {
    static luaL_Reg patch__index[] = {
        {"get_patch_data", &patch_get_patch_data<PATCH> },
        {"get_opacity", &patch_get_opacity<PATCH> },
        { nullptr, nullptr }
    };
    if (!rvg_lua_typeexists<PATCH>(L, ctxidx)) {
        rvg_lua_createtype<PATCH>(L, patch_type_name, ctxidx);
        rvg_lua_set_xformable<PATCH>(L, ctxidx);
        rvg_lua_setmethods<PATCH>(L, patch__index, 0, ctxidx);
    }
    return 1;
}

static const luaL_Reg mod_patch[] = {
    {"tensor_product_patch", &create_patch<patch<16,4>>},
    {"coons_patch", &create_patch<patch<12,4>>},
    {"gouraud_triangle", &create_patch<patch<3,3>>},
    {nullptr, nullptr}
};

int rvg_lua_patch_init(lua_State *L, int ctxidx) {
    rvg_lua_xform_init(L, ctxidx);
    rvg_lua_patch_data_init(L, ctxidx);
    patch_init<patch<16,4>>(L, "tensor_product_patch", ctxidx);
    patch_init<patch<12,4>>(L, "coons_patch", ctxidx);
    patch_init<patch<3,3>>(L, "gouraud_triangle", ctxidx);
    return 0;
}

int rvg_lua_patch_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_patch_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_patch, 1); // mod
    return 0;
}
