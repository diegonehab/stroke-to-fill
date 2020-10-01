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
#include <unordered_map>
#include <string>
#include <sstream>

#include "rvg-lua.h"

#include "rvg-lua-facade.h"

#include "rvg-indent.h"
#include "rvg-i-image.h"
#include "rvg-base64.h"
#include "rvg-pngio.h"
#include "rvg-rgba.h"
#include "rvg-named-colors.h"
#include "rvg-xform.h"
#include "rvg-paint.h"
#include "rvg-path-data.h"

#include "rvg-svg-path-f-command-printer.h"
#include "rvg-input-path-f-to-svg-path.h"
#include "rvg-input-path-f-xform.h"

#include "rvg-driver-svg.h"

namespace rvg {
    namespace driver {
        namespace svg {

const scene &accelerate(const scene &c, const window &w,
    const viewport &v) {
    (void) w;
    (void) v;
    return c;
}

static const char *winding_rule_name(e_winding_rule w) {
    switch (w) {
        case e_winding_rule::non_zero: return "nonzero";
        case e_winding_rule::zero: return "zero";
        case e_winding_rule::odd: return "evenodd";
        case e_winding_rule::even: return "even";
        default: return "";
    }
}

static void print_rotation(float c, float s, const char *title, std::ostream &out) {
    if (!util::is_almost_one(c) || !util::is_almost_zero(s)) {
        out << title << "=\"rotate(" << util::deg(std::atan2(s, c)) << ")\"";
    }
}

static void print_scaling(float sx, float sy, const char *title, std::ostream &out) {
    if (util::is_almost_equal(sx, sy)) {
        float avg = 0.5f*(sx+sy);
        if (!util::is_almost_one(avg)) {
            out << title << "=\"scale(" << avg << ")\"";
        }
    } else {
        out << title << "=\"scale(" << sx << ',' << sy << ")\"";
    }
}

static void print_translation(float tx, float ty, const char *title,
    std::ostream &out) {
    if (!util::is_almost_zero(tx) || !util::is_almost_zero(ty)) {
        out << title << "=\"translate(" << tx << ',' << ty << ")\"";
    }
}

static void print_linear(float a, float b, float c, float d, const char *title,
    std::ostream &out) {
    if (util::is_almost_zero(a*b+c*d) && util::is_almost_one(a*d-b*c) &&
        util::is_almost_one(a*a+c*c) && util::is_almost_one(b*b+d*d)) {
        return print_rotation(a, c, title, out);
    } else if (util::is_almost_zero(b) && util::is_almost_zero(c)) {
        return print_scaling(a, d, title, out);
    } else {
        out << title << "=\"matrix(" <<
            a << ',' << c << ',' <<
            b << ',' << d << ",0,0)\"";
    }
}

static void print_affinity(float a, float b, float tx,
   float c, float d, float ty, const char *title, std::ostream &out) {
    if (util::is_almost_zero(tx) && util::is_almost_zero(ty)) {
        return print_linear(a, b, c, d, title, out);
    } else {
        if (util::is_almost_one(a) && util::is_almost_zero(b) &&
        util::is_almost_zero(c) && util::is_almost_one(d)) {
            return print_translation(tx, ty, title, out);
        } else {
            out << title << "=\"matrix(" <<
                a << ',' << c << ',' <<
                b << ',' << d << ',' <<
                tx << ',' << ty << ")\"";
        }
    }
}

static void print_xform(const xform &xf, const char *title,
    std::ostream &out) {
    print_affinity(xf[0][0], xf[0][1], xf[0][2], xf[1][0], xf[1][1],
        xf[1][2], title, out);
}

static void print_path_data(const path_data &path, const xform &pre_xf,
    std::ostream &out) {
    out << " d=\"";
    if (!pre_xf.is_identity()) {
        path.iterate(
            make_input_path_f_xform(pre_xf,
                make_input_path_f_to_svg_path(
                    make_svg_path_f_command_printer(out, ' '))));
    } else {
        path.iterate(
            make_input_path_f_to_svg_path(
                make_svg_path_f_command_printer(out, ' ')));
    }
    out << "\"";
}

static void print_path_data(const path_data &path, std::ostream &out) {
    out << " d=\"";
    path.iterate(
        make_input_path_f_to_svg_path(
            make_svg_path_f_command_printer(out, ' ')));
    out << "\"";
}


class scene_f_print_painted final:
    public i_scene_data<scene_f_print_painted> {
    indent &m_nl;
    const std::unordered_map<std::string, int> &m_map;
    const xform &m_screen_xf;
	std::ostream &m_out;
	int m_shape_id, m_clippath_id;
    std::vector<int> m_active_clips, m_not_yet_active_clips;
public:
	scene_f_print_painted(
        indent &nl,
        const std::unordered_map<std::string, int> &map,
        const xform &screen_xf,
        std::ostream &out):
        m_nl(nl),
        m_map(map),
        m_screen_xf(screen_xf),
        m_out(out),
        m_shape_id(0),
        m_clippath_id(0)
        { ; }

private:
    using base = i_scene_data<scene_f_print_painted>;

    void print_paint(const char *mode, const shape &shape, const paint &paint) {
        (void) shape;
        m_out << ' ' << mode << "=\"";
        switch (paint.get_type()) {
            case paint::e_type::solid_color: {
				const auto c = paint.get_solid_color();
                const auto found = named_colors::RGBA8_to_string.find(c);
				if (found == named_colors::RGBA8_to_string.end()) {
                    m_out << "rgb("
                        << static_cast<int>(c[0]) << ','
                        << static_cast<int>(c[1]) << ','
                        << static_cast<int>(c[2]) << ')';
				} else {
					m_out << found->second;
				}
                if (c[3] != unorm8{255} ||
                    paint.get_opacity() != unorm8{255}) {
                    m_out << "\" " << mode << "-opacity=\"" <<
                        unorm<float>{c[3] * paint.get_opacity()};
                }
                m_out << '"';
                break;
            }
            case paint::e_type::linear_gradient:
            case paint::e_type::radial_gradient: {
                std::ostringstream s;
                s << &paint;
                const auto found = m_map.find(s.str());
                if (found != m_map.end()) {
                    m_out << "url(#gradient" << found->second << ")";
                    if (paint.get_opacity() != unorm8{255}) {
                        m_out << "\" " << mode << "-opacity=\"" <<
                            unorm<float>{paint.get_opacity()};
                    }
                }
                m_out << '"';
                break;
            }
            case paint::e_type::texture: {
                std::ostringstream s;
                s << &paint;
                const auto found = m_map.find(s.str());
                if (found != m_map.end()) {
                    m_out << "url(#texture" << found->second << ")";
                    if (paint.get_opacity() != unorm8{255}) {
                        m_out << "\" " << mode << "-opacity=\"" <<
                            unorm<float>{paint.get_opacity()};
                    }
                }
                m_out << '"';
                break;
            }
            default:
                break;
        }
    }

    const char *join_name(e_stroke_join join) {
        switch (join) {
            case e_stroke_join::miter_clip:
                return "miter-clip";
            case e_stroke_join::round:
                return "round";
            case e_stroke_join::bevel:
                return "bevel";
            case e_stroke_join::miter_or_bevel:
                return "miter";
            case e_stroke_join::arcs:
                return "arcs";
            default:
                return "bevel";
        }
    }

    const char *cap_name(e_stroke_cap cap) {
        switch (cap) {
            case e_stroke_cap::butt:
                return "butt";
            case e_stroke_cap::round:
                return "round";
            case e_stroke_cap::square:
                return "square";
            default:
                return "butt";
        }
    }

    void print_stroke_style(float width, const stroke_style &st) {
        m_out << " stroke-width=\"" << width << '"';
        m_out << " stroke-linejoin=\"" << join_name(st.get_join()) << '"';
        m_out << " stroke-miterlimit=\"" << st.get_miter_limit() << '"';
        if (st.get_initial_cap() == st.get_terminal_cap() &&
            st.get_initial_cap() == st.get_dash_initial_cap() &&
            st.get_initial_cap() == st.get_dash_terminal_cap()) {
            m_out << " stroke-linecap=\"" <<
                cap_name(st.get_initial_cap()) << '"';
        }
        if (!st.get_dashes().empty()) {
            m_out << " stroke-dasharray=\"";
            for (float d: st.get_dashes())
                m_out << d*width << ' ';
            m_out << '"';
        }
        m_out << " stroke-dashoffset=\"" << st.get_dash_offset()*width << '"';
    }

    friend base;

    void do_painted_shape(e_winding_rule wr, const shape &s, const paint &p) {
        const char *mode = nullptr;
        const stroke_style *st = nullptr;
        float w = 0.f;
        path_data::const_ptr data;
        xform pre_xf;
        xform post_xf = s.get_xf();
        if (s.is_stroke()) {
            mode = "stroke";
            st = &s.get_stroke_data().get_style();
            w = s.get_stroke_data().get_width();
            pre_xf = s.get_stroke_data().get_shape().get_xf();
            // convert shape to be stroked into a path
            data = s.get_stroke_data().get_shape().as_path_data_ptr(
                pre_xf.transformed(m_screen_xf));
        } else {
            mode = "fill";
            data = s.as_path_data_ptr(m_screen_xf);
        }
        m_out << m_nl << "<path id=\"shape" << m_shape_id << '"';
        m_out << " fill-rule=\"" << winding_rule_name(wr) << '"';
        if (st) {
            m_out << " fill=\"none\"";
            print_stroke_style(w, *st);
        }
        print_xform(post_xf, " transform", m_out);
        print_paint(mode, s, p);
        print_path_data(*data, pre_xf, m_out);
        m_out << "/>";
        ++m_shape_id;
    }

    void do_tensor_product_patch(const patch<16,4> &) { ; }
    void do_coons_patch(const patch<12,4> &) { ; }
    void do_gouraud_triangle(const patch<3,3> &) { ; }

    void do_stencil_shape(e_winding_rule winding_rule, const shape &shape) {
        (void) winding_rule; (void) shape;
        // all stencil elements are in inside the <defs> section
    }

    void do_begin_clip(uint16_t depth) {
        (void) depth;
        m_not_yet_active_clips.push_back(m_clippath_id);
        ++m_clippath_id;
    }

    void do_activate_clip(uint16_t depth) {
        (void) depth;
        m_active_clips.push_back(m_not_yet_active_clips.back());
        m_not_yet_active_clips.pop_back();
        if (m_not_yet_active_clips.empty()) {
            m_out << m_nl++ << "<g clip-path=\"url(#clip" <<
                m_active_clips.back() << ")\">";
        }
    }

    void do_end_clip(uint16_t depth) {
        (void) depth;
        m_active_clips.pop_back();
        if (m_not_yet_active_clips.empty()) {
            m_out << --m_nl << "</g>";
        }
    }

    void do_begin_fade(uint16_t depth, unorm8 opacity) {
        (void) depth;
        if (opacity != unorm8{255}) {
            m_out << m_nl++ << "<g opacity=\"" <<
                unorm<float>{opacity} << "\">";
        }
    }

    void do_end_fade(uint16_t depth, unorm8 opacity) {
        (void) depth;
        if (opacity != unorm8{255}) {
            m_out << --m_nl << "</g>";
        }
    }

    void do_begin_blur(uint16_t depth, float radius) {
        (void) depth;
        if (!util::is_almost_zero(radius)) {
            std::ostringstream s;
            s << "blur " << radius;
            const auto found = m_map.find(s.str());
            m_out << m_nl++ << "<g filter=\"url(#blur" <<
                ((found != m_map.end())? found->second: -1) << ")\">";
        }
    }

    void do_end_blur(uint16_t depth, float radius) {
        (void) depth;
        if (!util::is_almost_zero(radius)) {
            m_out << --m_nl << "</g>";
        }
    }

    void do_begin_transform(uint16_t depth, const xform &xf) {
        (void) depth;
        if (m_not_yet_active_clips.empty()) {
            m_out << m_nl++ << "<g";
            print_xform(xf, " transform", m_out);
            m_out << '>';
        }
    }

    void do_end_transform(uint16_t depth, const xform &xf) {
        (void) depth; (void) xf;
        if (m_not_yet_active_clips.empty()) {
            m_out << --m_nl << "</g>";
        }
    }
};

class scene_f_print_paint_and_stencil final:
    public i_scene_data<scene_f_print_paint_and_stencil> {
    indent &m_nl;
    std::unordered_map<std::string, int> &m_map;
    const xform &m_screen_xf;
	std::ostream &m_out;
	int m_blur_id, m_gradient_id, m_texture_id, m_stencil_id, m_clippath_id;
    xform m_stencil_xf;
    std::vector<xform> m_stencil_xf_stack;
    std::vector<int> m_active_clips, m_not_yet_active_clips;
public:
	scene_f_print_paint_and_stencil(
        indent &nl,
        std::unordered_map<std::string, int> &map,
        const xform &screen_xf,
        std::ostream &out):
        m_nl(nl),
        m_map(map),
        m_screen_xf(screen_xf),
        m_out(out),
        m_blur_id(0),
        m_gradient_id(0),
        m_texture_id(0),
        m_stencil_id(0),
        m_clippath_id(0)
        { ; }

private:
    using base = i_scene_data<scene_f_print_paint_and_stencil>;

    const char *spread_name(e_spread s) {
        switch (s) {
            case e_spread::clamp: return "pad";
            case e_spread::wrap: return "repeat";
            case e_spread::mirror: return "reflect";
            case e_spread::transparent: return "transparent";
            default: return nullptr;
        }
    }

    void print_ramp(const color_ramp &ramp) {
        for (const auto &stop: ramp.get_color_stops()) {
            m_out << m_nl << "<stop offset=\"" << stop.get_offset() <<
                "\" stop-color=\"";
            const auto &c = stop.get_color();
            const auto found = named_colors::RGBA8_to_string.find(c);
            if (found == named_colors::RGBA8_to_string.end()) {
                m_out << "rgb("
                    << static_cast<int>(c[0]) << ','
                    << static_cast<int>(c[1]) << ','
                    << static_cast<int>(c[2]) << ')';
            } else {
                m_out << found->second;
            }
            if (c[3] < unorm8{255}) {
                m_out << "\" stop-opacity=\"" <<
                    unorm<float>{c[3]};
            }
            m_out << "\"/>";
        }
    }

    void print_linear_gradient(const shape &shape, const paint &paint) {
        (void) shape;
        std::ostringstream s;
        s << &paint;
        auto found = m_map.insert({s.str(), m_gradient_id});
        if (found.second) {
            const auto &linear_gradient = paint.get_linear_gradient_data();
            m_out << m_nl << "<linearGradient id=\"gradient" << m_gradient_id <<
                "\" gradientUnits=\"userSpaceOnUse\" x1=\"" <<
                linear_gradient.get_x1() << "\" y1=\"" <<
                linear_gradient.get_y1() << "\" x2=\"" <<
                linear_gradient.get_x2() << "\" y2=\"" <<
                linear_gradient.get_y2() << "\" spreadMethod=\"" <<
                spread_name(linear_gradient.get_color_ramp().
                    get_spread()) << "\"";
            print_xform(paint.get_xf().transformed(shape.get_xf().inverse()),
                " gradientTransform", m_out);
            m_out << ">";
            m_nl++;
            print_ramp(linear_gradient.get_color_ramp());
            m_nl--;
            m_out << m_nl << "</linearGradient>";
            ++m_gradient_id;
        }
    }

    void print_radial_gradient(const shape &shape, const paint &paint) {
        (void) shape;
        std::ostringstream s;
        s << &paint;
        auto found = m_map.insert({s.str(), m_gradient_id});
        if (found.second) {
            const auto &radial_gradient = paint.get_radial_gradient_data();
            m_out << m_nl << "<radialGradient id=\"gradient" << m_gradient_id <<
                "\" gradientUnits=\"userSpaceOnUse\" cx=\"" <<
                radial_gradient.get_cx() << "\" cy=\"" <<
                radial_gradient.get_cy() << "\" fx=\"" <<
                radial_gradient.get_fx() << "\" fy=\"" <<
                radial_gradient.get_fy() << "\" r=\"" <<
                radial_gradient.get_r() << "\" spreadMethod=\"" <<
                spread_name(radial_gradient.get_color_ramp().
                    get_spread()) << "\"";
            print_xform(paint.get_xf().transformed(shape.get_xf().inverse()),
                " gradientTransform", m_out);
            m_out << ">";
            m_nl++;
            print_ramp(radial_gradient.get_color_ramp());
            m_nl--;
            m_out << m_nl << "</radialGradient>";
            ++m_gradient_id;
        }
    }

    void print_texture(const shape &shape, const paint &paint) {
        (void) shape;
        std::ostringstream s;
        s << &paint;
        auto found = m_map.insert({s.str(), m_texture_id});
        if (found.second) {
            const auto &texture = paint.get_texture_data();
            m_out << m_nl++ << "<pattern id=\"texture" << m_texture_id <<
                "\" patternUnits=\"userSpaceOnUse\" width=\"1\" height=\"1\" preserveAspectRatio=\"none\" ";
            print_xform(paint.get_xf().transformed(shape.get_xf().inverse()),
                " patternTransform", m_out);
            m_out << '>';
            m_out << m_nl << "<image id=\"image" << m_texture_id <<
                "\" width=\"1\" height=\"1\" preserveAspectRatio=\"none\"" <<
                " transform=\"scale(1,-1) translate(0,-1)\" xlink:href=\"" <<
                " data:image/png;base64,\n";
            std::string simg;
            if (texture.get_image().get_channel_type() ==
                e_channel_type::uint8_t_) {
                store_png<uint8_t>(&simg, texture.get_image_ptr());
            } else {
                store_png<uint16_t>(&simg, texture.get_image_ptr());
            }
            m_out << encode_base64(simg) << "\"/>";
            m_out << --m_nl << "</pattern>";
            ++m_texture_id;
        }
    }

    friend base;

    void do_painted_shape(e_winding_rule, const shape &s, const paint &p) {
        switch (p.get_type()) {
            case paint::e_type::solid_color:
                break;
            case paint::e_type::linear_gradient:
                print_linear_gradient(s, p);
                break;
            case paint::e_type::radial_gradient:
                print_radial_gradient(s, p);
                break;
            case paint::e_type::texture:
                print_texture(s, p);
                break;
            default:
                break;
        }
    }

    void do_tensor_product_patch(const patch<16,4> &) { ; }
    void do_coons_patch(const patch<12,4> &) { ; }
    void do_gouraud_triangle(const patch<3,3> &) { ; }

    void do_stencil_shape(e_winding_rule wr, const shape &s) {
        m_out << m_nl << "<path id=\"stencil" << m_stencil_id << '"';
        m_out << " clip-rule=\"" << winding_rule_name(wr) << '"';
        path_data::const_ptr data = s.as_path_data_ptr(m_screen_xf);
        print_path_data(*data, m_out);
        print_xform(s.get_xf().transformed(m_stencil_xf), " transform", m_out);
        m_out << "/>";
        ++m_stencil_id;
    }

    void do_begin_clip(uint16_t depth) {
        (void) depth;
        m_not_yet_active_clips.push_back(m_clippath_id);
        ++m_clippath_id;
    }

    void do_activate_clip(uint16_t depth) {
        (void) depth;
        m_active_clips.push_back(m_not_yet_active_clips.back());
        m_not_yet_active_clips.pop_back();
    }

    void do_end_clip(uint16_t depth) {
        (void) depth;
        m_active_clips.pop_back();
    }

    void do_begin_fade(uint16_t depth, unorm8 opacity) {
        (void) depth; (void) opacity;
    }

    void do_end_fade(uint16_t depth, unorm8 opacity) {
        (void) depth; (void) opacity;
    }

    void do_begin_blur(uint16_t depth, float radius) {
        (void) depth;
        if (!util::is_almost_zero(radius)) {
            std::ostringstream s;
            s << "blur " << radius;
            auto found = m_map.insert({s.str(), m_blur_id});
            if (found.second) {
                m_out << m_nl++ << "<filter id=\"blur" << m_blur_id << "\">";
                m_out << m_nl << "<feGaussianBlur stdDeviation=\"" << radius
                    << "\"/>";
                m_out << --m_nl << "</filter>";
                ++m_blur_id;
            }
        }
    }

    void do_end_blur(uint16_t depth, float radius) {
        (void) depth; (void) radius;
    }

    void do_begin_transform(uint16_t depth, const xform &xf) {
        (void) depth;
        // if the transformation happened within a stencil
        // definition, we accumulate it
        if (!m_not_yet_active_clips.empty()) {
            m_stencil_xf_stack.push_back(m_stencil_xf);
            m_stencil_xf = xf.transformed(m_stencil_xf);
        }
    }

    void do_end_transform(uint16_t depth, const xform &xf) {
        (void) depth; (void) xf;
        if (!m_not_yet_active_clips.empty()) {
            m_stencil_xf = m_stencil_xf_stack.back();
            m_stencil_xf_stack.pop_back();
        }
    }
};

class scene_f_print_clip final:
    public i_scene_data<scene_f_print_clip> {
    indent &m_nl;
    const std::unordered_map<std::string, int> &m_map;
    const xform &m_screen_xf;
	std::ostream &m_out;
	int m_stencil_id, m_clippath_id;
    std::vector<int> m_active_clips, m_not_yet_active_clips, m_nested_clips;
public:
	scene_f_print_clip(
        indent &nl,
        const std::unordered_map<std::string, int> &map,
        const xform &screen_xf,
        std::ostream &out):
        m_nl(nl),
        m_map(map),
        m_screen_xf(screen_xf),
        m_out(out),
        m_stencil_id(0),
        m_clippath_id(0)
        { ; }

private:
    friend i_scene_data<scene_f_print_clip>;

    void do_painted_shape(e_winding_rule wr, const shape &s, const paint &p) {
        (void) wr;
        (void) s;
        (void) p;
    }

    void do_tensor_product_patch(const patch<16,4> &) { ; }
    void do_coons_patch(const patch<12,4> &) { ; }
    void do_gouraud_triangle(const patch<3,3> &) { ; }

    void do_stencil_shape(e_winding_rule wr, const shape &s) {
        (void) wr; (void) s;
        m_out << m_nl << "<use xlink:href=\"#stencil" << m_stencil_id << '"';
        if (!m_nested_clips.empty()) {
            m_out << " clip-path=\"url(#clip" << m_nested_clips.back() << ")\"";
        }
        m_out << "/>";
        ++m_stencil_id;
    }

    void do_begin_clip(uint16_t depth) {
        (void) depth;
        m_out << m_nl++ << "<clipPath id=\"clip" << m_clippath_id << "\">";
        m_not_yet_active_clips.push_back(m_clippath_id);
        ++m_clippath_id;
    }

    void do_activate_clip(uint16_t depth) {
        (void) depth;
        m_out << --m_nl << "</clipPath>";
        m_active_clips.push_back(m_not_yet_active_clips.back());
        m_not_yet_active_clips.pop_back();
        if (!m_not_yet_active_clips.empty()) {
            m_nested_clips.push_back(m_active_clips.back());
        }
    }

    void do_end_clip(uint16_t depth) {
        (void) depth;
        if (!m_nested_clips.empty() &&
            m_nested_clips.back() == m_active_clips.back()) {
            m_nested_clips.pop_back();
        }
        m_active_clips.pop_back();
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
        (void) depth; (void) xf;
    }

    void do_end_transform(uint16_t depth, const xform &xf) {
        (void) depth; (void) xf;
    }
};

void render(const scene &accel, const window &w, const viewport &v,
    std::ostream &out, const std::vector<std::string> &args) {
    (void) args;
    int vxl, vyb, vxr, vyt;
    std::tie(vxl, vyb) = v.bl();
    std::tie(vxr, vyt) = v.tr();
    float wxl, wyb, wxr, wyt;
    std::tie(wxl, wyb) = w.bl();
    std::tie(wxr, wyt) = w.tr();
    std::unordered_map<std::string, int> map;
    indent nl;
    auto b = accel.get_background_color();
    out << "<?xml version=\"1.0\" standalone=\"no\"?>\n" <<
       "<svg\n" <<
       "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n" <<
       "  xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n" <<
       "  xmlns:cc=\"http://creativecommons.org/ns#\"\n" <<
       "  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n" <<
       "  xmlns:svg=\"http://www.w3.org/2000/svg\"\n" <<
       "  xmlns=\"http://www.w3.org/2000/svg\"\n" <<
       "  version=\"1.1\"\n" <<
       "  style=\"background-color: rgba(" <<
           static_cast<int>(b[0]) << "," << static_cast<int>(b[1]) << ","
               << static_cast<int>(b[2]) << "," << static_cast<float>(b[3])/255.f << ");\"\n" <<
       "  preserveAspectRatio=\"none\"\n" <<
       "  width=\"" << std::abs(vxr-vxl) <<
         "\" height=\"" << std::abs(vyt-vyb) << "\"\n" <<
           "  viewBox=\"" << std::min(wxl,wxr) << ' ' <<
             std::min(wyt,wyb) << ' ' << std::abs(wxl-wxr) << ' ' <<
             std::abs(wyt-wyb) << "\"" << ">";
    xform flip = make_identity();
    if (wxl > wxr) {
        flip = flip.translated(0.f,-static_cast<float>(wxl)).
            scaled(-1.f,1.f).translated(0.f,static_cast<float>(wxr));
    }
    if (wyt > wyb) {
        flip = flip.translated(0.f,-static_cast<float>(wyb)).
            scaled(1.f,-1.f).translated(0.f,static_cast<float>(wyt));
    }
    xform screen_xf = accel.get_xf().
       windowviewport(w, v).
            transformed(flip);
    ++nl;
    out << nl++ << "<defs>";
    // write stencil shape, gradient paints, and textures
	scene_f_print_paint_and_stencil psp(nl, map, screen_xf, out);
	accel.get_scene_data().iterate(psp);
    // write clip-paths
	scene_f_print_clip cp(nl, map, screen_xf, out);
	accel.get_scene_data().iterate(cp);
    out << --nl << "</defs>";
    out << nl++ << "<g";
    print_xform(flip, " transform", out);
    out << "> <!-- invert y -->";
    out << nl++ << "<g";
    print_xform(accel.get_xf(), " transform", out);
    out << "> <!-- scene-transformation -->";
    // write painted shapes
	scene_f_print_painted pp(nl, map, screen_xf, out);
	accel.get_scene_data().iterate(pp);
    out << --nl << "</g>";
    out << --nl << "</g>";
    out << "\n</svg>\n";
}

} } } // namespace rvg::driver::svg

// Lua version of the accelerate function.
// Since there is no acceleration, we simply
// and return the input scene unmodified.
static int luaaccelerate(lua_State *L) {
    lua_settop(L, 1);
    return 1;
}

// Lua version of render function
static int luarender(lua_State *L) {
    auto accel = rvg_lua_check<rvg::scene>(L, 1);
    auto w = rvg_lua_check<rvg::window>(L, 2);
    auto v = rvg_lua_check<rvg::viewport>(L, 3);
    FILE *f = rvg_lua_check_file(L, 4);
    std::ostringstream sout;
    rvg::driver::svg::render(accel, w, v, sout);
    fwrite(sout.str().data(), 1, sout.str().size(), f);
    return 0;
}

// List of Lua functions exported into driver table
static const luaL_Reg modsvg[] = {
    {"render", luarender },
    {"accelerate", luaaccelerate },
    {NULL, NULL}
};

// Lua function invoked to be invoked by require"driver.svg"
extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_driver_svg(lua_State *L) {
	// build driver with all Lua functions needed to build
    // the scene description and add our methods to it
	rvg_lua_facade_new_driver(L, modsvg); // driver mettab
    return 1;
}
