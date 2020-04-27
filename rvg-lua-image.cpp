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
#include <array>
#include <new>

#include "rvg-lua.h"

#include "rvg-i-image.h"
#include "rvg-image.h"
#include "rvg-pngio.h"

#include "rvg-lua-image.h"

template <typename T, size_t N>
static rvg::i_image::ptr
create_image_ptr(int width, int height) {
    auto img = rvg::make_intrusive<rvg::image<T,N>>();
    img->resize(width, height);
    img->set_color_space(rvg::e_color_space::sRGB);
    return img;
}

template <typename T>
static rvg::i_image::ptr
create_image_ptr(int width, int height, int channels) {
    switch (channels) {
        case 1:
            return create_image_ptr<T,1>(width, height);
        case 2:
            return create_image_ptr<T,2>(width, height);
        case 3:
            return create_image_ptr<T,3>(width, height);
        case 4:
            return create_image_ptr<T,4>(width, height);
        default:
            return nullptr;
    }
}

rvg::i_image::ptr rvg_lua_image_create(lua_State *L, int base) {
    int width = static_cast<int>(luaL_checkinteger(L, base+0));
    if (width <= 0) luaL_argerror(L, base+0, "invalid width");
    int height = static_cast<int>(luaL_checkinteger(L, base+1));
    if (height <= 0) luaL_argerror(L, base+1, "invalid height");
    int channels = static_cast<int>(luaL_checkinteger(L, base+2));
    if (channels <= 0 || channels > 4)
        luaL_argerror(L, base+2, "invalid number of channels");
    const char *const lst[] = { "uint8_t", "uint16_t", "float", nullptr };
    int channel_type = luaL_checkoption(L, base+3, "uint8_t", lst);
    switch (channel_type) {
        case 0:
            return create_image_ptr<uint8_t>(width, height, channels);
        case 1:
            return create_image_ptr<uint16_t>(width, height, channels);
        case 2:
            return create_image_ptr<float>(width, height, channels);
        default:
            return nullptr;
    }
}

