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
#include <utility> // std::forward, std::make_index_sequence

#include "rvg-ptr.h"
#include "rvg-lua.h"

#include "rvg-lua-path-data.h"
#include "rvg-lua-triangle-data.h"
#include "rvg-lua-circle-data.h"
#include "rvg-lua-rect-data.h"
#include "rvg-lua-polygon-data.h"
#include "rvg-lua-patch-data.h"
#include "rvg-lua-patch.h"
#include "rvg-lua-xform.h"
#include "rvg-lua-xformable.h"
#include "rvg-lua-bbox.h"
#include "rvg-lua-stroke-style.h"
#include "rvg-lua-shape.h"
#include "rvg-lua-rgba.h"
#include "rvg-lua-spread.h"
#include "rvg-lua-color-ramp.h"
#include "rvg-lua-linear-gradient-data.h"
#include "rvg-lua-radial-gradient-data.h"
#include "rvg-lua-texture-data.h"
#include "rvg-lua-paint.h"
#include "rvg-lua-winding-rule.h"
#include "rvg-lua-scene-data.h"
#include "rvg-lua-scene.h"

#include "rvg-facade-scene-data.h"
#include "rvg-facade-stencil-compound.h"
#include "rvg-facade-painted-compound.h"

#include "rvg-lua-facade.h"

namespace rvg_facade {

struct lua_driver { };

}

using namespace rvg;

template <>
int rvg_lua_tostring<rvg_facade::lua_driver>(lua_State *L) {
    lua_pushliteral(L, "lua_driver");
    return 1;
}

template <typename T> struct table_iterator_getter;

template <> struct table_iterator_getter<rvg_facade::stencil_compound> {
    rvg_facade::stencil_compound operator()(lua_State *L, int tab, int item) {
        lua_rawgeti(L, tab, item);
        if (!rvg_lua_is<rvg_facade::stencil_compound>(L, -1))
            luaL_error(L, "entry %d is not a stencil", item);
        rvg_facade::stencil_compound s = rvg_lua_to<rvg_facade::stencil_compound>(L, -1);
        lua_pop(L, 1);
        return s;
    }
};

template <> struct table_iterator_getter<rvg_facade::painted_compound> {
    rvg_facade::painted_compound operator()(lua_State *L, int tab, int item) {
        lua_rawgeti(L, tab, item);
        rvg_facade::painted_compound p;
        if (rvg_lua_is<rvg_facade::painted_compound>(L, -1)) {
            p = rvg_lua_to<rvg_facade::painted_compound>(L, -1);
        } else if (rvg_lua_is<patch<3,3>>(L, -1)) {
            p = rvg_lua_to<patch<3,3>>(L, -1);
        } else if (rvg_lua_is<patch<12,4>>(L, -1)) {
            p = rvg_lua_to<patch<12,4>>(L, -1);
        } else if (rvg_lua_is<patch<16,4>>(L, -1)) {
            p = rvg_lua_to<patch<16,4>>(L, -1);
        } else {
            luaL_error(L, "entry %d is not painted", item);
        }
        lua_pop(L, 1);
        return p;
    }
};

static int driver_punch(lua_State *L, e_winding_rule winding_rule) {
    rvg_lua_push<rvg_facade::stencil_compound>(L,
        rvg_facade::make_stencil_compound_stencil_shape(winding_rule,
            rvg_lua_check<shape>(L, 1)));
    return 1;
}

static int driver_opunch(lua_State *L) {
    return driver_punch(L, e_winding_rule::odd);
}

static int driver_epunch(lua_State *L) {
    return driver_punch(L, e_winding_rule::even);
}

static int driver_nzpunch(lua_State *L) {
    return driver_punch(L, e_winding_rule::non_zero);
}

static int driver_zpunch(lua_State *L) {
    return driver_punch(L, e_winding_rule::zero);
}

