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
#include "rvg-lua-color-ramp.h"
#include "rvg-lua-radial-gradient-data.h"

using namespace rvg;

static int const_radial_gradient_data_get_cx(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<radial_gradient_data::const_ptr>(L, 1)->
        get_cx());
    return 1;
}

static int const_radial_gradient_data_get_cy(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<radial_gradient_data::const_ptr>(L, 1)->
        get_cy());
    return 1;
}

static int const_radial_gradient_data_get_fx(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<radial_gradient_data::const_ptr>(L, 1)->
        get_fx());
    return 1;
}

static int const_radial_gradient_data_get_fy(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<radial_gradient_data::const_ptr>(L, 1)->
        get_fy());
    return 1;
}

static int const_radial_gradient_data_get_r(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<radial_gradient_data::const_ptr>(L, 1)->
        get_r());
    return 1;
}

static int const_radial_gradient_data_get_color_ramp(lua_State *L) {
    rvg_lua_push<color_ramp::const_ptr>(L,
        rvg_lua_check<radial_gradient_data::const_ptr>(L, 1)->
            get_color_ramp_ptr());
    return 1;
}

static luaL_Reg const_radial_gradient_data__index[] = {
    {"get_cx", const_radial_gradient_data_get_cx },
    {"get_cy", const_radial_gradient_data_get_cy },
    {"get_fx", const_radial_gradient_data_get_fx },
    {"get_fy", const_radial_gradient_data_get_fy },
    {"get_r", const_radial_gradient_data_get_r },
    {"get_color_ramp", const_radial_gradient_data_get_color_ramp },
    { nullptr, nullptr }
};

radial_gradient_data::ptr rvg_lua_radial_gradient_data_create(lua_State *L,
    int base) {
    color_ramp::const_ptr ramp_ptr = rvg_lua_check<
        color_ramp::const_ptr>(L, base+0);
    float cx = rvg_lua_checkfloat(L, base+1);
    float cy = rvg_lua_checkfloat(L, base+2);
    float fx = rvg_lua_checkfloat(L, base+3);
    float fy = rvg_lua_checkfloat(L, base+4);
    float r = rvg_lua_checkfloat(L, base+5);
    return make_intrusive<radial_gradient_data>(ramp_ptr, cx, cy, fx, fy, r);
}

static int create(lua_State *L) {
    rvg_lua_push<radial_gradient_data::const_ptr>(L,
        rvg_lua_radial_gradient_data_create(L, 1));
    return 1;
}

static const luaL_Reg mod_radial_gradient_data[] = {
    {"radial_gradient_data", create},
    {NULL, NULL}
};

int rvg_lua_radial_gradient_data_init(lua_State *L, int ctxidx) {
    rvg_lua_color_ramp_init(L, ctxidx);
    if (!rvg_lua_typeexists<radial_gradient_data::const_ptr>(L, ctxidx)) {
        rvg_lua_createtype<radial_gradient_data::const_ptr>(L,
            "const radial_gradient_data", ctxidx);
        rvg_lua_setmethods<radial_gradient_data::const_ptr>(L,
            const_radial_gradient_data__index, 0, ctxidx);
    }
    return 0;
}

int rvg_lua_radial_gradient_data_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_radial_gradient_data_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_radial_gradient_data, 1); // mod
    return 0;
}