static int create_image(lua_State *L) {
    auto img = rvg_lua_image_create(L, 1);
    if (img) {
        rvg_lua_push<rvg::i_image::ptr>(L, img);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int is_image(lua_State *L) {
    if (rvg_lua_is<rvg::i_image::ptr>(L, 1)) lua_pushnumber(L, 1);
    else lua_pushnil(L);
    return 1;
}

static const luaL_Reg mod_image[] = {
    {"image", create_image},
    {"is_image", is_image},
    {NULL, NULL}
};

static const char *channel_type_name(rvg::e_channel_type channel_type) {
    switch (channel_type) {
        case rvg::e_channel_type::float_: return "float";
        case rvg::e_channel_type::uint8_t_: return "uint8_t";
        case rvg::e_channel_type::uint16_t_: return "uint16_t";
        default: return "unknown";
    }
}

static const char *color_space_name(rvg::e_color_space color_space) {
    switch (color_space) {
        case rvg::e_color_space::sRGB: return "sRGB";
        case rvg::e_color_space::linear: return "linear";
        default: return "unknown";
    }
}

template <>
int rvg_lua_tostring<rvg::i_image::ptr>(lua_State *L) {
    auto img = rvg_lua_check<rvg::i_image::ptr>(L, 1);
    lua_pushfstring(L, "image{%d,%d,%d,%s}", img->get_width(),
        img->get_height(), img->get_num_channels(), channel_type_name(
            img->get_channel_type()));
    return 1;
}

template <>
int rvg_lua_tostring<rvg::i_image::const_ptr>(lua_State *L) {
    auto img = rvg_lua_check<rvg::i_image::const_ptr>(L, 1);
    lua_pushfstring(L, "const image{%d,%d,%d,%s}", img->get_width(),
        img->get_height(), img->get_num_channels(), channel_type_name(
            img->get_channel_type()));
    return 1;
}

static int image_set_pixel(lua_State *L) {
    auto img = rvg_lua_check<rvg::i_image::ptr>(L, 1);
    int x = static_cast<int>(luaL_checkinteger(L, 2))-1;
    int y = static_cast<int>(luaL_checkinteger(L, 3))-1;
    if (x >= 0 && x < img->get_width() && y >= 0 && y < img->get_height()) {
        for (int c = 0; c < img->get_num_channels(); c++) {
            img->set_unorm(x, y, c,
                static_cast<float>(luaL_checknumber(L, 4+c)));
        }
        return 0;
    } else {
        return luaL_error(L, "index out of bounds");
    }
}

template <typename PTR>
static int image_get_pixel(lua_State *L) {
    auto img = rvg_lua_check<PTR>(L, 1);
    int x = static_cast<int>(luaL_checkinteger(L, 2))-1;
    int y = static_cast<int>(luaL_checkinteger(L, 3))-1;
    if (x >= 0 && x < img->get_width() && y >= 0 && y < img->get_height()) {
        for (int c = 0; c < img->get_num_channels(); c++) {
            lua_pushnumber(L, img->get_unorm(x, y, c));
        }
        return img->get_num_channels();
    } else {
        return luaL_error(L, "index out of bounds");
    }
}

template <typename PTR>
static int image_get_width(lua_State *L) {
    auto img = rvg_lua_check<PTR>(L, 1);
    lua_pushinteger(L, img->get_width());
    return 1;
}

template <typename PTR>
static int image_get_height(lua_State *L) {
    auto img = rvg_lua_check<PTR>(L, 1);
    lua_pushinteger(L, img->get_height());
    return 1;
}

template <typename PTR>
static int image_get_num_channels(lua_State *L) {
    auto img = rvg_lua_check<PTR>(L, 1);
    lua_pushinteger(L, img->get_num_channels());
    return 1;
}

template <typename PTR>
static int image_get_color_space(lua_State *L) {
    auto img = rvg_lua_check<PTR>(L, 1);
    lua_pushstring(L, color_space_name(img->get_color_space()));
    return 1;
}

template <typename PTR>
static int image_get_channel_type(lua_State *L) {
    auto img = rvg_lua_check<PTR>(L, 1);
    lua_pushstring(L, channel_type_name(img->get_channel_type()));
    return 1;
}

static const luaL_Reg image__index[] = {
    {"set_pixel", image_set_pixel},
    {"get_pixel", &image_get_pixel<rvg::i_image::ptr>},
    {"get_width", &image_get_width<rvg::i_image::ptr>},
    {"get_height", &image_get_height<rvg::i_image::ptr>},
    {"get_num_channels", &image_get_num_channels<rvg::i_image::ptr>},
    {"get_channel_type", &image_get_channel_type<rvg::i_image::ptr>},
    {"get_color_space", &image_get_color_space<rvg::i_image::ptr>},
    {NULL, NULL}
};

static const luaL_Reg const_image__index[] = {
    {"get_pixel", &image_get_pixel<rvg::i_image::const_ptr>},
    {"get_width", &image_get_width<rvg::i_image::const_ptr>},
    {"get_height", &image_get_height<rvg::i_image::const_ptr>},
    {"get_num_channels", &image_get_num_channels<rvg::i_image::const_ptr>},
    {"get_channel_type", &image_get_channel_type<rvg::i_image::const_ptr>},
    {"get_color_space", &image_get_color_space<rvg::i_image::const_ptr>},
    {NULL, NULL}
};

static void describe_helper(lua_State *L, int width, int height,
    int channels, int depth) {
    lua_createtable(L, 0, 4);
    lua_pushinteger(L, width);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, height);
    lua_setfield(L, -2, "height");
    lua_pushinteger(L, channels);
    lua_setfield(L, -2, "channels");
    lua_pushinteger(L, depth);
    lua_setfield(L, -2, "depth");
}

static int describe_png(lua_State *L) {
    int width, height, channels, depth;
    // try to load from string
    if (lua_isstring(L, 1)) {
        size_t len = 0;
        const char *str = lua_tolstring(L, 1, &len);
        if (!rvg::describe_png(std::string(str, len), &width, &height,
            &channels, &depth)) {
            lua_pushnil(L);
            lua_pushliteral(L, "failed to load PNG from memory");
            return 2;
        } else {
            describe_helper(L, width, height, channels, depth);
            return 1;
        }
    // else try to load from file
    } else {
        FILE *f = rvg_lua_check_file(L, 1);
        if (!rvg::describe_png(f, &width, &height, &channels, &depth)) {
            lua_pushnil(L);
            lua_pushliteral(L, "failed to load PNG from file");
            return 2;
        } else {
            describe_helper(L, width, height, channels, depth);
            return 1;
        }
    }
}

static void copy_attrs(lua_State *L, int idx,
	const rvg::image_attributes &attrs) {
    idx = rvg_lua_abs_index(L, idx);
    for (auto &attr: attrs) {
        lua_pushstring(L, attr.first.c_str());
        lua_pushstring(L, attr.second.c_str());
        lua_settable(L, idx);
    }
}

template <typename R>
static int load_png_by_input_type(lua_State *L, R &&r) {
    int wanted_channels = static_cast<int>(luaL_optinteger(L, 2, 0));
    rvg::image_attributes attrs;
    rvg::image_attributes *attrs_ptr = lua_istable(L, 3)? &attrs: nullptr;
	auto img = rvg::load_png(std::forward<R>(r), wanted_channels, attrs_ptr);
	if (img) {
        rvg_lua_push<rvg::i_image::ptr>(L, img);
		if (attrs_ptr) {
            copy_attrs(L, 2, attrs);
			return 2;
		} else {
			return 1;
		}
    } else {
        lua_pushnil(L);
        lua_pushliteral(L, "failed to load image");
        return 2;
    }
}

static int load_png(lua_State *L) {
    // try to load from string
    if (lua_isstring(L, 1)) {
        size_t len = 0;
        const char *str = lua_tolstring(L, 1, &len);
        return load_png_by_input_type(L, std::string(str, len));
    // else try to load from file
    } else {
        return load_png_by_input_type(L, rvg_lua_check_file(L, 1));
    }
}

static rvg::image_attributes opt_attrs(lua_State *L, int idx) {
    rvg::image_attributes attrs;
    idx = rvg_lua_abs_index(L, idx);
    if (idx <= lua_gettop(L)) {
        luaL_checktype(L, idx, LUA_TTABLE);
        lua_pushnil(L);
        while (lua_next(L, idx) != 0) {
            lua_pushvalue(L, -2);
            const char *key = lua_tostring(L, -1);
            const char *value = lua_tostring(L, -2);
            if (value && key) attrs.emplace_back(key, value);
            lua_pop(L, 2);
        }
    }
    return attrs;
}

template <typename U>
static int string_png(lua_State *L) {
    auto img = rvg_lua_check_either<
        rvg::i_image::const_ptr,
        rvg::i_image::ptr
    >(L, 1);
    auto attrs = opt_attrs(L, 2);
    std::string str;
    if (!rvg::store_png<U>(&str, img, attrs))
        luaL_error(L, "store to memory failed");
    lua_pushlstring(L, str.data(), str.length());
    return 1;
}

template <typename U>
static int store_png(lua_State *L) {
    FILE *f = rvg_lua_check_file(L, 1);
    auto img = rvg_lua_check_either<
        rvg::i_image::const_ptr,
        rvg::i_image::ptr
    >(L, 2);
    auto attrs = opt_attrs(L, 3);
    if (!rvg::store_png<U>(f, img, attrs))
        luaL_error(L, "store to file failed");
    lua_pushinteger(L, 1);
    return 1;
}

static const luaL_Reg mod_png[] = {
    {"load", load_png},
    {"describe", describe_png},
    {"store8", &store_png<uint8_t>},
    {"store16", &store_png<uint16_t>},
    {"string8", &string_png<uint8_t>},
    {"string16", &string_png<uint8_t>},
    {NULL, NULL}
};

int rvg_lua_image_init(lua_State *L, int ctxidx) {
    if (!rvg_lua_typeexists<rvg::i_image::ptr>(L, ctxidx)) {
        rvg_lua_createtype<rvg::i_image::ptr>(L, "image", ctxidx); // mod ctxtab
        rvg_lua_setmethods<rvg::i_image::ptr>(L, image__index, 0, ctxidx); // mod ctxtab
    }
    if (!rvg_lua_typeexists<rvg::i_image::const_ptr>(L, ctxidx)) {
        rvg_lua_createtype<rvg::i_image::const_ptr>(L, "const image", ctxidx); // mod ctxtab
    }
    return 0;
}

int rvg_lua_image_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_image_init(L, ctxidx); // mod
    rvg_lua_setmethods<rvg::i_image::const_ptr>(L, const_image__index, 0,
        ctxidx); // mod
    lua_newtable(L); // mod png
    lua_pushvalue(L, ctxidx); // mod png ctxtab
    rvg_lua_setfuncs(L, mod_png, 1); // mod png
    lua_setfield(L, -2, "png"); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_image, 1); // mod
    return 0;
}

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_image(lua_State *L) {
    rvg_lua_init(L); // ctxtab
    int ctxidx = lua_gettop(L);
    lua_newtable(L); // mod
    rvg_lua_image_export(L, ctxidx);
    return 1;
}
