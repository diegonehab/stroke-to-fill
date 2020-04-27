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
#include <cmath>

#include "rvg-lua.h"
#include "rvg-util.h"
#include "rvg-floatint.h"
#include "rvg-xform-svd.h"

#ifdef USE_GPERFTOOLS
#include <gperftools/profiler.h>
#endif

using rvg::rvgf;

static int util_is_almost_equal(lua_State *L) {
    lua_pushboolean(L, rvg::util::is_almost_equal(luaL_checknumber(L, 1),
            luaL_checknumber(L, 2), luaL_optinteger(L, 3, 8)));
    return 1;
}

static int util_is_almost_one(lua_State *L) {
    lua_pushboolean(L, rvg::util::is_almost_one(luaL_checknumber(L, 1),
            luaL_optinteger(L, 2, 8)));
    return 1;
}

static int util_is_almost_zero(lua_State *L) {
    lua_pushboolean(L, rvg::util::is_almost_zero(luaL_checknumber(L, 1)));
    return 1;
}

static int util_hypot(lua_State *L) {
    lua_pushnumber(L, std::hypot(luaL_checknumber(L, 1),
            luaL_checknumber(L,2)));
    return 1;
}

static int util_det(lua_State *L) {
    lua_pushnumber(L, rvg::util::det(luaL_checknumber(L, 1),
        luaL_checknumber(L, 2), luaL_checknumber(L, 3),
        luaL_checknumber(L, 4)));
    return 1;
}

static int util_sgn(lua_State *L) {
    lua_pushnumber(L, rvg::util::sgn(luaL_checknumber(L, 1)));
    return 1;
}

static int util_asvd(lua_State *L) {
    auto a = static_cast<rvgf>(luaL_checknumber(L, 1));
    auto b = static_cast<rvgf>(luaL_checknumber(L, 2));
    auto c = static_cast<rvgf>(luaL_checknumber(L, 3));
    auto d = static_cast<rvgf>(luaL_checknumber(L, 4));
    rvg::rotation u;
    rvg::scaling s;
    rvg::linearity v;
    rvg::asvd(rvg::linearity{a, b, c, d}, u, s, v);
    lua_pushnumber(L, u.get_cos());
    lua_pushnumber(L, u.get_sin());
    lua_pushnumber(L, s.get_sx());
    lua_pushnumber(L, s.get_sy());
    lua_pushnumber(L, v[0][0]);
    lua_pushnumber(L, v[0][1]);
    lua_pushnumber(L, v[1][0]);
    lua_pushnumber(L, v[1][1]);
    return 8;
}

static int util_profiler_start(lua_State *L) {
#ifdef USE_GPERFTOOLS
    ProfilerStart(luaL_checkstring(L, 1));
#else
    luaL_error(L, "compiled without GPERFTOOLS");
#endif
    return 0;
}

static int util_profiler_stop(lua_State *L) {
#ifdef USE_GPERFTOOLS
    (void) L;
    ProfilerStop();
#else
    luaL_error(L, "compiled without GPERFTOOLS");
#endif
    return 0;
}

static const luaL_Reg modutil[] = {
    {"is_almost_equal", util_is_almost_equal},
    {"is_almost_one", util_is_almost_one},
    {"is_almost_zero", util_is_almost_zero},
    {"hypot", util_hypot},
    {"asvd", util_asvd},
    {"det", util_det},
    {"sgn", util_sgn},
    {"profiler_start", util_profiler_start},
    {"profiler_stop", util_profiler_stop},
    {NULL, NULL}
};

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_util(lua_State *L) {
    lua_newtable(L); // mod
    rvg_lua_init(L); // mod ctxtab
    rvg_lua_setfuncs(L, modutil, 1); // mod
    return 1;
}