static int driver_fill(lua_State *L, e_winding_rule winding_rule) {
    if (rvg_lua_is<RGBA8>(L, 2)) {
        rvg_lua_push<rvg_facade::painted_compound>(L,
            rvg_facade::make_painted_compound_painted_shape(
                winding_rule,
                rvg_lua_check<shape>(L, 1),
                paint{rvg_lua_to<RGBA8>(L, 2), unorm8{255}}));
        return 1;
    } else {
        rvg_lua_push<rvg_facade::painted_compound>(L,
            rvg_facade::make_painted_compound_painted_shape(
			winding_rule, rvg_lua_check<shape>(L, 1),
            rvg_lua_check<paint>(L, 2)));
        return 1;
    }
}

static int driver_ofill(lua_State *L) {
    return driver_fill(L, e_winding_rule::odd);
}

static int driver_efill(lua_State *L) {
    return driver_fill(L, e_winding_rule::even);
}

static int driver_nzfill(lua_State *L) {
    return driver_fill(L, e_winding_rule::non_zero);
}

static int driver_zfill(lua_State *L) {
    return driver_fill(L, e_winding_rule::zero);
}

template <typename T>
static int is_singleton_or_list_of(lua_State *L, int idx) {
    if (lua_type(L, idx) == LUA_TTABLE) {
        lua_rawgeti(L, idx, 1);
        int ret = rvg_lua_is<T>(L, -1);
        lua_pop(L, 1);
        return ret;
    } else {
        return rvg_lua_is<T>(L, idx);
    }
}

static inline rvg_facade::stencil_compound
make_clipped(const std::vector<rvg_facade::stencil_compound> &clipper,
    const std::vector<rvg_facade::stencil_compound> &clipee) {
    return make_stencil_compound_clipped(clipper, clipee);
}

static inline rvg_facade::painted_compound
make_clipped(const std::vector<rvg_facade::stencil_compound> &clipper,
    const std::vector<rvg_facade::painted_compound> &clipee) {
    return make_painted_compound_clipped(clipper, clipee);
}

template <typename T>
static int driver_clip_helper(lua_State *L) {
    std::vector<rvg_facade::stencil_compound> clipper;
    if (lua_type(L, 1) == LUA_TTABLE) {
        int n = rvg_lua_len(L, 1);
        auto get = table_iterator_getter<rvg_facade::stencil_compound>{};
        auto range = rvg_lua_make_table_range<
            rvg_facade::stencil_compound>(L, 1, 1, n+1, get);
        clipper.insert(clipper.begin(), range.first, range.second);
    } else {
        clipper.push_back(rvg_lua_check<rvg_facade::stencil_compound>(L, 1));
    }
    std::vector<T> clippee;
    if (lua_type(L, 2) == LUA_TTABLE) {
        int n = rvg_lua_len(L, 2);
        auto get = table_iterator_getter<T>{};
        auto range = rvg_lua_make_table_range<T>(L, 2, 1, n+1, get);
        clippee.insert(clippee.begin(), range.first, range.second);
    } else {
        clippee.push_back(rvg_lua_check<T>(L, 2));
    }
    rvg_lua_push<T>(L, make_clipped(clipper, clippee));
    return 1;
}

static int driver_clip(lua_State *L) {
    if (!is_singleton_or_list_of<rvg_facade::stencil_compound>(L, 1))
        return luaL_argerror(L, 1, "expected stencil or list of");
    if (is_singleton_or_list_of<rvg_facade::stencil_compound>(L, 2)) {
        return driver_clip_helper<rvg_facade::stencil_compound>(L);
    } else if (is_singleton_or_list_of<rvg_facade::painted_compound>(L, 2)) {
        return driver_clip_helper<rvg_facade::painted_compound>(L);
    } else
        return luaL_argerror(L, 2, "expected stencil, painted, or list of");
}

static inline rvg_facade::stencil_compound
make_xformed(const xform &xf, const std::vector<rvg_facade::stencil_compound> &s) {
    return make_stencil_compound_xformed(xf, s);
}

