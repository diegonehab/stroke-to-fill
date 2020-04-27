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
#include "rvg-named-colors.h"
#include "rvg-input-path-f-to-svg-path.h"
#include "rvg-svg-path-f-command-printer.h"
#include "rvg-pngio.h"
#include "rvg-base64.h"

#include "rvg-scene-f-print-rvg.h"

#include <boost/range/adaptor/sliced.hpp>

namespace rvg {

template <typename FLAVOR>
scene_f_print_rvg<FLAVOR>::
scene_f_print_rvg(std::ostream &out):
    m_out(out),
    m_nl(2) {
    ;
}

template <typename FLAVOR>
const char *
scene_f_print_rvg<FLAVOR>::
class_member_operator(void) {
    return m_flavor.class_member_operator();
}

template <typename FLAVOR>
const char *
scene_f_print_rvg<FLAVOR>::
scope_resolution_operator(void) {
    return m_flavor.scope_resolution_operator();
}

template <typename FLAVOR>
const char *
scene_f_print_rvg<FLAVOR>::
begin_raw_string_literal(void) {
    return m_flavor.begin_raw_string_literal();

}

template <typename FLAVOR>
const char *
scene_f_print_rvg<FLAVOR>::
end_raw_string_literal(void) {
    return m_flavor.end_raw_string_literal();
}

template <typename FLAVOR>
float
scene_f_print_rvg<FLAVOR>::
opacity_scale(unorm8 o) {
    return m_flavor.opacity_scale(o);
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_rotation(float c, float s, std::ostream &out) {
    if (!util::is_almost_one(c) || !util::is_almost_zero(s)) {
        out << class_member_operator()
            << "rotated(" << util::deg(std::atan2(s, c)) << ')';
    }
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_scaling(float sx, float sy, std::ostream &out) {
    if (util::is_almost_equal(sx, sy)) {
        float avg = 0.5f*(sx+sy);
        if (!util::is_almost_one(avg)) {
            out << class_member_operator()
                << "scaled(" << avg << ')';
        }
    } else {
        out << class_member_operator()
            << "scaled(" << sx << ',' << sy << ')';
    }
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_translation(float tx, float ty, std::ostream &out) {
    if (!util::is_almost_zero(tx) || !util::is_almost_zero(ty)) {
        out << class_member_operator()
            << "translated(" << tx << ',' << ty << ')';
    }
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_linear(float a, float b, float c, float d, std::ostream &out) {
    if (util::is_almost_zero(a*b+c*d) && util::is_almost_one(a*d-b*c) &&
        util::is_almost_one(a*a+c*c) && util::is_almost_one(b*b+d*d)) {
        return print_rotation(a, c, out);
    } else if (util::is_almost_zero(b) && util::is_almost_zero(c)) {
        return print_scaling(a, d, out);
    } else {
        out << class_member_operator()
            << "linear(" << a << ',' << b << ',' << c << ',' << d << ')';
    }
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_affinity(float a, float b, float tx,
   float c, float d, float ty, std::ostream &out) {
    if (util::is_almost_zero(tx) && util::is_almost_zero(ty)) {
        return print_linear(a, b, c, d, out);
    } else {
        if (util::is_almost_one(a) && util::is_almost_zero(b) &&
            util::is_almost_zero(c) && util::is_almost_one(d)) {
            return print_translation(tx, ty, out);
        } else {
            out << class_member_operator()
                << "affine(" << a << ',' << b << ',' << tx << ','
                  << c << ',' << d << ',' << ty << ')';
        }
    }
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_projectivity(float a, float b, float c,
   float d, float e, float f,
   float g, float h, float i, std::ostream &out) {
    if (util::is_almost_zero(g) && util::is_almost_zero(h) &&
        util::is_almost_one(i)) {
        return print_affinity(a, b, c, d, e, f, out);
    } else {
        out << class_member_operator()
            << "projected(" << a << ',' << b << ',' << c << ','
            << d << ',' << e << ',' << f << ','
            << g << ',' << h << ',' << i << ')';
    }
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_xform(const xform &xf, std::ostream &out) {
    print_affinity(xf[0][0], xf[0][1], xf[0][2], xf[1][0], xf[1][1],
        xf[1][2], out);
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_path(const path_data &p, std::ostream &out) {
    out << "path({";
    p.iterate(
        make_input_path_f_to_svg_path(
            make_svg_path_f_command_printer(out, ',')));
    out << "})";
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_circle(const circle_data &c, std::ostream &out) {
    out << "circle(" << c.get_cx() << ',' << c.get_cy()
        << ',' << c.get_r()  << ')';
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_triangle(const triangle_data &t, std::ostream &out) {
    out << "triangle("
        << t.get_x1() << ',' << t.get_y1() << ','
        << t.get_x2() << ',' << t.get_y2() << ','
        << t.get_x3() << ',' << t.get_y3() << ')';
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_rect(const rect_data &r, std::ostream &out) {
    out << "rect("
        << r.get_x() << ',' << r.get_y() << ','
        << r.get_width() << ',' << r.get_height() << ')';
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_polygon(const polygon_data &p, std::ostream &out) {
    out << "polygon({";
    const auto &c = p.get_coordinates();
    if (!c.empty()) {
        out << c[0];
        for (unsigned i = 1; i < c.size(); ++i) {
            out << ',' << c[i];
        }
    }
    out << "})";
}

template <typename FLAVOR>
const char *
scene_f_print_rvg<FLAVOR>::
cap_name(e_stroke_cap cap) {
    switch (cap) {
        case e_stroke_cap::butt: return "butt";
        case e_stroke_cap::round: return "round";
        case e_stroke_cap::triangle: return "triangle";
        case e_stroke_cap::fletching: return "fletching";
        case e_stroke_cap::square: return "square";
        default: return "butt";
    }
}

template <typename FLAVOR>
const char *
scene_f_print_rvg<FLAVOR>::
join_name(e_stroke_join join) {
    switch (join) {
        case e_stroke_join::miter_clip: return "miter_clip";
        case e_stroke_join::round: return "round";
        case e_stroke_join::bevel: return "bevel";
        case e_stroke_join::miter_or_bevel: return "miter_or_bevel";
        case e_stroke_join::arcs: return "arcs";
        default: return "bevel";
    }
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_style(float width, const stroke_style &st, std::ostream &out) {
    out << class_member_operator()
        << "stroked(" << width << ')';
    if (st.get_miter_limit() != stroke_style::default_miter_limit) {
        out << class_member_operator()
            << "miter_limited(" << st.get_miter_limit() << ')';
    }
    if (st.get_join() != stroke_style::default_join) {
        out << class_member_operator()
            << "joined(stroke_join"
            << scope_resolution_operator()
            << join_name(st.get_join())
            << ')';
    }
    if (st.get_inner_join() != stroke_style::default_inner_join) {
        out << class_member_operator()
            << "inner_joined(stroke_join"
            << scope_resolution_operator()
            << join_name(st.get_inner_join())
            << ')';
    }
    if (st.get_initial_cap() == st.get_terminal_cap() &&
        st.get_initial_cap() == st.get_dash_initial_cap() &&
        st.get_initial_cap() == st.get_dash_terminal_cap()) {
        if (st.get_initial_cap() != stroke_style::default_cap) {
            out << class_member_operator()
                << "capped(stroke_cap"
                << scope_resolution_operator()
                << cap_name(st.get_initial_cap()) << ")";
        }
    } else {
        if (st.get_initial_cap() != stroke_style::default_cap) {
            out << class_member_operator()
                << "initial_capped(stroke_cap"
                << scope_resolution_operator()
                << cap_name(st.get_initial_cap()) << ")";
        }
        if (st.get_terminal_cap() != stroke_style::default_cap) {
            out << class_member_operator()
                << "terminal_capped(stroke_cap"
                << scope_resolution_operator()
                << cap_name(st.get_terminal_cap()) << ")";
        }
        if (st.get_dash_initial_cap() != stroke_style::default_cap) {
            out << class_member_operator()
                << "dash_initial_capped(stroke_cap"
                << scope_resolution_operator()
                << cap_name(st.get_dash_initial_cap()) << ")";
        }
        if (st.get_dash_terminal_cap() != stroke_style::default_cap) {
            out << class_member_operator()
                << "dash_terminal_capped(stroke_cap"
                << scope_resolution_operator()
                << cap_name(st.get_dash_terminal_cap()) << ")";
        }
    }
    if (!st.get_dashes().empty()) {
        out << class_member_operator()
            << "dashed({";
        for (float d: st.get_dashes()) out << d << ",";
        out << "})";
    }
    if (st.get_dash_offset() != stroke_style::default_dash_offset) {
        out << class_member_operator()
            << "dash_offset(" << st.get_dash_offset() << ")";
    }
    if (st.get_resets_on_move() != stroke_style::default_resets_on_move) {
        out << class_member_operator()
            << "reset_on_move("
            << st.get_resets_on_move()
            << ")";
    }
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_shape(const shape &s, std::ostream &out) {
    switch (s.get_type()) {
        case shape::e_type::path:
            print_path(s.get_path_data(), out);
            break;
        case shape::e_type::circle:
            print_circle(s.get_circle_data(), out);
            break;
        case shape::e_type::triangle:
            print_triangle(s.get_triangle_data(), out);
            break;
        case shape::e_type::rect:
            print_rect(s.get_rect_data(), out);
            break;
        case shape::e_type::polygon:
            print_polygon(s.get_polygon_data(), out);
            break;
        case shape::e_type::stroke: {
            const auto &stk = s.get_stroke_data();
            print_shape(stk.get_shape(), out);
            print_xform(stk.get_shape().get_xf(), out);
            print_style(stk.get_width(), stk.get_style(), out);
            break;
        }
        default:
            break;
    }
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_rgba8(const RGBA8 &c, std::ostream &out) {
    const auto found = named_colors::RGBA8_to_string.find(c);
    if (found == named_colors::RGBA8_to_string.end()) {
        if (c[3] != unorm8{255}) {
            out << "rgba8("
                << static_cast<int>(c[0]) << ','
                << static_cast<int>(c[1]) << ','
                << static_cast<int>(c[2]) << ','
                << static_cast<int>(c[3]) << ')';
        } else {
            out << "rgb8("
                << static_cast<int>(c[0]) << ','
                << static_cast<int>(c[1]) << ','
                << static_cast<int>(c[2]) << ')';
        }
    } else {
        out << "color"
            << scope_resolution_operator()
            << found->second;
    }
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_solid_color(const RGBA8 &c, unorm8 opacity, std::ostream &out) {
    if (opacity != unorm8{255})
        out << "solid_color(";
    print_rgba8(c, out);
    if (opacity != unorm8{255})
        out << ',' << opacity_scale(opacity) << ')';
}

template <typename FLAVOR>
const char *
scene_f_print_rvg<FLAVOR>::
spread_name(e_spread s) {
    switch (s) {
        case e_spread::clamp: return "clamp";
        case e_spread::wrap: return "wrap";
        case e_spread::mirror: return "mirror";
        case e_spread::transparent: return "transparent";
        default: return nullptr;
    }
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_ramp(const color_ramp &r, std::ostream &out) {
    out << "color_ramp(spread"
        << scope_resolution_operator()
        << spread_name(r.get_spread()) << ",{";
    for (const auto &stop: r.get_color_stops()) {
        out << '{' << stop.get_offset() << ',';
        print_rgba8(stop.get_color(), out);
        out << "},";
    }
    out << "})";
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_linear_gradient(const linear_gradient_data &lingrad,
    unorm8 opacity, std::ostream &out) {
    out << "linear_gradient(";
    print_ramp(lingrad.get_color_ramp(), out);
    out << ',' << lingrad.get_x1() << ',' << lingrad.get_y1() << ','
        << lingrad.get_x2() << ',' << lingrad.get_y2();
    if (opacity < unorm8{255})
        out << ',' << opacity_scale(opacity);
    out << ')';
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_radial_gradient(const radial_gradient_data &radgrad,
    unorm8 opacity, std::ostream &out) {
    out << "radial_gradient(";
    print_ramp(radgrad.get_color_ramp(), out);
    out << ',' << radgrad.get_cx() << ',' << radgrad.get_cy() << ','
        << radgrad.get_fx() << ',' << radgrad.get_fy() << ','
        << radgrad.get_r();
    if (opacity < unorm8{255})
        out << ',' << opacity_scale(opacity);
    out << ')';
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_texture(const texture_data &tex, unorm8 opacity,
    std::ostream &out) {
    out << "texture(";
    if (tex.get_spread() != e_spread::clamp)
        out << "spread"
            << scope_resolution_operator()
            << spread_name(tex.get_spread())
            << ",";
    out << "load_png(decode_base64("
        << begin_raw_string_literal()
        << '\n';
    std::string s;
    if (tex.get_image().get_channel_type() == e_channel_type::uint8_t_) {
        store_png<uint8_t>(&s, tex.get_image_ptr());
    } else {
        store_png<uint16_t>(&s, tex.get_image_ptr());
    }
    out << encode_base64(s)
        << end_raw_string_literal()
        << "))";
    if (opacity != unorm8{255})
        out << ',' << opacity_scale(opacity);
    out << ')';
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
print_paint(const paint &p, std::ostream &out) {
    switch (p.get_type()) {
        case paint::e_type::solid_color:
            print_solid_color(p.get_solid_color(), p.get_opacity(), out);
            break;
        case paint::e_type::linear_gradient:
            print_linear_gradient(p.get_linear_gradient_data(),
                p.get_opacity(), out);
            break;
        case paint::e_type::radial_gradient:
            print_radial_gradient(p.get_radial_gradient_data(),
                p.get_opacity(), out);
            break;
        case paint::e_type::texture:
            print_texture(p.get_texture_data(), p.get_opacity(), out);
        default:
            break;
    }
}

template <typename FLAVOR>
const char *
scene_f_print_rvg<FLAVOR>::
winding_rule_prefix(e_winding_rule wr) {
    switch (wr) {
        case e_winding_rule::non_zero: return "";
        case e_winding_rule::zero: return "z";
        case e_winding_rule::odd: return "eo";
        case e_winding_rule::even: return "e";
        default: return "";
    }
}

template <typename FLAVOR>
template <typename P, typename C>
void
scene_f_print_rvg<FLAVOR>::
print_patch(const P &points, const C &colors, std::ostream &out) {
    using namespace boost::adaptors;
    out << '{';
    m_flavor.print_point(points[0], out);
    for (const auto &p: points | sliced(1, points.size())) {
        out << ", ";
        m_flavor.print_point(p, out);
    }
    out << "}, {";
    print_rgba8(colors[0], out);
    for (const auto &c: colors | sliced(1, colors.size())) {
        out << ", ";
        print_rgba8(c, out);
    }
    out << '}';
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_tensor_product_patch(const patch<16,4> &patch) {
    m_out << m_nl << "tensor_product_patch(";
    print_patch(patch.get_patch_data().get_points(),
        patch.get_patch_data().get_colors(), m_out);
    m_out << "),";
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_coons_patch(const patch<12,4> &patch) {
    m_out << m_nl << "coons_patch(";
    print_patch(patch.get_patch_data().get_points(),
        patch.get_patch_data().get_colors(), m_out);
    m_out << "),";
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_gouraud_triangle(const patch<3,3> &tri) {
    m_out << m_nl << "gouraud_triangle(";
    print_patch(tri.get_patch_data().get_points(),
        tri.get_patch_data().get_colors(), m_out);
    m_out << "),";
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_painted_shape(e_winding_rule wr, const shape &s, const paint &p) {
    m_out << m_nl << winding_rule_prefix(wr) << "fill(";
    m_nl++;
    if (s.get_xf() != p.get_xf()) {
        print_shape(s, m_out);
        print_xform(s.get_xf(), m_out);
        m_out << ", " << m_nl;
        print_paint(p, m_out);
        print_xform(p.get_xf(), m_out);
        --m_nl;
        m_out << "),";
    } else {
        print_shape(s, m_out);
        m_out << ", " << m_nl;
        print_paint(p, m_out);
        --m_nl;
        m_out << ')';
        print_xform(p.get_xf(), m_out);
        m_out << ',';
    }
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_stencil_shape(e_winding_rule wr, const shape &s) {
    m_out << m_nl << winding_rule_prefix(wr) << "punch(";
    print_shape(s, m_out);
    print_xform(s.get_xf(), m_out);
    m_out << "),";
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_begin_clip(uint16_t depth) {
    (void) depth;
    m_out << m_nl << "clip({";
    m_nl++;
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_activate_clip(uint16_t depth) {
    (void) depth;
    m_nl--;
    m_out << m_nl << "},{";
    m_nl++;
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_end_clip(uint16_t depth) {
    (void) depth;
    m_nl--;
    m_out << m_nl << "}),";
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_begin_fade(uint16_t depth, unorm8 opacity) {
    (void) depth;
    m_out << m_nl << "fade(" << opacity_scale(opacity) << ", {\n";
    m_nl++;
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_end_fade(uint16_t depth, unorm8 opacity) {
    (void) depth; (void) opacity;
    m_nl--;
    m_out << m_nl << "}),";
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_begin_blur(uint16_t depth, float radius) {
    (void) depth;
    m_out << m_nl << "blur(" << radius << ", {";
    m_nl++;
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_end_blur(uint16_t depth, float radius) {
    (void) depth; (void) radius;
    m_nl--;
    m_out << m_nl << "}),";
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_begin_transform(uint16_t depth, const xform &xf) {
    (void) depth;
    m_out << m_nl << "transform(identity()";
    print_xform(xf, m_out);
    m_out << ", {";
    m_nl++;
}

template <typename FLAVOR>
void
scene_f_print_rvg<FLAVOR>::
do_end_transform(uint16_t depth, const xform &xf) {
    (void) depth; (void) xf;
    m_nl--;
    m_out << m_nl << "}),";
}

template <typename FLAVOR>
FLAVOR scene_f_print_rvg<FLAVOR>::m_flavor = FLAVOR{};

scene_f_print_rvg<rvg::detail::cpp_flavor>
make_scene_f_print_rvg_cpp(std::ostream &out) {
    return scene_f_print_rvg<detail::cpp_flavor>{out};
}

scene_f_print_rvg<rvg::detail::lua_flavor>
make_scene_f_print_rvg_lua(std::ostream &out) {
    return scene_f_print_rvg<detail::lua_flavor>{out};
}

template class scene_f_print_rvg<rvg::detail::cpp_flavor>;


template class scene_f_print_rvg<rvg::detail::lua_flavor>;

} // namespace rvg
