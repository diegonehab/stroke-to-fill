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
#include "rvg-lua-scene-data.h"

#include "rvg-lua-scene.h"
#include "rvg-lua-facade.h"
#include "rvg-lua-rgba.h"

using namespace rvg;

static int scene_get_scene_data(lua_State *L) {
	const auto &s = rvg_lua_check<scene>(L, 1);
	rvg_lua_push<scene_data::const_ptr>(L,
		s.get_scene_data_ptr());
    return 1;
}

static int scene_get_background_color(lua_State *L) {
	rvg_lua_push<RGBA8>(L, rvg_lua_check<scene>(L, 1).get_background_color());
    return 1;
}

static int scene_over(lua_State *L) {
	rvg_lua_push<scene>(L, rvg_lua_check<scene>(L, 1).
        over(rvg_lua_check<RGBA8>(L, 2)));
    return 1;
}

static int scene_without_background_color(lua_State *L) {
	rvg_lua_push<scene>(L, rvg_lua_check<scene>(L, 1).
        without_background_color());
    return 1;
}

static luaL_Reg scene__index[] = {
    {"get_scene_data", scene_get_scene_data },
    {"get_background_color", scene_get_background_color },
    {"without_background_color", scene_without_background_color },
    {"over", scene_over },
    { nullptr, nullptr }
};

scene rvg_lua_scene_create(lua_State *L, int base) {
    return scene{
        rvg_lua_facade_scene_data_create(L, base),
        rvg_lua_opt<RGBA8>(L, base+1, RGBA8{255,255,255,255})
    };
}

static int create(lua_State *L) {
    return rvg_lua_push<scene>(L, rvg_lua_scene_create(L, 1));
}

static const luaL_Reg mod_scene[] = {
    {"scene", create},
    {NULL, NULL}
};

int rvg_lua_scene_init(lua_State *L, int ctxidx) {
    rvg_lua_rgba_init(L, ctxidx);
    rvg_lua_xform_init(L, ctxidx);
    rvg_lua_scene_data_init(L, ctxidx);
    if (!rvg_lua_typeexists<scene>(L, ctxidx)) {
		rvg_lua_createtype<scene>(L, "scene", ctxidx);
		rvg_lua_set_xformable<scene>(L, ctxidx);
		rvg_lua_setmethods<scene>(L, scene__index, 0, ctxidx);
    }
    return 0;
}

int rvg_lua_scene_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_scene_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_scene, 1); // mod
    return 0;
}
