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
#include "rvg-lua-spread.h"

using namespace rvg;

static rvg_lua_named_value<e_spread> named_spreads[] = {
    {"clamp", e_spread::clamp},
    {"mirror", e_spread::mirror},
    {"wrap", e_spread::wrap},
    {"transparent", e_spread::transparent},
    { nullptr, e_spread::transparent},
};

int rvg_lua_spread_init(lua_State *L, int ctxidx) {
    if (!rvg_lua_typeexists<e_spread>(L, ctxidx)) {
        rvg_lua_enum_createtype<e_spread>(L, "spread",
            named_spreads, ctxidx);
    }
    return 0;
}

int rvg_lua_spread_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_spread_init(L, ctxidx); // mod
    rvg_lua_enum_pushmap<e_spread>(L, ctxidx); // mod map
    rvg_lua_readonlyproxy(L); // mod mapproxy
    lua_setfield(L, -2, "spread"); // mod
    return 0;
}
