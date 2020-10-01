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
#include <cstdio>
#include <sstream>

#include "rvg-lua.h"
#include "rvg-base64.h"

static int encode(lua_State *L) {
    size_t len = 0;
    const char *str = luaL_checklstring(L, 1, &len);
    std::string encoded = rvg::encode_base64(std::string(str, len));
    lua_pushlstring(L, encoded.data(), encoded.length());
    return 1;
}

static int decode(lua_State *L) {
    size_t len = 0;
    const char *str = luaL_checklstring(L, 1, &len);
    std::string decoded = rvg::decode_base64(std::string(str, len));
    lua_pushlstring(L, decoded.data(), decoded.length());
    return 1;
}

static const luaL_Reg mod[] = {
    {"encode", encode},
    {"decode", decode},
    {NULL, NULL}
};

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_base64(lua_State *L) {
    lua_newtable(L);
    rvg_lua_setfuncs(L, mod, 0);
    return 1;
}
