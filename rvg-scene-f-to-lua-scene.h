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
#ifndef RVG_SCENE_F_TO_LUA_SCENE_H
#define RVG_SCENE_F_TO_LUA_SCENE_H

#include "rvg-lua.h"
#include "rvg-i-scene-data.h"

namespace rvg {

// The is a Lua object in the stack, given by its index.
// If the Lua object is guaranteed to remain in the stack from construction to
// destruction of the filter, set REF to false.
// Otherwise, set it to true, which will cause the filter to hold a reference to
// the object until the filter is destroyed.
template <bool REF>
class scene_f_to_lua_scene:
    public i_scene_data<scene_f_to_lua_scene<REF>> {

	lua_State *m_L;
	int m_sink;

public:
	scene_f_to_lua_scene(lua_State *L, int sink):
		m_L(L),
		m_sink(rvg_lua_abs_index(L, sink)) {
        if (REF) {
            lua_pushvalue(m_L, sink);
            m_sink = luaL_ref(m_L, LUA_REGISTRYINDEX);
        }
    }

	scene_f_to_lua_scene(const scene_f_to_lua_scene<REF> &other) = delete;
	scene_f_to_lua_scene &operator=(const scene_f_to_lua_scene<REF> &other) = delete;

    scene_f_to_lua_scene(scene_f_to_lua_scene<REF> &&other):
        m_L{other.m_L},
        m_sink{other.m_sink} {
        other.m_L = nullptr;
        other.m_sink = LUA_NOREF;
    }

    scene_f_to_lua_scene<REF> &operator=(scene_f_to_lua_scene<REF> &&other) {
        m_L = other.m_L;
        m_sink = other.m_sink;
        other.m_L = nullptr;
        other.m_sink = LUA_NOREF;
    }

    ~scene_f_to_lua_scene() {
        if (REF && m_L) {
            luaL_unref(m_L, LUA_REGISTRYINDEX, m_sink);
        }
    }

private:

    void push_helper(void) { ; }

    template <typename ...REST>
    void push_helper(int first, REST ...rest) {
        lua_pushinteger(m_L, first);
        push_helper(rest...);
    }

    template <typename ...REST>
    void push_helper(float first, REST ...rest) {
        lua_pushnumber(m_L, first);
        push_helper(rest...);
    }

    template <typename ...REST>
    void push_helper(const xform &first, REST ...rest) {
        rvg_lua_push<xform>(m_L, first);
        push_helper(rest...);
    }

    template <typename ...REST>
    void push_helper(uint16_t first, REST ...rest) {
        lua_pushinteger(m_L, first);
        push_helper(rest...);
    }

    template <typename ...REST>
    void push_helper(const paint &first, REST ...rest) {
        rvg_lua_push<paint>(m_L, first);
        push_helper(rest...);
    }

    template <typename ...REST>
    void push_helper(const patch<16,4> &first, REST ...rest) {
        rvg_lua_push<patch<16,4>>(m_L, first);
        push_helper(rest...);
    }

    template <typename ...REST>
    void push_helper(const patch<12,4> &first, REST ...rest) {
        rvg_lua_push<patch<12,4>>(m_L, first);
        push_helper(rest...);
    }

    template <typename ...REST>
    void push_helper(const patch<3,3> &first, REST ...rest) {
        rvg_lua_push<patch<3,3>>(m_L, first);
        push_helper(rest...);
    }

    template <typename ...REST>
    void push_helper(const shape &first, REST ...rest) {
        rvg_lua_push<shape>(m_L, first);
        push_helper(rest...);
    }

    template <typename ...REST>
    void push_helper(e_winding_rule first, REST ...rest) {
        rvg_lua_enum_push<e_winding_rule>(m_L, first);
        push_helper(rest...);
    }

	template <typename ...REST>
	void call(const char *method, REST ...rest) {
        if (REF) {
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_sink);
            lua_getfield(m_L, -1, method);
            lua_insert(m_L, -2);
        } else {
            lua_getfield(m_L, m_sink, method);
			lua_pushvalue(m_L, m_sink);
        }
		if (!lua_isnil(m_L, -2)) {
			push_helper(rest...);
			lua_call(m_L, sizeof...(REST)+1, 0);
		} else {
			lua_pop(m_L, 2);
		}
	}

friend i_scene_data<scene_f_to_lua_scene<REF>>;

	void do_painted_shape(e_winding_rule rule, const shape &s,
                const paint &p) {
		call("painted_shape", rule, s, p);
	}

	void do_tensor_product_patch(const patch<16,4> &tpp) {
		call("tensor_product_patch", tpp);
	}

	void do_coons_patch(const patch<12,4> &cp) {
		call("coons_patch", cp);
	}

	void do_gouraud_triangle(const patch<3,3> &gt) {
		call("gouraud_triangle", gt);
	}

	void do_stencil_shape(e_winding_rule rule, const shape &s) {
		call("stencil_shape", rule, s);
	}

	void do_begin_clip(uint16_t depth) {
		call("begin_clip", depth);
	}

	void do_end_clip(uint16_t depth) {
		call("end_clip", depth);
	}

	void do_activate_clip(uint16_t depth) {
		call("activate_clip", depth);
	}

    void do_begin_fade(uint16_t depth, unorm8 opacity) {
        call("begin_fade", depth,
            static_cast<float>(unorm<float>(opacity)));
    }

    void do_end_fade(uint16_t depth, unorm8 opacity) {
        call("end_fade", depth,
            static_cast<float>(unorm<float>(opacity)));
    }

    void do_begin_blur(uint16_t depth, float radius) {
        call("begin_blur", depth, radius);
    }

    void do_end_blur(uint16_t depth, float radius) {
        call("end_blur", depth, radius);
    }

    void do_begin_transform(uint16_t depth, const xform &xf) {
        call("begin_transform", depth, xf);
    }

    void do_end_transform(uint16_t depth, const xform &xf) {
        call("end_transform", depth, xf);
    }
};

static inline auto
make_scene_f_to_lua_scene_in_stack(lua_State *L, int sink) {
    return scene_f_to_lua_scene<false>{L, sink};
}

static inline auto
make_scene_f_to_lua_scene_ref(lua_State *L, int sink) {
    return scene_f_to_lua_scene<true>{L, sink};
}

}

#endif