static inline rvg_facade::painted_compound
make_xformed(const xform &xf, const std::vector<rvg_facade::painted_compound> &p) {
    return make_painted_compound_xformed(xf, p);
}

template <typename T>
static int driver_transform_helper(lua_State *L) {
    std::vector<T> totransform;
    if (lua_type(L, 2) == LUA_TTABLE) {
        int n = rvg_lua_len(L, 2);
        auto get = table_iterator_getter<T>{};
        auto range = rvg_lua_make_table_range<T>(L, 2, 1, n+1, get);
        totransform.insert(totransform.begin(), range.first, range.second);
    } else {
        totransform.push_back(rvg_lua_check<T>(L, 2));
    }
    rvg_lua_push<T>(L, make_xformed(rvg_lua_check<xform>(L, 1), totransform));
    return 1;
}

static int driver_transform(lua_State *L) {
    if (is_singleton_or_list_of<rvg_facade::stencil_compound>(L, 2)) {
        return driver_transform_helper<rvg_facade::stencil_compound>(L);
    } else if (is_singleton_or_list_of<rvg_facade::painted_compound>(L, 2)) {
        return driver_transform_helper<rvg_facade::painted_compound>(L);
    } else
        return luaL_argerror(L, 2, "expected stencil, painted, or list of");
}

static int driver_fade(lua_State *L) {
    if (is_singleton_or_list_of<rvg_facade::painted_compound>(L, 2)) {
        std::vector<rvg_facade::painted_compound> tofade;
        if (lua_type(L, 2) == LUA_TTABLE) {
            int n = rvg_lua_len(L, 2);
            auto get = table_iterator_getter<rvg_facade::painted_compound>{};
            auto range = rvg_lua_make_table_range<
                rvg_facade::painted_compound>(L, 2, 1, n+1, get);
            tofade.insert(tofade.begin(), range.first, range.second);
        } else {
            tofade.push_back(rvg_lua_to<rvg_facade::painted_compound>(L, 2));
        }
        rvg_lua_push<rvg_facade::painted_compound>(L, make_painted_compound_faded(
            unorm8{unorm<float>::clamped(rvg_lua_checkfloat(L, 1))},
                tofade));
        return 1;
    } else
        return luaL_argerror(L, 2, "expected painted, or list of");
}

static int driver_blur(lua_State *L) {
    if (is_singleton_or_list_of<rvg_facade::painted_compound>(L, 2)) {
        std::vector<rvg_facade::painted_compound> toblur;
        if (lua_type(L, 2) == LUA_TTABLE) {
            int n = rvg_lua_len(L, 2);
            auto get = table_iterator_getter<rvg_facade::painted_compound>{};
            auto range = rvg_lua_make_table_range<
                rvg_facade::painted_compound>(L, 2, 1, n+1, get);
            toblur.insert(toblur.begin(), range.first, range.second);
        } else {
            toblur.push_back(rvg_lua_to<rvg_facade::painted_compound>(L, 2));
        }
        rvg_lua_push<rvg_facade::painted_compound>(L, make_painted_compound_blurred(
                rvg_lua_checkfloat(L, 1), toblur));
        return 1;
    } else
        return luaL_argerror(L, 2, "expected painted, or list of");
}

scene_data::ptr create_scene_data_from_table(lua_State *L, int base) {
    int n = rvg_lua_len(L, base);
    auto get = table_iterator_getter<rvg_facade::painted_compound>{};
    auto range = rvg_lua_make_table_range<rvg_facade::painted_compound>(L, base,
        1, n+1, get);
    // ??D We could avoid creating this array if we exposed
    // a version of the scene() function that receives iterators...
    std::vector<rvg_facade::painted_compound> painted(range.first, range.second);
    return make_intrusive<scene_data>(make_scene_data(painted));
}

