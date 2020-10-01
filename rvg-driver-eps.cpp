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
#include <sstream>

#include "rvg-lua.h"

#include "rvg-xform.h"
#include "rvg-input-path-f-xform.h"

#include "rvg-lua-facade.h"

#include "rvg-driver-eps.h"

namespace rvg {
    namespace driver {
        namespace eps {

void print_xform(const xform &xf, std::ostream &out) {
    if (!xf.is_identity())
        out << "[" << xf[0][0] << ' ' << xf[1][0] << ' '
                    << xf[0][1] << ' ' << xf[1][1] << ' '
                    << xf[0][2] << ' ' << xf[1][2] << "] concat\n";
}


class input_path_f_print_eps final:
    public i_input_path<input_path_f_print_eps> {

    std::ostream &m_out;

public:
    input_path_f_print_eps(std::ostream &out): m_out(out) { }

private:

friend i_input_path<input_path_f_print_eps>;

    void do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
        (void) x0; (void) y0;
		m_out << x1 << ' ' << y1 << " lineto\n";
    }

    void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2) {
        // elevate quadratic to cubic
        rvgf u1 = (x0+rvgf{2}*x1)/rvgf{3}, v1 = (y0+rvgf{2}*y1)/rvgf{3};
        rvgf u2 = (x2+rvgf{2}*x1)/rvgf{3}, v2 = (y2+rvgf{2}*y1)/rvgf{3};
		m_out << u1 << ' ' << v1 << ' '
			  << u2 << ' ' << v2 << ' '
		      << x2 << ' ' << y2 << " curveto\n";
    }

    void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2) {
        if (util::is_almost_zero(util::det(x0, y0, x1, y1, w1, x2, y2))) {
            return do_linear_segment(x0, y0, x2, y2);
        }
        const rvgf cos = w1; // in elipse, this is cos of the half arc
        const rvgf sin2 = rvgf{1}-cos*cos; // sin2 with sign
        if (util::is_almost_zero(sin2)) {
            return do_quadratic_segment(x0, y0, x1, y1, x2, y2);
        }
        rvgf sin = std::copysign(std::sqrt(std::fabs(sin2)), sin2);
        // affine transformation mapping unit circle to our arc
        rvgf a = (rvgf{2}*x1-cos*(x0+x2))/(rvgf{2}*sin2);
        rvgf b = (sin*(x2-x0))/(rvgf{2}*sin2);
        rvgf tx = (x0+x2-rvgf{2}*cos*x1)/(rvgf{2}*sin2);
        rvgf c = (rvgf{2}*y1-cos*(y0+y2))/(rvgf{2}*sin2);
        rvgf d = (sin*(y2-y0))/(rvgf{2}*sin2);
        rvgf ty = (y0+y2-rvgf{2}*cos*y1)/(rvgf{2}*sin2);
        auto xf = make_affinity(a,b,tx,c,d,ty);
        rvgf ang = util::deg(std::atan2(sin, cos));
        m_out << "matrix currentmatrix\n";
        print_xform(xf, m_out);
        m_out << "0 0 1 " << -ang << ' ' << ang << " arc\n";
        m_out << "setmatrix\n";
    }

    void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        (void) x0; (void) y0;
		m_out << x1 << ' ' << y1 << ' '
			  << x2 << ' ' << y2 << ' '
		      << x3 << ' ' << y3 << " curveto\n";
    }

    void do_begin_contour(rvgf x0, rvgf y0) {
		m_out << x0 << ' ' << y0 << " moveto\n";
    }

    void do_end_open_contour(rvgf x0, rvgf y0) {
        (void) x0; (void) y0;
    }

    void do_end_closed_contour(rvgf x0, rvgf y0) {
        (void) x0;
        (void) y0;
		m_out << "closepath\n";
    }
};

inline auto make_input_path_f_print_eps(std::ostream &out) {
    return input_path_f_print_eps{out};
}

static void print_path_data(const path_data &p, const xform &pre_xf,
    std::ostream &out) {
    if (!pre_xf.is_identity()) {
        p.iterate(
            make_input_path_f_xform(pre_xf,
                make_input_path_f_print_eps(out)));
    } else {
        p.iterate(make_input_path_f_print_eps(out));
    }
}

