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

#include "rvg-image.h"
#include "rvg-lua-path-data.h"
#include "rvg-path-f-to-lua-path.h"
#include "rvg-freetype-typeface.h"
#include "rvg-lua-freetype-typeface.h"

using namespace rvg;

static int get_glyph_alpha(lua_State *L) {
    glyph_alpha ga;
    int idx = static_cast<int>(luaL_checkinteger(L, 2));
    int pixels_per_EM = static_cast<int>(luaL_checkinteger(L, 3));
    if (!rvg_lua_check<freetype_typeface::ptr>(L, 1)->get_glyph_alpha(
        idx, pixels_per_EM, &ga)) {
        luaL_error(L, "error getting glyph %d alpha", idx);
        return 0; // To remove warnings
    }
    lua_newtable(L);
    lua_pushinteger(L, ga.pixels_per_EM);
    lua_setfield(L, -2, "pixels_per_EM");
    lua_pushinteger(L, ga.left);
    lua_setfield(L, -2, "left");
    lua_pushinteger(L, ga.top);
    lua_setfield(L, -2, "top");
    lua_pushinteger(L, ga.bottom);
    lua_setfield(L, -2, "bottom");
    rvg_lua_push<i_image::ptr>(L, ga.alpha);
    lua_setfield(L, -2, "alpha");
    return 1;
}

static int get_glyph_metrics(lua_State *L) {
    glyph_metrics gm;
    int idx = static_cast<int>(luaL_checkinteger(L, 2));
    if (!rvg_lua_check<freetype_typeface::ptr>(L, 1)->get_glyph_metrics(
            idx, &gm)) {
        luaL_error(L, "error getting glyph %d metrics", idx);
        return 0; // To remove warnings
    }
    lua_newtable(L);
    lua_pushinteger(L, gm.width);
	lua_setfield(L, -2, "width");
    lua_pushinteger(L, gm.height);
	lua_setfield(L, -2, "height");
    lua_pushinteger(L, gm.horiz_bearing_x);
	lua_setfield(L, -2, "horiz_bearing_x");
    lua_pushinteger(L, gm.horiz_bearing_y);
	lua_setfield(L, -2, "horiz_bearing_y");
    lua_pushinteger(L, gm.horiz_advance);
	lua_setfield(L, -2, "horiz_advance");
    lua_pushinteger(L, gm.vert_bearing_x);
	lua_setfield(L, -2, "vert_bearing_x");
    lua_pushinteger(L, gm.vert_bearing_y);
	lua_setfield(L, -2, "vert_bearing_y");
    lua_pushinteger(L, gm.vert_advance);
	lua_setfield(L, -2, "vert_advance");
    return 1;
}

static int iterate_glyph_outline(lua_State *L) {
    int idx = static_cast<int>(luaL_checkinteger(L, 2));
    if (!rvg_lua_check<freetype_typeface::ptr>(L, 1)->iterate_glyph_outline(
        idx, make_path_f_to_lua_path_in_stack(L, 3))) {
        luaL_error(L, "error iterating glyph %d outline", idx);
    }
    return 1;
}

static int get_glyph_outline(lua_State *L) {
    int idx = static_cast<int>(luaL_checkinteger(L, 2));
    auto p = make_intrusive<path_data>();
    if (!rvg_lua_check<freetype_typeface::ptr>(L, 1)->
        iterate_glyph_outline(idx, *p)) {
        luaL_error(L, "error getting glyph %d outline", idx);
    }
    rvg_lua_push<path_data::ptr>(L, p);
    return 1;
}

static int get_kerning(lua_State *L) {
    int left = static_cast<int>(luaL_checkinteger(L, 2));
    int right = static_cast<int>(luaL_checkinteger(L, 3));
    auto k = rvg_lua_check<freetype_typeface::ptr>(L, 1)->
        get_kerning(left, right);
    lua_pushinteger(L, k.first);
    lua_pushinteger(L, k.second);
    return 2;
}

static int get_char_index(lua_State *L) {
    lua_pushinteger(L, rvg_lua_check<freetype_typeface::ptr>(L, 1)->
        get_char_index((int) luaL_checkinteger(L, 2)));
    return 1;
}

int get_attributes(lua_State *L) {
    face_attributes attr;
    auto ptr = rvg_lua_check<freetype_typeface::ptr>(L, 1);
    if (!ptr) {
        luaL_error(L, "broken ptr?");
    }
    if (!ptr->get_attributes(&attr)) {
        luaL_error(L, "error getting face attributes");
    }
    lua_newtable(L);
    int idx = lua_gettop(L);
    lua_pushinteger(L, attr.num_faces);
    lua_setfield(L, idx, "num_faces");
    lua_pushinteger(L, attr.face_index);
    lua_setfield(L, idx, "face_index");
    lua_pushinteger(L, attr.num_glyphs);
    lua_setfield(L, idx, "num_glyphs");
    lua_pushstring(L, attr.family_name.c_str());
    lua_setfield(L, idx, "face_family");
    lua_pushstring(L, attr.style_name.c_str());
    lua_setfield(L, idx, "style_name");
    lua_pushinteger(L, attr.units_per_EM);
    lua_setfield(L, idx, "units_per_EM");
    lua_pushinteger(L, attr.ascender);
    lua_setfield(L, idx, "ascender");
    lua_pushinteger(L, attr.descender);
    lua_setfield(L, idx, "descender");
    lua_pushinteger(L, attr.height);
    lua_setfield(L, idx, "height");
    lua_pushinteger(L, attr.max_advance_width);
    lua_setfield(L, idx, "max_advance_width");
    lua_pushinteger(L, attr.max_advance_height);
    lua_setfield(L, idx, "max_advance_height");
    lua_pushinteger(L, attr.underline_position);
    lua_setfield(L, idx, "underline_position");
    lua_pushinteger(L, attr.underline_thickness);
    lua_setfield(L, idx, "underline_thickness");
    lua_newtable(L);
    lua_pushinteger(L, attr.bounding_box[0]);
    lua_rawseti(L, -2, 1);
    lua_pushinteger(L, attr.bounding_box[1]);
    lua_rawseti(L, -2, 2);
    lua_pushinteger(L, attr.bounding_box[2]);
    lua_rawseti(L, -2, 3);
    lua_pushinteger(L, attr.bounding_box[3]);
    lua_rawseti(L, -2, 4);
    lua_setfield(L, idx, "bounding_box");
    return 1;
}

static luaL_Reg typeface__index[] = {
    {"get_kerning", get_kerning },
    {"get_char_index", get_char_index },
    {"get_attributes", get_attributes },
    {"get_glyph_metrics", get_glyph_metrics },
    {"get_glyph_alpha", get_glyph_alpha },
    {"get_glyph_outline", get_glyph_outline },
    {"iterate_glyph_outline", iterate_glyph_outline },
    { nullptr, nullptr }
};

int rvg_lua_freetype_typeface_init(lua_State *L, int ctxidx) {
    rvg_lua_require(L, "image"); lua_pop(L, 1);
    rvg_lua_path_data_init(L, ctxidx);
    if (!rvg_lua_typeexists<freetype_typeface::ptr>(L, ctxidx)) {
        rvg_lua_createtype<freetype_typeface::ptr>(L, "typeface", ctxidx);
        rvg_lua_setmethods<freetype_typeface::ptr>(L,
            typeface__index, 0, ctxidx);
    }
    return 0;
}
