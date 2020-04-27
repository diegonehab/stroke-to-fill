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
#include <chrono>
#include <thread>

#include "rvg-lua.h"

#include "rvg-chronos.h"
#include "rvg-lua-chronos.h"

template <>
int rvg_lua_tostring<rvg::chronos>(lua_State *L) {
    rvg::chronos *time = rvg_lua_check_pointer<rvg::chronos>(L, 1);
    lua_pushfstring(L, "chronos{%f}", time->time());
    return 1;
}

static int resetchronos(lua_State *L) {
    rvg::chronos *time = rvg_lua_check_pointer<rvg::chronos>(L, 1);
    time->reset();
    return 0;
}

static int elapsedchronos(lua_State *L) {
    rvg::chronos *time = rvg_lua_check_pointer<rvg::chronos>(L, 1);
    lua_pushnumber(L, time->elapsed());
    return 1;
}

static int timechronos(lua_State *L) {
    rvg::chronos *time = rvg_lua_check_pointer<rvg::chronos>(L, 1);
    lua_pushnumber(L, time->time());
    return 1;
}

static int sleepchronos(lua_State *L) {
    std::this_thread::sleep_for(
        std::chrono::duration<double>(
            luaL_checknumber(L, 1)));
    return 0;
}

static const luaL_Reg methodschronos[] = {
    {"reset", resetchronos},
    {"time", timechronos},
    {"elapsed", elapsedchronos},
    {NULL, NULL}
};

static int newchronos(lua_State *L) {
    rvg_lua_push<rvg::chronos>(L, rvg::chronos{});
    return 1;
}

static const luaL_Reg modchronos[] = {
    {"chronos", newchronos},
    {"sleep", sleepchronos},
    {NULL, NULL}
};

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_chronos(lua_State *L) {
    lua_newtable(L); // mod
    rvg_lua_init(L); // mod ctxtab
    rvg_lua_createtype<rvg::chronos>(L, "chronos", -1); // mod ctxtab
    rvg_lua_setmethods<rvg::chronos>(L, methodschronos, 0, -1); // mod ctxtab
    rvg_lua_setfuncs(L, modchronos, 1); // mod
    return 1;
}