void print_stroke_style(float width, const stroke_style &st,
    std::ostream &out) {
    out << "[ ";
    for (float d: st.get_dashes()) out << d*width << ' ';
    out << "] " << st.get_dash_offset()*width << " setdash\n";
    switch (st.get_join()) {
        case e_stroke_join::miter_clip: // ps does not have clipped mitters
        case e_stroke_join::miter_or_bevel: // only fallbacks to bevel
            out << st.get_miter_limit() << " setmiterlimit\n";
            out << "0 setlinejoin\n";
            break;
        case e_stroke_join::round:
            out << "1 setlinejoin\n";
            break;
        default:
            out << "2 setlinejoin\n";
            break;
    }
    switch (st.get_initial_cap()) {
        case e_stroke_cap::round:
            out << "1 setlinecap\n";
            break;
        case e_stroke_cap::square:
            out << "2 setlinecap\n";
            break;
        default:
            out << "0 setlinecap\n";
            break;
    }
    out << width << " setlinewidth\n";
}

void print_rgba8(const RGBA8 &c, std::ostream &out) {
    out << unorm<float>{c[0]} << ' ' <<
           unorm<float>{c[1]} << ' ' <<
           unorm<float>{c[2]} << " setrgbcolor\n";
}

void print_solid_color(const RGBA8 &c, unorm8 opacity,
    std::ostream &out) {
    (void) opacity;
    print_rgba8(c, out);
}

void print_paint(const paint &p, std::ostream &out) {
    switch (p.get_type()) {
        case paint::e_type::solid_color:
            print_solid_color(p.get_solid_color(), p.get_opacity(), out);
            break;
        default:
            std::cerr << "not supported\n";
            break;
    }
}

class scene_f_print_eps final: public i_scene_data<scene_f_print_eps> {

    std::ostream &m_out;
    std::vector<xform> m_xf_stack;

public:
    scene_f_print_eps(const xform &screen_xf, std::ostream &out):
        m_out(out) {
        m_xf_stack.push_back(screen_xf);
    }

private:

    friend i_scene_data<scene_f_print_eps>;

    void do_painted_shape(e_winding_rule wr, const shape &s, const paint &p) {
        const char *mode = nullptr;
        const stroke_style *st = nullptr;
        float w = 0.f;
        path_data::const_ptr path_shape;
        xform pre_xf;
        xform post_xf = s.get_xf();
        if (s.is_stroke()) {
            mode = "stroke";
            st = &s.get_stroke_data().get_style();
            w = s.get_stroke_data().get_width();
            pre_xf = s.get_stroke_data().get_shape().get_xf();
            path_shape = s.get_stroke_data().get_shape().as_path_data_ptr(
                pre_xf.transformed(m_xf_stack.back()));
        } else {
            mode = (wr == e_winding_rule::non_zero)? "fill":
				"eofill";
            path_shape = s.as_path_data_ptr(m_xf_stack.back());
        }
        m_out << "gsave\n";
        if (st) {
            print_stroke_style(w, *st, m_out);
        }
        print_xform(post_xf, m_out);
        print_paint(p, m_out);
        print_path_data(*path_shape, pre_xf, m_out);
		m_out << mode << '\n';
        m_out << "grestore\n";
    }

    void do_tensor_product_patch(const patch<16,4> &tpp) {
        m_out << "<<\n/ShadingType 7\n/ColorSpace [ /DeviceRGB ]\n";
        m_out << "/DataSource [\n";
        const auto &points = tpp.get_patch_data().get_points();
        const auto &colors = tpp.get_patch_data().get_colors();
        m_out << "0\n";
        for (const auto &p: points) {
            m_out << p[0] << ' ' << p[1] << "\n";
        }
        for (const auto &c: colors) {
            m_out << unorm<float>{c[0]} << ' ' <<
               unorm<float>{c[1]} << ' ' <<
               unorm<float>{c[2]} << "\n";
        }
        m_out << "]\n>> shfill\n";
    }