scene_data::ptr rvg_lua_facade_scene_data_create(lua_State *L, int base) {
    int top = lua_gettop(L);
    if (top < base) {
        return make_intrusive<scene_data>();
    } else if (lua_istable(L, base)) {
        return create_scene_data_from_table(L, base);
    } else if (rvg_lua_is<scene_data::ptr>(L, base)) {
        return make_intrusive<scene_data>(
            *rvg_lua_to<scene_data::ptr>(L, base));
    } else if (rvg_lua_is<scene_data::const_ptr>(L, base)) {
        return make_intrusive<scene_data>(
            *rvg_lua_to<scene_data::const_ptr>(L, base));
    } else {
        luaL_argerror(L, 1, "expected table or scene data");
        return make_intrusive<scene_data>();
    }
}

static luaL_Reg driver__index[] = {
    {"blur", driver_blur },
    {"clip", driver_clip },
    {"efill", driver_efill },
    {"eofill", driver_ofill },
    {"eopunch", driver_opunch },
    {"epunch", driver_epunch },
    {"fade", driver_fade },
    {"fill", driver_nzfill },
    {"nzfill", driver_nzfill },
    {"nzpunch", driver_nzpunch },
    {"ofill", driver_ofill },
    {"opunch", driver_opunch },
    {"punch", driver_nzpunch },
    {"transform", driver_transform },
    {"zfill", driver_zfill },
    {"zpunch", driver_zpunch },
    { nullptr, nullptr }
};

static int rvg_lua_facade_painted_compound_init(lua_State *L, int ctxidx) {
    rvg_lua_xform_init(L, ctxidx);
    rvg_lua_shape_init(L, ctxidx);
    rvg_lua_paint_init(L, ctxidx);
    rvg_lua_winding_rule_init(L, ctxidx);
    rvg_lua_createtype<rvg_facade::painted_compound>(L, "painted_compound", ctxidx);
    rvg_lua_set_xformable<rvg_facade::painted_compound>(L, ctxidx);
    return 0;
}

static int rvg_lua_facade_stencil_compound_init(lua_State *L, int ctxidx) {
    rvg_lua_xform_init(L, ctxidx);
    rvg_lua_shape_init(L, ctxidx);
    rvg_lua_winding_rule_init(L, ctxidx);
    rvg_lua_createtype<rvg_facade::stencil_compound>(L, "stencil_shape", ctxidx);
    rvg_lua_set_xformable<rvg_facade::stencil_compound>(L, ctxidx);
    return 0;
}

static void mergetables(lua_State *L, int srcidx, int destidx) {
    srcidx = rvg_lua_abs_index(L, srcidx);
    destidx = rvg_lua_abs_index(L, destidx);
    lua_pushnil(L);
    while (lua_next(L, srcidx) != 0) {
        lua_pushvalue(L, -2);
        lua_pushvalue(L, -2);
        lua_settable(L, destidx);
        lua_pop(L, 1);
    }
}

