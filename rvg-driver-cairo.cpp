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
#include <cairo.h>

#include <string>
#include <sstream>
#include <cmath>
#include <memory>

#include "rvg-lua.h"

#include "rvg-i-input-path.h"
#include "rvg-i-scene-data.h"
#include "rvg-stroke-style.h"

#include "rvg-input-path-f-xform.h"
#include "rvg-rgba.h"

#include "rvg-lua-facade.h"

#include "rvg-driver-cairo.h"


namespace rvg {
    namespace driver {
        namespace cairo {

class accelerated {
    std::shared_ptr<cairo_surface_t> m_surface;
public:
    accelerated(cairo_surface_t *surface): m_surface(surface, &cairo_surface_destroy) { }
    cairo_surface_t * get_surface(void) const { return m_surface.get(); }
    cairo_surface_t * get_surface(void) { return m_surface.get(); }
};

class input_path_f_to_cairo final: public i_input_path<input_path_f_to_cairo> {

    cairo_t *m_cr;

public:
    input_path_f_to_cairo(cairo_t *cr);

private:

friend i_input_path<input_path_f_to_cairo>;

    void do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1);
    void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2);
    void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2);
    void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3);
    void do_begin_contour(rvgf x0, rvgf y0);
    void do_end_open_contour(rvgf x0, rvgf y0);
    void do_end_closed_contour(rvgf x0, rvgf y0);
};

inline auto make_input_path_f_to_cairo(cairo_t *cr) {
    return input_path_f_to_cairo{cr};
}

class scene_f_to_cairo final: public i_scene_data<scene_f_to_cairo> {

    cairo_t *m_cr;
    std::vector<xform> m_xf_stack;

public:

    scene_f_to_cairo(const xform &screen_xf, cairo_t *cr);

private:

    void push_xf(const xform &xf);
    void pop_xf(void);
    const xform &top_xf(void) const;

    void set_path(const path_data &p, const xform &pre_xf);
    void set_stroke_style(float width, const stroke_style &st);

    friend i_scene_data<scene_f_to_cairo>;

    void do_painted_shape(e_winding_rule wr, const shape &s, const paint &p);
    void do_tensor_product_patch(const patch<16,4> &tpp);
    void do_coons_patch(const patch<12,4> &cp);
    void do_gouraud_triangle(const patch<3,3> &gt);
    void do_stencil_shape(e_winding_rule wr, const shape &s);
    void do_begin_clip(uint16_t depth);
    void do_activate_clip(uint16_t depth);
    void do_end_clip(uint16_t depth);
    void do_begin_fade(uint16_t depth, unorm8 opacity);
    void do_end_fade(uint16_t depth, unorm8 opacity);
    void do_begin_blur(uint16_t depth, float radius);
    void do_end_blur(uint16_t depth, float radius);
    void do_begin_transform(uint16_t depth, const xform &xf);
    void do_end_transform(uint16_t depth, const xform &xf);

};

static cairo_matrix_t xf_to_cairo(const xform &xf) {
    cairo_matrix_t cm;
    cm.xx = xf[0][0];
    cm.xy = xf[0][1];
    cm.x0 = xf[0][2];
    cm.yx = xf[1][0];
    cm.yy = xf[1][1];
    cm.y0 = xf[1][2];
    return cm;
}

input_path_f_to_cairo::input_path_f_to_cairo(cairo_t *cr): m_cr(cr) { ; }

void input_path_f_to_cairo::do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
    (void) x0; (void) y0;
    cairo_line_to(m_cr, x1, y1);
}

void input_path_f_to_cairo::do_quadratic_segment(rvgf x0, rvgf y0,
    rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
    // elevate quadratic to cubic
    rvgf u1 = (x0+rvgf{2}*x1)/rvgf{3}, v1 = (y0+rvgf{2}*y1)/rvgf{3};
    rvgf u2 = (x2+rvgf{2}*x1)/rvgf{3}, v2 = (y2+rvgf{2}*y1)/rvgf{3};
    cairo_curve_to(m_cr, u1, v1, u2, v2, x2, y2);
}