    void do_coons_patch(const patch<12,4> &cp) {
        m_out << "<<\n/ShadingType 6\n/ColorSpace [ /DeviceRGB ]\n";
        m_out << "/DataSource [\n";
        const auto &points = cp.get_patch_data().get_points();
        const auto &colors = cp.get_patch_data().get_colors();
        m_out << "0\n";
        for (const auto &p: points) {
            m_out << p[0] << ' ' << p[1] << "\n";
        }
        for (const auto &c: colors) {
            m_out << unorm<float>{c[0]} << ' ' <<
               unorm<float>{c[1]} << ' ' <<
               unorm<float>{c[2]} << "\n";
        }
        m_out << "]\n>> shfill\n";
    }

    void do_gouraud_triangle(const patch<3,3> &gt) {
        m_out << "<<\n/ShadingType 4\n/ColorSpace [ /DeviceRGB ]\n";
        m_out << "/DataSource [\n";
        const auto &points = gt.get_patch_data().get_points();
        const auto &colors = gt.get_patch_data().get_colors();
        for (int i = 0; i < 3; i++) {
            m_out << "0\n";
            m_out << points[i][0] << ' ' << points[i][1] << "\n";
            m_out << unorm<float>{colors[i][0]} << ' ' <<
               unorm<float>{colors[i][1]} << ' ' <<
               unorm<float>{colors[i][2]} << "\n";
        }
        m_out << "]\n>> shfill\n";
    }

    void do_stencil_shape(e_winding_rule wr, const shape &s) {
        (void) wr;
        (void) s;
    }

    void do_begin_clip(uint16_t depth) {
        (void) depth;
    }

    void do_activate_clip(uint16_t depth) {
        (void) depth;
    }

    void do_end_clip(uint16_t depth) {
        (void) depth;
    }

    void do_begin_fade(uint16_t depth, unorm8 opacity) {
        (void) depth; (void) opacity;
    }

    void do_end_fade(uint16_t depth, unorm8 opacity) {
        (void) depth; (void) opacity;
    }

    void do_begin_blur(uint16_t depth, float radius) {
        (void) depth; (void) radius;
    }

    void do_end_blur(uint16_t depth, float radius) {
        (void) depth; (void) radius;
    }

    void do_begin_transform(uint16_t depth, const xform &xf) {
        (void) depth;
        m_out << "gsave\n";
        m_xf_stack.push_back(xf.transformed(m_xf_stack.back()));
        print_xform(xf, m_out);
    }

    void do_end_transform(uint16_t depth, const xform &xf) {
        (void) depth; (void) xf;
        m_out << "grestore\n";
        m_xf_stack.pop_back();
    }
};


const scene &accelerate(const scene &c, const window &w,
    const viewport &v) {
    (void) w;
    (void) v;
    return c;
}

void render(const scene &c, const window &w, const viewport &v,
    std::ostream &out, const std::vector<std::string> &args) {
    (void) args;
    auto screen_xf = make_windowviewport(w, v)*c.get_xf();
    scene_f_print_eps sp(screen_xf, out);
    out << "%!PS-Adobe-3.0 EPSF-3.0\n";
    const auto &bb = v.corners();
    out << "%%BoundingBox: " << bb[0] << ' ' << bb[1] << ' ' <<
                                bb[2] << ' ' << bb[3] << '\n';
    print_xform(screen_xf, out);
    c.get_scene_data().iterate(sp);
}

} } } // namespace rvg::driver::cpp

// Lua version of the accelerate function.
// Since there is no acceleration, we simply
// and return the input scene unmodified.
static int luaaccelerate(lua_State *L) {
    lua_settop(L, 1);
    return 1;
}

// Lua version of the render function
static int luarender(lua_State *L) {
    auto c = rvg_lua_check<rvg::scene>(L, 1);
    auto w = rvg_lua_check<rvg::window>(L, 2);
    auto v = rvg_lua_check<rvg::viewport>(L, 3);
    FILE *f = rvg_lua_check_file(L, 4);
    std::ostringstream sout;
    rvg::driver::eps::render(c, w, v, sout);
    fwrite(sout.str().data(), 1, sout.str().size(), f);
    return 0;
}

// List of Lua functions exported into driver table
static const luaL_Reg modeps[] = {
    {"render", luarender },
    {"accelerate", luaaccelerate },
    {NULL, NULL}
};

// Lua function invoked to be invoked by require"driver.eps"
extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_driver_eps(lua_State *L) {
    rvg_lua_facade_new_driver(L, modeps);
    return 1;
}
