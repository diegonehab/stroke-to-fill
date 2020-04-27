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

#include "rvg-harfbuzz-font.h"
#include "rvg-harfbuzz-buffer.h"
#include "rvg-harfbuzz-features.h"

using namespace rvg;

static int buffer_set_direction(lua_State *L) {
    size_t len;
    const char *buf = luaL_checklstring(L, 2, &len);
	rvg_lua_check<harfbuzz_buffer::ptr>(L, 1)->set_script(buf, static_cast<int>(len));
    return 0;
}

static int buffer_set_script(lua_State *L) {
    size_t len;
    const char *buf = luaL_checklstring(L, 2, &len);
	rvg_lua_check<harfbuzz_buffer::ptr>(L, 1)->set_script(buf, static_cast<int>(len));
    return 0;
}

static int buffer_set_language(lua_State *L) {
    size_t len;
    const char *buf = luaL_checklstring(L, 2, &len);
	rvg_lua_check<harfbuzz_buffer::ptr>(L, 1)->set_language(buf, static_cast<int>(len));
    return 0;
}

static int buffer_add(lua_State *L) {
    size_t data_len;
    const char *data = luaL_checklstring(L, 2, &data_len);
    int text_os = static_cast<int>(luaL_optinteger(L, 3, 1) - 1);
    int text_len = static_cast<int>(luaL_optinteger(L, 4, -1));
    if (text_os < 0 || text_os >= (int) data_len ||
        (text_len < 0 && text_len != -1) ||
        text_os + text_len > (int) data_len) {
        luaL_error(L, "invalid range");
    }
	rvg_lua_check<harfbuzz_buffer::ptr>(L, 1)->add(data,
		static_cast<int>(data_len), text_os, text_len);
    return 0;
}

static int buffer_reset(lua_State *L) {
	rvg_lua_check<harfbuzz_buffer::ptr>(L, 1)->reset();
    return 0;
}

static int buffer_shape(lua_State *L) {
	static harfbuzz_features::ptr empty(nullptr);
	rvg_lua_check<harfbuzz_buffer::ptr>(L, 1)->shape(
		rvg_lua_check<harfbuzz_font::ptr>(L, 2),
    	rvg_lua_opt<harfbuzz_features::ptr>(L, 3, empty));
    return 0;
}

static int buffer_normalize_glyphs(lua_State *L) {
	rvg_lua_check<harfbuzz_buffer::ptr>(L, 1)->normalize_glyphs();
    return 0;
}

static int buffer_set_cluster_level(lua_State *L) {
    static const char *const levels[] = {
        "monotone graphemes", "monotone characters", "characters", nullptr };
	rvg_lua_check<harfbuzz_buffer::ptr>(L, 1)->set_cluster_level(
		static_cast<hb_buffer_cluster_level_t>(
			luaL_checkoption(L, 2, "monotone graphemes", levels)));
    return 0;
}

static int buffer_guess_segment_properties(lua_State *L) {
	rvg_lua_check<harfbuzz_buffer::ptr>(L, 1)->
		guess_segment_properties();
    return 0;
}

static int buffer_get_glyph_infos(lua_State *L) {
    unsigned length = 0;
	hb_glyph_info_t* infos = rvg_lua_check<harfbuzz_buffer::ptr>(L, 1)->
		get_glyph_infos(&length);
    lua_createtable(L, length, 0); // infos
    lua_pushliteral(L, "codepoint"); // infos "cp"
    lua_pushliteral(L, "cluster"); // infos "cp" "cl"
    for (unsigned i = 0; i < length; ++i) {
        lua_createtable(L, 0, 2); // infos "cp" "cl" info
        lua_pushvalue(L, -3); // infos "cp" "cl" info "cp"
        lua_pushinteger(L, infos[i].codepoint); // infos "cp" "cl" info "cp" cp
        lua_rawset(L, -3); // infos "cp" "cl" info
        lua_pushvalue(L, -2); // infos "cp" "cl" info "cl"
        lua_pushinteger(L, infos[i].cluster); // infos "cp" "cl" info "cl" cl
        lua_rawset(L, -3); // infos "cp" "cl" info
        lua_rawseti(L, -4, i+1); // infos "cp" "cl"
    }
    lua_pop(L, 2); // infos
    lua_pushinteger(L, (int) length);
    return 2;
}

