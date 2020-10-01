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
#include "rvg-bezier-arc-length.h"
#include "rvg-gaussian-quadrature.h"
#include "rvg-i-point-input-path-f-forwarder.h"
#include "rvg-lua-xform.h"

#include "rvg-lua.h"

#ifdef STROKER_RVG
#include "rvg-stroker-rvg.h"
#endif

#ifdef STROKER_LIVAROT
#include "livarot/rvg-stroker-livarot.h"
#endif

#ifdef STROKER_DIRECT2D
#include "direct2d/rvg-stroker-direct2d.h"
#endif

#ifdef STROKER_QUARTZ
#include "quartz/rvg-stroker-quartz.h"
#endif

#ifdef STROKER_AGG
#include "agg/rvg-stroker-agg.h"
#endif

#ifdef STROKER_SKIA
#include "skia/rvg-stroker-skia.h"
#endif

#ifdef STROKER_QT5
#include "qt5/rvg-stroker-qt5.h"
#endif

#ifdef STROKER_MUPDF
#include "mupdf/rvg-stroker-mupdf.h"
#endif

#ifdef STROKER_CAIRO
#include "cairo/rvg-stroker-cairo.h"
#endif

#ifdef STROKER_GS
#include "gs/rvg-stroker-gs.h"
#endif

#ifdef STROKER_OPENVG_RI
#include "openvg-ri/rvg-stroker-openvg-ri.h"
#endif

using namespace rvg;

static stroke_style::const_ptr default_stroke_style_ptr =
    make_intrusive<stroke_style>();

template <typename STROKER>
int luastroke(lua_State *L, STROKER stroker) {
    auto s = rvg_lua_check<shape>(L, 1);
    auto xf = rvg_lua_check<xform>(L, 2);
    float width = static_cast<float>(luaL_checknumber(L, 3));
    auto style = rvg_lua_opt<stroke_style::const_ptr>(L, 4,
		default_stroke_style_ptr);
    rvg_lua_push<shape>(L, stroker(s, xf, width, style));
    return 1;
}

#ifdef STROKER_QUARTZ
static int luaquartzstroke(lua_State *L) {
    return luastroke(L, stroker::quartz);
}
#endif

#ifdef STROKER_DIRECT2D
static int luadirect2dstroke(lua_State *L) {
    return luastroke(L, stroker::direct2d);
}
#endif

#ifdef STROKER_SKIA
static int luaskiastroke(lua_State *L) {
    return luastroke(L, stroker::skia);
}
#endif

#ifdef STROKER_QT5
static int luaqt5stroke(lua_State *L) {
    return luastroke(L, stroker::qt5);
}
#endif

#ifdef STROKER_MUPDF
static int luamupdfstroke(lua_State *L) {
    return luastroke(L, stroker::mupdf);
}
#endif

#ifdef STROKER_OPENVG_RI
static int luaopenvgristroke(lua_State *L) {
    return luastroke(L, stroker::openvg_ri);
}
#endif

#ifdef STROKER_GS
static int luaghostscriptcompatstroke(lua_State *L) {
    return luastroke(L, stroker::gs_compat);
}

static int luaghostscriptfaststroke(lua_State *L) {
    return luastroke(L, stroker::gs_fast);
}

static int luaghostscriptstroke(lua_State *L) {
    return luastroke(L, stroker::gs);
}
#endif

#ifdef STROKER_AGG
int luaaggstroke(lua_State *L) {
    return luastroke(L, stroker::agg);
}
#endif

#ifdef STROKER_CAIRO
static int luacairotrapsstroke(lua_State *L) {
    return luastroke(L, stroker::cairo_traps);
}

//??D Couldn't yet get this to work properly
static int luacairotristripstroke(lua_State *L) {
    return luastroke(L, stroker::cairo_tristrip);
}

static int luacairopolygonstroke(lua_State *L) {
    return luastroke(L, stroker::cairo_polygon);
}
#endif

#ifdef STROKER_RVG
static int luarvgstroke(lua_State *L) {
    return luastroke(L, stroker::rvg);
}
#endif

#ifdef STROKER_LIVAROT
int lualivarotstrokestroke(lua_State *L) {
    return luastroke(L, stroker::livarot_stroke);
}

int lualivarotoutlinestroke(lua_State *L) {
    return luastroke(L, stroker::livarot_outline);
}
#endif

