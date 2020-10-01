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

#include "rvg-lua-winding-rule.h"

using namespace rvg;

static rvg_lua_named_value<e_winding_rule> named_winding_rules[] = {
    {"non_zero", e_winding_rule::non_zero},
    {"odd", e_winding_rule::odd},
    {"zero", e_winding_rule::zero},
    {"even", e_winding_rule::even},
    {nullptr, e_winding_rule::even},
};

int rvg_lua_winding_rule_init(lua_State *L, int ctxidx) {
    if (!rvg_lua_typeexists<e_winding_rule>(L, ctxidx)) {
        rvg_lua_enum_createtype<e_winding_rule>(L, "winding_rule",
            named_winding_rules, ctxidx);
    }
    return 0;
}

int rvg_lua_winding_rule_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_winding_rule_init(L, ctxidx); // mod
    rvg_lua_enum_pushmap<e_winding_rule>(L, ctxidx); // mod map
    rvg_lua_readonlyproxy(L); // mod mapproxy
    lua_setfield(L, -2, "winding_rule"); // mod
    return 0;
}