int rvg_lua_facade_init(lua_State *L, int ctxidx) {
    ctxidx = rvg_lua_abs_index(L, ctxidx);
    lua_pushstring(L, rvg_lua_raw_name<rvg_facade::lua_driver>()); // rawname
    lua_rawget(L, ctxidx); // drvmet or nil
    if (lua_isnil(L, -1)) { // have not been initialized yet?
        lua_pop(L, 1); //
        lua_newtable(L); // drvmet
        lua_pushstring(L, rvg_lua_raw_name<rvg_facade::lua_driver>()); // drvmet rawname
        lua_pushvalue(L, -2); // drvmet rawname drvmet
        lua_newtable(L); // drvmet rawname drvmet drvidx
        // add all exports to drvidx
        rvg_lua_bbox_export(L, ctxidx);
        rvg_lua_xform_export(L, ctxidx);
        rvg_lua_path_data_export(L, ctxidx);
        rvg_lua_triangle_data_export(L, ctxidx);
        rvg_lua_polygon_data_export(L, ctxidx);
        rvg_lua_circle_data_export(L, ctxidx);
        rvg_lua_rect_data_export(L, ctxidx);
        rvg_lua_patch_data_export(L, ctxidx);
        rvg_lua_patch_export(L, ctxidx);
        rvg_lua_stroke_style_export(L, ctxidx);
        rvg_lua_shape_export(L, ctxidx);
        rvg_lua_rgba_export(L, ctxidx);
        rvg_lua_spread_export(L, ctxidx);
        rvg_lua_color_ramp_export(L, ctxidx);
        rvg_lua_linear_gradient_data_export(L, ctxidx);
        rvg_lua_radial_gradient_data_export(L, ctxidx);
        rvg_lua_texture_data_export(L, ctxidx);
        rvg_lua_paint_export(L, ctxidx);
        rvg_lua_winding_rule_export(L, ctxidx);
        rvg_lua_scene_data_export(L, ctxidx);
        rvg_lua_scene_export(L, ctxidx);
        rvg_lua_facade_painted_compound_init(L, ctxidx);
        rvg_lua_facade_stencil_compound_init(L, ctxidx);
        lua_pushvalue(L, ctxidx); // drvmet rawname drvmet drvidx ctxtab
        rvg_lua_setfuncs(L, driver__index, 1); // drvmet rawname drvmet drvidx
        rvg_lua_require(L, "image"); // drvmet rawname drvmet drvidx image
        lua_setfield(L, -2, "image"); // drvmet rawname drvmet drvidx
        rvg_lua_require(L, "base64"); // drvmet rawname drvmet drvidx base64
        lua_setfield(L, -2, "base64");
        rvg_lua_require(L, "math"); // drvmet rawname drvmet drvidx math
        lua_getglobal(L, "type"); // drvmet rawname drvmet drvidx math type
        mergetables(L, -2, -3); // drvmet rawname drvmet drvidx math type
        lua_setfield(L, -3, "type"); // drvmet rawname drvmet drvidx math
        lua_pop(L, 1); // drvmet rawname drvmet drvidx
        lua_setfield(L, -2, "__index"); // drvmet rawname drvmet
        lua_pushvalue(L, ctxidx); // drvmet rawname drvmet ctxtab
        lua_pushcclosure(L, &rvg_lua_tostring<rvg_facade::lua_driver>, 1); // drvmet rawname drvmet tostring
        lua_setfield(L, -2, "__tostring"); // drvmet rawname drvmet
        lua_pushliteral(L, "driver"); // drvmet rawname drvmet "driver"
        lua_setfield(L, -2, "__name"); // drvmet rawname drvmet
        lua_rawset(L, ctxidx); // drvmet
    }
    // drvmet
    return 1;
}

// creates the driver table with all necessary functions
// accessible via the __index metamethod and include any
// desired additional functions passing the ctxtab as the
// first upvalue
int rvg_lua_facade_new_driver(lua_State *L, const luaL_Reg *funcs) {
    lua_newtable(L); // driver
    rvg_lua_init(L); // driver ctxtab
    rvg_lua_facade_init(L, -1); // driver ctxtab drvmet
    lua_setmetatable(L, -3); // driver ctxtab
    rvg_lua_setfuncs(L, funcs, 1); // driver
    return 1;
}

int facade_new_driver(lua_State *L) {
	if (lua_gettop(L) > 0) {
        luaL_checktype(L, 1, LUA_TTABLE); // driver
	} else {
		lua_newtable(L); // driver
	}
	rvg_lua_gettypemetatable<rvg_facade::lua_driver>(L); // driver drvmet
    lua_setmetatable(L, -2); // driver
    return 1;
}

static const luaL_Reg mod_facade[] = {
    {"driver", facade_new_driver},
    {NULL, NULL}
};

// Lua function invoked to be invoked by require"facade"
extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_facade(lua_State *L) {
	lua_newtable(L); // facade
    rvg_lua_init(L); // facade ctxtab
    rvg_lua_facade_init(L, -1); // facade ctxtab drvmet
	lua_pop(L, 1); // facade ctxtab
    rvg_lua_setfuncs(L, mod_facade, 1); // facade
    return 1;
}
