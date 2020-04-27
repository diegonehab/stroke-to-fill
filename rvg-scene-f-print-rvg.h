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
#ifndef RVG_SCENE_F_PRINT_RVG
#define RVG_SCENE_F_PRINT_RVG

#include "rvg-indent.h"
#include "rvg-i-scene-data.h"
#include "rvg-shape.h"
#include "rvg-paint.h"
#include "rvg-unorm.h"

namespace rvg {

namespace detail {

class lua_flavor {
public:
    static const char *class_member_operator(void) {
        return ":";
    }
    static const char *scope_resolution_operator(void) {
        return ".";
    }
    static const char *begin_raw_string_literal(void) {
        return "[=[";
    }
    static const char *end_raw_string_literal(void) {
        return "]=]";
    }
    static float opacity_scale(unorm8 v) {
        return unorm<float>{v};
    }
    static void print_point(const R2 &p, std::ostream &out) {
        out << p[0] << ',' << p[1];
    }
};

class cpp_flavor {
public:
    static const char *class_member_operator(void) {
        return ".";
    }
    static const char *scope_resolution_operator(void) {
        return "::";
    }
    static const char *begin_raw_string_literal(void) {
        return "R\"-(";
    }
    static const char *end_raw_string_literal(void) {
        return ")-\"";
    }
    static float opacity_scale(unorm8 v) {
        return static_cast<float>(v);
    }
    static void print_point(const R2 &p, std::ostream &out) {
        out << "R2{" << p[0] << ',' << p[1] << "}";
    }
};

}

template <typename FLAVOR>
class scene_f_print_rvg final:

    public i_scene_data<scene_f_print_rvg<FLAVOR>> {

    std::ostream &m_out;
    indent m_nl;

    static FLAVOR m_flavor;

public:

    static const char *class_member_operator(void);

    static const char *scope_resolution_operator(void);

    static const char *begin_raw_string_literal(void);

    static const char *end_raw_string_literal(void);

    static float opacity_scale(unorm8 o);

    scene_f_print_rvg(std::ostream &out);

    static void print_rotation(float c, float s, std::ostream &out);

    static void print_scaling(float sx, float sy, std::ostream &out);

    static void print_translation(float tx, float ty, std::ostream &out);

    static void print_linear(float a, float b, float c, float d,
        std::ostream &out);

    static void print_affinity(float a, float b, float tx,
       float c, float d, float ty, std::ostream &out);

    static void print_projectivity(float a, float b, float c,
       float d, float e, float f,
       float g, float h, float i, std::ostream &out);

    static void print_xform(const xform &xf, std::ostream &out);

    static void print_path(const path_data &p, std::ostream &out);

    static void print_circle(const circle_data &c, std::ostream &out);

    static void print_triangle(const triangle_data &t, std::ostream &out);

    static void print_rect(const rect_data &r, std::ostream &out);

    static void print_polygon(const polygon_data &p, std::ostream &out);

    static const char *cap_name(e_stroke_cap cap);

    static const char *join_name(e_stroke_join join);

    static void print_style(float width, const stroke_style &st,
        std::ostream &out);

    static void print_shape(const shape &s, std::ostream &out);

    static void print_rgba8(const RGBA8 &c, std::ostream &out);

    template <typename P, typename C>
    static void print_patch(const P &points, const C &colors,
        std::ostream &out);

    static void print_solid_color(const RGBA8 &c, unorm8 opacity,
        std::ostream &out);

    static const char *spread_name(e_spread s);

    static void print_ramp(const color_ramp &r, std::ostream &out);

    static void print_linear_gradient(const linear_gradient_data &lingrad,
        unorm8 opacity, std::ostream &out);

    static void print_radial_gradient(const radial_gradient_data &radgrad,
        unorm8 opacity, std::ostream &out);

    static void print_texture(const texture_data &tex, unorm8 opacity,
        std::ostream &out);

    static void print_paint(const paint &p, std::ostream &out);

    static const char *winding_rule_prefix(e_winding_rule wr);

private:

    friend i_scene_data<scene_f_print_rvg<FLAVOR>>;

    void do_painted_shape(e_winding_rule wr, const shape &s, const paint &p);

    void do_stencil_shape(e_winding_rule wr, const shape &s);

    void do_tensor_product_patch(const patch<16,4> &tpp);

    void do_coons_patch(const patch<12,4> &cp);

    void do_gouraud_triangle(const patch<3,3> &gt);

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

extern template class scene_f_print_rvg<rvg::detail::cpp_flavor>;

scene_f_print_rvg<rvg::detail::cpp_flavor>
make_scene_f_print_rvg_cpp(std::ostream &out);

extern template class scene_f_print_rvg<rvg::detail::lua_flavor>;

scene_f_print_rvg<rvg::detail::lua_flavor>
make_scene_f_print_rvg_lua(std::ostream &out);

} // namespace rvg

#endif