class input_path_f_push_arc_length final:
    public i_sink<input_path_f_push_arc_length>,
    public i_point_input_path_f_forwarder<input_path_f_push_arc_length> {

    lua_State *m_L;
    rvgf m_length;
    path_f_null m_null;

public:

    explicit input_path_f_push_arc_length(lua_State *L): m_L(L), m_length{0} {
        ;
    }

    ~input_path_f_push_arc_length() {
        lua_pushnumber(m_L, m_length);
    }

private:

friend i_sink<input_path_f_push_arc_length>;

    path_f_null &do_sink(void) {
        return m_null;
    }

    const path_f_null &do_sink(void) const {
        return m_null;
    }

friend i_point_input_path<input_path_f_push_arc_length>;

    void do_linear_segment(const R2 &p0, const R2 &p1) {
        m_length += len(p1-p0);
    }

    void do_quadratic_segment(const R2 &p0, const R2 &p1, const R2 &p2) {
        auto ds2 = make_quadratic_segment_ds2<rvgf>(p0, p1, p2);
        m_length += std::fabs(gaussian_quadrature<rvgf>(
            [&](rvgf t) -> rvgf {
			    return rvgf{std::sqrt(ds2(t))};
		    },
            0, 1, 64
        ));
    }

    void do_rational_quadratic_segment(const R3 &p0, const R3 &p1,
        const R3 &p2) {
        auto ds2 = make_rational_quadratic_segment_ds2<rvgf>(p0, p1, p2);
        m_length += std::fabs(gaussian_quadrature<rvgf>(
            [&](rvgf t) -> rvgf {
			    return rvgf{std::sqrt(ds2(t))};
		    },
            0, 1, 64
        ));
    }

    void do_cubic_segment(const R2 &p0, const R2 &p1, const R2 &p2,
        const R2 &p3) {
        auto ds2 = make_cubic_segment_ds2<rvgf>(p0, p1, p2, p3);
        m_length += std::fabs(gaussian_quadrature<rvgf>(
            [&](rvgf t) -> rvgf {
			    return rvgf{std::sqrt(ds2(t))};
		    },
            0, 1, 64
        ));
    }
};

inline auto make_input_path_f_push_arc_length(lua_State *L) {
    return input_path_f_push_arc_length{L};
}

static int luaarclength(lua_State *L) {
    rvg_lua_check<shape>(L, 1).as_path_data_ptr()->
        iterate(make_input_path_f_push_arc_length(L));
    return 1;
}


// List of Lua functions exported into module table
static const luaL_Reg modother[] = {
    {"arc_length", luaarclength },
#ifdef STROKER_RVG
    {"rvg", luarvgstroke },
#endif
#ifdef STROKER_LIVAROT
    {"livarot_stroke", lualivarotstrokestroke },
    {"livarot_outline", lualivarotoutlinestroke },
#endif
#ifdef STROKER_AGG
    {"agg", luaaggstroke },
#endif
#ifdef STROKER_SKIA
    {"skia", luaskiastroke },
#endif
#ifdef STROKER_QUARTZ
    {"quartz", luaquartzstroke },
#endif
#ifdef STROKER_DIRECT2D
    {"direct2d", luadirect2dstroke },
#endif
#ifdef STROKER_QT5
    {"qt5", luaqt5stroke },
#endif
#ifdef STROKER_MUPDF
    {"mupdf", luamupdfstroke },
#endif
#ifdef STROKER_OPENVG_RI
    {"openvg_ri", luaopenvgristroke },
#endif
#ifdef STROKER_GS
    {"gs_fast", luaghostscriptfaststroke },
    {"gs_compat", luaghostscriptcompatstroke },
    {"gs", luaghostscriptstroke},
#endif
#ifdef STROKER_CAIRO
    {"cairo_traps", luacairotrapsstroke },
    {"cairo_tristrip", luacairotristripstroke },
    {"cairo_polygon", luacairopolygonstroke },
#endif
    {NULL, NULL}
};

// Lua function invoked to be invoked by require"strokers"
extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_strokers(lua_State *L) {
    lua_newtable(L); // strokers
	rvg_lua_init(L); // strokers ctxtab
    rvg_lua_setfuncs(L, modother, 1); // strokers
    return 1;
}