void input_path_f_to_cairo::do_rational_quadratic_segment(rvgf x0, rvgf y0,
    rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
    const rvgf c = w1; // in elipse, this is cos of the half arc
    const rvgf s2 = rvgf{1}-c*c; // s2 with sign
    if (!util::is_almost_zero(s2)) {
        rvgf s = std::copysign(std::sqrt(std::fabs(s2)), s2);
        // affine transformation mapping unit circle to our arc
        cairo_matrix_t T;
        T.xx = (rvgf{2}*x1-c*(x0+x2))/(rvgf{2}*s2);
        T.xy = (s*(x2-x0))/(rvgf{2}*s2);
        T.x0 = (x0+x2-rvgf{2}*c*x1)/(rvgf{2}*s2);
        T.yx = (rvgf{2}*y1-c*(y0+y2))/(rvgf{2}*s2);
        T.yy = (s*(y2-y0))/(rvgf{2}*s2);
        T.y0 = (y0+y2-rvgf{2}*c*y1)/(rvgf{2}*s2);
        rvgf a2 = std::atan2(s, c);
        rvgf a1 = -a2;
        cairo_matrix_t P;
        cairo_get_matrix(m_cr, &P);
        cairo_transform(m_cr, &T);
        cairo_arc(m_cr, 0., 0., 1., a1, a2);
        cairo_set_matrix(m_cr, &P);
    } else {
        do_quadratic_segment(x0, y0, x1, y1, x2, y2);
    }
}

void input_path_f_to_cairo::do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
    rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
    (void) x0; (void) y0;
    cairo_curve_to(m_cr, x1, y1, x2, y2, x3, y3);
}

void input_path_f_to_cairo::do_begin_contour(rvgf x0, rvgf y0) {
    cairo_move_to(m_cr, x0, y0);
}

void input_path_f_to_cairo::do_end_open_contour(rvgf x0, rvgf y0) {
    (void) x0; (void) y0;
}

void input_path_f_to_cairo::do_end_closed_contour(rvgf x0, rvgf y0) {
    (void) x0; (void) y0;
    cairo_close_path(m_cr);
}

scene_f_to_cairo::scene_f_to_cairo(
    const xform &screen_xf,
    cairo_t *cr):
    m_cr(cr) {
    push_xf(screen_xf);
}

void scene_f_to_cairo::push_xf(const xform &xf) {
    m_xf_stack.push_back(top_xf() * xf);
    auto cm = xf_to_cairo(top_xf());
    cairo_set_matrix(m_cr, &cm);
}

void scene_f_to_cairo::pop_xf(void) {
    if (m_xf_stack.size() > 0) {
        m_xf_stack.pop_back();
    }
    auto cm = xf_to_cairo(top_xf());
    cairo_set_matrix(m_cr, &cm);
}

const xform &scene_f_to_cairo::top_xf(void) const {
    static xform id;
    if (m_xf_stack.empty()) return id;
    else return m_xf_stack.back();
}

void scene_f_to_cairo::set_path(const path_data &p, const xform &pre_xf) {
    if (!pre_xf.is_identity()) {
        p.iterate(
            make_input_path_f_xform(pre_xf,
                make_input_path_f_to_cairo(m_cr)));
    } else {
        p.iterate(make_input_path_f_to_cairo(m_cr));
    }
}

static void set_cairo_pattern_ramp(cairo_pattern_t *pattern, double opacity, const color_ramp& ramp) {
    switch (ramp.get_spread()) {
        case e_spread::clamp: cairo_pattern_set_extend(pattern, CAIRO_EXTEND_PAD); break;
        case e_spread::wrap: cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT); break;
        case e_spread::mirror: cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REFLECT); break;
        case e_spread::transparent: cairo_pattern_set_extend(pattern, CAIRO_EXTEND_NONE); break;
    }

    for (auto stop : ramp.get_color_stops()) {
        double t = stop.get_offset();
        auto c = stop.get_color();
        double a = opacity * double(c[3]) / 255.f;
        cairo_pattern_add_color_stop_rgba(pattern, t, double(c[0])/255.f, double(c[1])/255.f, double(c[2])/255.f, a);
    }
}