static int buffer_get_glyph_positions(lua_State *L) {
    unsigned length = 0;
	hb_glyph_position_t* pos = rvg_lua_check<harfbuzz_buffer::ptr>(L, 1)->
		get_glyph_positions(&length);
    lua_createtable(L, length, 0); // poss
    lua_pushliteral(L, "x_advance"); // poss "xa"
    lua_pushliteral(L, "y_advance"); // poss "xa" "ya"
    lua_pushliteral(L, "x_offset"); // poss "xa" "ya" "xo"
    lua_pushliteral(L, "y_offset"); // poss "xa" "ya" "xo" "yo"
    for (unsigned i = 0; i < length; ++i) {
        lua_createtable(L, 0, 2); // poss ... pos
        lua_pushvalue(L, -5); // poss ... pos "xa"
        lua_pushinteger(L, pos[i].x_advance);  // poss ... pos "xa" xa
        lua_rawset(L, -3); // poss ... pos
        lua_pushvalue(L, -4); // poss ... pos "ya"
        lua_pushinteger(L, pos[i].y_advance);  // poss ... pos "ya" ya
        lua_rawset(L, -3); // poss ... pos
        lua_pushvalue(L, -3); // poss ... pos "ya"
        lua_pushinteger(L, pos[i].x_offset);  // poss ... pos "xo" xo
        lua_rawset(L, -3); // poss ... pos
        lua_pushvalue(L, -2); // poss ... pos "yo"
        lua_pushinteger(L, pos[i].y_offset);  // poss ... pos "yo" yo
        lua_rawset(L, -3); // poss ... pos
        lua_rawseti(L, -6, i+1); // poss ...
    }
    lua_pop(L, 4); // poss
    lua_pushinteger(L, (int) length);
    return 2;
}

static const luaL_Reg buffer__index[] = {
    {"set_direction", buffer_set_direction},
    {"set_script", buffer_set_script},
    {"set_language", buffer_set_language},
    {"set_cluster_level", buffer_set_cluster_level},
    {"guess_segment_properties", buffer_guess_segment_properties},
    {"add", buffer_add},
    {"reset", buffer_reset},
    {"shape", buffer_shape},
    {"normalize_glyphs", buffer_normalize_glyphs},
    {"get_glyph_infos", buffer_get_glyph_infos},
    {"get_glyph_positions", buffer_get_glyph_positions},
    {NULL, NULL}
};

static int font_get_glyph_name(lua_State *L) {
    char name[128];
    rvg_lua_check<harfbuzz_font::ptr>(L, 1)->get_glyph_name(
		static_cast<int>(luaL_checkinteger(L, 2)), name, sizeof(name));
    lua_pushstring(L, name);
    return 1;
}

static int font_set_scale(lua_State *L) {
    int sx = static_cast<int>(luaL_checkinteger(L, 2));
    int sy = static_cast<int>(luaL_optinteger(L, 3, sx));
    rvg_lua_check<harfbuzz_font::ptr>(L, 1)->set_scale(sx, sy);
    return 0;
}

static int font_set_ft_funcs(lua_State *L) {
    rvg_lua_check<harfbuzz_font::ptr>(L, 1)->set_ft_funcs();
    return 0;
}

static int font_set_ot_funcs(lua_State *L) {
    rvg_lua_check<harfbuzz_font::ptr>(L, 1)->set_ot_funcs();
    return 0;
}

static int font_get_units_per_EM(lua_State *L) {
    lua_pushinteger(L, rvg_lua_check<harfbuzz_font::ptr>(L, 1)->
		get_units_per_EM());
    return 1;
}

static int font_set_units_per_EM(lua_State *L) {
    rvg_lua_check<harfbuzz_font::ptr>(L, 1)->
		set_units_per_EM(static_cast<int>(luaL_checkinteger(L, 2)));
    return 0;
}