static cairo_pattern_t * paint_to_cairo_pattern(const paint &p, const xform &xf) {
    double opacity = double(p.get_opacity()) / 255.f;
    switch (p.get_type()) {
        case paint::e_type::solid_color: {
            auto c = p.get_solid_color();
            return cairo_pattern_create_rgba(double(c[0])/255.f, double(c[1])/255.f, double(c[2])/255.f, opacity * double(c[3])/255.f);
        }
        case paint::e_type::linear_gradient: {
            auto &lg = p.get_linear_gradient_data();
            float x1, y1, x2, y2, w;
            std::tie(x1, y1, w) = p.get_xf().transformed(xf).apply(lg.get_x1(), lg.get_y1());
            std::tie(x2, y2, w) = p.get_xf().transformed(xf).apply(lg.get_x2(), lg.get_y2());
            auto pattern = cairo_pattern_create_linear(x1, y1, x2, y2);
            set_cairo_pattern_ramp(pattern, opacity, lg.get_color_ramp());
            return pattern;
        }
        case paint::e_type::radial_gradient: {
            auto &rg = p.get_radial_gradient_data();
            auto pattern = cairo_pattern_create_radial(rg.get_fx(), rg.get_fy(), 0,  rg.get_cx(), rg.get_cy(), rg.get_r());

            auto ixf = p.get_xf().transformed(xf).inverse();
            auto m = xf_to_cairo(ixf);
            cairo_pattern_set_matrix(pattern, &m);
            set_cairo_pattern_ramp(pattern, opacity, rg.get_color_ramp());
            return pattern;
        }
        case paint::e_type::texture:
        default:
            return nullptr;
    }
}

void scene_f_to_cairo::set_stroke_style(float width,
    const stroke_style &st) {
    cairo_set_line_width(m_cr, width);
    switch (st.get_join()) {
        case e_stroke_join::miter_clip: // cairo does not have clipped miter
        case e_stroke_join::miter_or_bevel: // only fallbacks to bevel
            cairo_set_line_join(m_cr, CAIRO_LINE_JOIN_MITER);
            break;
        case e_stroke_join::round:
            cairo_set_line_join(m_cr, CAIRO_LINE_JOIN_ROUND);
            break;
        case e_stroke_join::bevel:
        default:
            cairo_set_line_join(m_cr, CAIRO_LINE_JOIN_BEVEL);
            break;
    }
    cairo_set_miter_limit(m_cr, st.get_miter_limit());
    switch (st.get_initial_cap()) {
        case e_stroke_cap::butt:
            cairo_set_line_cap(m_cr, CAIRO_LINE_CAP_BUTT);
            break;
        case e_stroke_cap::round:
            cairo_set_line_cap(m_cr, CAIRO_LINE_CAP_ROUND);
            break;
        case e_stroke_cap::square:
            cairo_set_line_cap(m_cr, CAIRO_LINE_CAP_SQUARE);
            break;
        default:
            break;
    }
    std::vector<double> dashes;
    for (auto d: st.get_dashes()) dashes.push_back(d*width);
    cairo_set_dash(m_cr, &dashes[0], dashes.size(), st.get_dash_offset()*width);
}

void scene_f_to_cairo::do_tensor_product_patch(const patch<16,4> &) {
    ;
}

void scene_f_to_cairo::do_coons_patch(const patch<12,4> &) {
    ;
}

void scene_f_to_cairo::do_gouraud_triangle(const patch<3,3> &) {
    ;
}

void scene_f_to_cairo::do_painted_shape(e_winding_rule wr,
    const shape &s, const paint &p) {
    const stroke_style *st = nullptr;
    float w = 0.f;
    xform pre_xf;
    path_data::const_ptr path_shape;
    push_xf(s.get_xf());
    if (s.is_stroke()) {
        st = &s.get_stroke_data().get_style();
        w = s.get_stroke_data().get_width();
        pre_xf = s.get_stroke_data().get_shape().get_xf();
        // convert shape to be stroked into a path
        path_shape = s.get_stroke_data().get_shape().as_path_data_ptr(
            pre_xf.transformed(top_xf()));
    } else {
        path_shape = s.as_path_data_ptr(top_xf());
    }

    auto pattern = paint_to_cairo_pattern(p, s.get_xf().inverse().transformed(pre_xf));
    cairo_set_source(m_cr, pattern);
    set_path(*path_shape, pre_xf);
    if (st) {
        set_stroke_style(w, *st);
        cairo_stroke(m_cr);
    } else {
        if (wr == e_winding_rule::odd) {
            cairo_set_fill_rule(m_cr, CAIRO_FILL_RULE_EVEN_ODD);
        } else {
            cairo_set_fill_rule(m_cr, CAIRO_FILL_RULE_WINDING);
        }
        cairo_fill(m_cr);
    }
    cairo_pattern_destroy(pattern);
    pop_xf();
}

void scene_f_to_cairo::do_stencil_shape(e_winding_rule wr, const shape &s) {
    (void) wr; (void) s;
}

void scene_f_to_cairo::do_begin_clip(uint16_t depth) {
    (void) depth;
}

void scene_f_to_cairo::do_activate_clip(uint16_t depth) {
    (void) depth;
}

void scene_f_to_cairo::do_end_clip(uint16_t depth) {
    (void) depth;
}

void scene_f_to_cairo::do_begin_fade(uint16_t depth, unorm8 opacity) {
    (void) depth; (void) opacity;
}

void scene_f_to_cairo::do_end_fade(uint16_t depth, unorm8 opacity) {
    (void) depth; (void) opacity;
}

void scene_f_to_cairo::do_begin_blur(uint16_t depth, float radius) {
    (void) depth; (void) radius;
}

void scene_f_to_cairo::do_end_blur(uint16_t depth, float radius) {
    (void) depth; (void) radius;
}

void scene_f_to_cairo::do_begin_transform(uint16_t depth, const xform &xf) {
    (void) depth;
    push_xf(xf);
}

void scene_f_to_cairo::do_end_transform(uint16_t depth, const xform &xf) {
    (void) depth;
    (void) xf;
    pop_xf();
}

const accelerated accelerate(const scene &c, const window &w,
    const viewport &v) {
    int xl, yb, xr, yt;
    std::tie(xl, yb) = v.bl();
    std::tie(xr, yt) = v.tr();
    xform flip = make_translation(0,-static_cast<rvgf>(yb)).
        scaled(1,-1).translated(0,static_cast<rvgf>(yt));
    xform screen_xf = flip * c.get_xf().windowviewport(w, v).translated(-xl, yb);

    cairo_surface_t *surface = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, nullptr);
    cairo_t *cr = cairo_create(surface);
    scene_f_to_cairo cs(screen_xf, cr);
    c.get_scene_data().iterate(cs);
    cairo_surface_flush(surface);
    cairo_destroy(cr);
    return surface;
}

static cairo_status_t write_fn(void *closure, const unsigned char *data,
    unsigned int len) {
    if (fwrite(data, 1, len, reinterpret_cast<FILE *>(closure)) == len)
        return CAIRO_STATUS_SUCCESS;
    else
        return CAIRO_STATUS_WRITE_ERROR;
}

static bool opt_no_output(const std::vector<std::string> &args) {
    for (const auto &s : args)
        if (s.compare("-no-output") == 0) return true;

    return false;
}

void render(const accelerated &a, const window &w, const viewport &v,
    FILE *out, const std::vector<std::string> &args) {
    (void) args;
    (void) w;
    int xl, yb, xr, yt;
    std::tie(xl, yb) = v.bl();
    std::tie(xr, yt) = v.tr();


    cairo_surface_t *surface = (cairo_surface_t *)
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, std::fabs(xl-xr),
            std::fabs(yt-yb));
    cairo_t *cr = cairo_create(surface);
    cairo_set_source_surface(cr, a.get_surface(), 0., 0.);
    cairo_paint(cr);
    cairo_surface_flush(surface);

    if (!opt_no_output(args))
        cairo_surface_write_to_png_stream(surface, write_fn, out);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

} } } // namespace rvg::driver::cairo

// Lua version of the accelerate function.
// Since there is no acceleration, we simply
// and return the input scene unmodified.
static int luaaccelerate(lua_State *L) {
    rvg_lua_push<rvg::driver::cairo::accelerated>(L,
        rvg::driver::cairo::accelerate(
            rvg_lua_check<rvg::scene>(L, 1),
            rvg_lua_check<rvg::window>(L, 2),
            rvg_lua_check<rvg::viewport>(L, 3)));
    return 1;
}

// Lua version of render function
static int luarender(lua_State *L) {
    auto a = rvg_lua_check<rvg::driver::cairo::accelerated>(L, 1);
    auto w = rvg_lua_check<rvg::window>(L, 2);
    auto v = rvg_lua_check<rvg::viewport>(L, 3);
    auto o = rvg_lua_optargs(L, 5);
    rvg::driver::cairo::render(a, w, v, rvg_lua_check_file(L, 4), o);
    return 0;
}

// List of Lua functions exported into driver table
static const luaL_Reg modcairopng[] = {
    {"render", luarender },
    {"accelerate", luaaccelerate },
    {NULL, NULL}
};

// Lua function invoked to be invoked by require"driver.cairo"
extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_driver_cairo(lua_State *L) {
    rvg_lua_init(L);
    if (!rvg_lua_typeexists<rvg::driver::cairo::accelerated>(L, -1)) {
        rvg_lua_createtype<rvg::driver::cairo::accelerated>(L,
            "cairo accelerated", -1);
    }
    rvg_lua_facade_new_driver(L, modcairopng);
    return 1;
}