static const luaL_Reg font__index[] = {
    {"get_glyph_name", font_get_glyph_name},
    {"set_scale", font_set_scale},
    {"set_ft_funcs", font_set_ft_funcs},
    {"set_ot_funcs", font_set_ot_funcs},
    {"get_units_per_EM", font_get_units_per_EM},
    {"set_units_per_EM", font_set_units_per_EM},
    {NULL, NULL}
};

static int create_font(lua_State *L) {
	auto p = make_intrusive<harfbuzz_font>(
		luaL_checkstring(L, 1),
		static_cast<int>(luaL_optinteger(L, 2, 0)));
	if (p->valid()) {
		return rvg_lua_push<harfbuzz_font::ptr>(L, p);
	} else {
		return 0;
	}
}

static int create_buffer(lua_State *L) {
	auto p = make_intrusive<harfbuzz_buffer>();
	if (p->valid()) {
		return rvg_lua_push<harfbuzz_buffer::ptr>(L, p);
	} else {
		return 0;
	}
}

static int create_features(lua_State *L) {
	return rvg_lua_push<harfbuzz_features::ptr>(L,
		make_intrusive<harfbuzz_features>(luaL_checkstring(L, 1)));
    return 1;
}

static const luaL_Reg mod_font[] = {
    {"font", create_font},
    {NULL, NULL}
};

static const luaL_Reg mod_buffer[] = {
    {"buffer", create_buffer},
    {NULL, NULL}
};

static const luaL_Reg mod_features[] = {
    {"features", create_features},
    {NULL, NULL}
};

int rvg_lua_harfbuzz_font_init(lua_State *L, int ctxidx) {
    if (!rvg_lua_typeexists<harfbuzz_font::ptr>(L, ctxidx)) {
        rvg_lua_createtype<harfbuzz_font::ptr>(L, "harfbuzz::font", ctxidx); // mod ctxtab
        rvg_lua_setmethods<harfbuzz_font::ptr>(L, font__index, 0, ctxidx); // mod ctxtab
	}
	return 0;
}

int rvg_lua_harfbuzz_buffer_init(lua_State *L, int ctxidx) {
    if (!rvg_lua_typeexists<harfbuzz_buffer::ptr>(L, ctxidx)) {
        rvg_lua_createtype<harfbuzz_buffer::ptr>(L, "harfbuzz::buffer", ctxidx); // mod ctxtab
        rvg_lua_setmethods<harfbuzz_buffer::ptr>(L, buffer__index, 0, ctxidx); // mod ctxtab
	}
	return 0;
}

int rvg_lua_harfbuzz_features_init(lua_State *L, int ctxidx) {
    if (!rvg_lua_typeexists<harfbuzz_features::ptr>(L, ctxidx)) {
        rvg_lua_createtype<harfbuzz_features::ptr>(L, "harfbuzz::features", ctxidx); // mod ctxtab
	}
	return 0;
}

int rvg_lua_harfbuzz_font_export(lua_State *L, int ctxidx) {
	// mod
	rvg_lua_harfbuzz_font_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_font, 1); // mod
	return 0;
}

int rvg_lua_harfbuzz_buffer_export(lua_State *L, int ctxidx) {
	// mod
	rvg_lua_harfbuzz_buffer_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_buffer, 1); // mod
	return 0;
}

int rvg_lua_harfbuzz_features_export(lua_State *L, int ctxidx) {
	// mod
	rvg_lua_harfbuzz_features_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_features, 1); // mod
	return 0;
}

int rvg_lua_harfbuzz_init(lua_State *L, int ctxidx) {
    rvg_lua_harfbuzz_font_init(L, ctxidx);
    rvg_lua_harfbuzz_buffer_init(L, ctxidx);
    rvg_lua_harfbuzz_features_init(L, ctxidx);
	return 0;
}

int rvg_lua_harfbuzz_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_harfbuzz_font_export(L, ctxidx); // mod
    rvg_lua_harfbuzz_buffer_export(L, ctxidx); // mod
    rvg_lua_harfbuzz_features_export(L, ctxidx); // mod
	return 0;
}

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_harfbuzz(lua_State *L) {
    rvg_lua_init(L); // ctxtab
    int ctxidx = lua_gettop(L);
    lua_newtable(L); // mod
    rvg_lua_harfbuzz_export(L, ctxidx);
    return 1;
}
