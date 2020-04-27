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
#include <type_traits>
#include <iterator>
#include <cstdint>

#include "rvg-ptr.h"
#include "rvg-meta.h"
#include "rvg-svg-path-parse.h"

#include "rvg-facade.h"

namespace rvg_facade { namespace driver {

rvg::RGBA8 rgba(float r, float g, float b, float a) {
    return rvg::make_rgba(r, g, b, a);
}

rvg::RGBA8 rgba8(int r, int g, int b, int a) {
    return rvg::make_rgba8(r, g, b, a);
}

rvg::RGBA8 rgb(float r, float g, float b) {
    return rvg::make_rgb(r, g, b);
}

rvg::RGBA8 rgb8(int r, int g, int b) {
    return rvg::make_rgb8(r, g, b);
}

rvg::shape path(const char *svg) {
    auto p = rvg::make_intrusive<rvg::path_data>();
    if (!svg_path_iterate(svg, *p)) {
        p->clear();
        p->shrink_to_fit();
    }
    return rvg::shape(p);
}

rvg::shape circle(rvgf x, rvgf y, rvgf r) {
    return rvg::shape(rvg::make_intrusive<rvg::circle_data>(x, y, r));
}

rvg::shape triangle(rvgf x1, rvgf y1, rvgf x2, rvgf y2,
    rvgf x3, rvgf y3) {
    return rvg::shape(rvg::make_intrusive<rvg::triangle_data>(x1, y1, x2, y2, x3, y3));
}

rvg::shape rect(rvgf x, rvgf y, rvgf width, rvgf height) {
    return rvg::shape(rvg::make_intrusive<rvg::rect_data>(x, y, width, height));
}

rvg::shape polygon(const std::initializer_list<rvgf> &coordinates) {
    return rvg::shape(rvg::make_intrusive<rvg::polygon_data>(coordinates));
}

rvg::shape path(const std::initializer_list<rvg::svg_path_token> &svg) {
    auto p = rvg::make_intrusive<rvg::path_data>();
    if (!rvg::svg_path_iterate(svg, *p)) {
        p->clear();
        p->shrink_to_fit();
    }
    return rvg::shape(p);
}

rvg::patch<16,4> tensor_product_patch(const R2 (&points)[16],
    const rvg::RGBA8 (&colors)[4], uint8_t opacity) {
    return rvg::patch<16,4>{rvg::make_intrusive<rvg::patch_data<16,4>>(
        points, colors), rvg::unorm8{opacity}};
}

rvg::patch<12,4> coons_patch(const R2 (&points)[12],
    const rvg::RGBA8 (&colors)[4], uint8_t opacity) {
    return rvg::patch<12,4>{rvg::make_intrusive<rvg::patch_data<12,4>>(
        points, colors), rvg::unorm8{opacity}};
}

rvg::patch<3,3> gouraud_triangle(const R2 (&points)[3],
    const rvg::RGBA8 (&colors)[3], uint8_t opacity) {
    return rvg::patch<3,3>{rvg::make_intrusive<rvg::patch_data<3,3>>(
        points, colors), rvg::unorm8{opacity}};
}

painted_compound nzfill(const rvg::shape &s, const rvg::paint &p) {
    return make_painted_compound_painted_shape(rvg::e_winding_rule::non_zero, s, p);
}

painted_compound zfill(const rvg::shape &s, const rvg::paint &p) {
    return make_painted_compound_painted_shape(rvg::e_winding_rule::zero, s, p);
}

painted_compound ofill(const rvg::shape &s, const rvg::paint &p) {
    return make_painted_compound_painted_shape(rvg::e_winding_rule::odd, s, p);
}

painted_compound efill(const rvg::shape &s, const rvg::paint &p) {
    return make_painted_compound_painted_shape(rvg::e_winding_rule::even, s, p);
}

painted_compound fill(const rvg::shape &s, const rvg::paint &p) {
    return make_painted_compound_painted_shape(rvg::e_winding_rule::non_zero, s, p);
}

painted_compound eofill(const rvg::shape &s, const rvg::paint &p) {
    return make_painted_compound_painted_shape(rvg::e_winding_rule::odd, s, p);
}

painted_compound nzfill(const rvg::shape &s, const rvg::RGBA8 &c) {
    return nzfill(s, solid_color(c));

}

painted_compound  zfill(const rvg::shape &s, const rvg::RGBA8 &c) {
    return zfill(s, solid_color(c));
}

painted_compound  ofill(const rvg::shape &s, const rvg::RGBA8 &c) {
    return zfill(s, solid_color(c));
}

painted_compound  efill(const rvg::shape &s, const rvg::RGBA8 &c) {
    return efill(s, solid_color(c));
}

painted_compound   fill(const rvg::shape &s, const rvg::RGBA8 &c) {
    return fill(s, solid_color(c));
}

painted_compound eofill(const rvg::shape &s, const rvg::RGBA8 &c) {
    return eofill(s, solid_color(c));
}

painted_compound fade(uint8_t opacity, const vec_painted &painted_compound) {
    return make_painted_compound_faded(rvg::unorm8{opacity}, painted_compound);
}

painted_compound fade(uint8_t opacity, const il_painted &painted_compound) {
    return make_painted_compound_faded(rvg::unorm8{opacity}, painted_compound);
}

painted_compound transform(const rvg::xform &xf, const vec_painted &painted_compound) {
    return make_painted_compound_xformed(xf, painted_compound);
}

painted_compound transform(const rvg::xform &xf, const il_painted &painted_compound) {
    return make_painted_compound_xformed(xf, painted_compound);
}

painted_compound blur(float radius, const vec_painted &painted_compound) {
    return make_painted_compound_blurred(radius, painted_compound);
}

painted_compound blur(float radius, const il_painted &painted_compound) {
    return make_painted_compound_blurred(radius, painted_compound);
}

painted_compound blur(float radius, const painted_compound &painted_compound) {
    return blur(radius, { painted_compound });
}

stencil_compound nzpunch(const rvg::shape &s) {
    return make_stencil_compound_stencil_shape(rvg::e_winding_rule::non_zero, s);
}

stencil_compound zpunch(const rvg::shape &s) {
    return make_stencil_compound_stencil_shape(rvg::e_winding_rule::zero, s);
}

stencil_compound opunch(const rvg::shape &s) {
    return make_stencil_compound_stencil_shape(rvg::e_winding_rule::odd, s);
}

stencil_compound epunch(const rvg::shape &s) {
    return make_stencil_compound_stencil_shape(rvg::e_winding_rule::even, s);
}

stencil_compound punch(const rvg::shape &s) {
    return make_stencil_compound_stencil_shape(rvg::e_winding_rule::non_zero, s);
}

stencil_compound eopunch(const rvg::shape &s) {
    return make_stencil_compound_stencil_shape(rvg::e_winding_rule::odd, s);
}

painted_compound clip(const vec_stencil &clipper, const vec_painted &clippee) {
    return make_painted_compound_clipped(clipper, clippee);
}

painted_compound clip(const il_stencil &clipper, const il_painted &clippee) {
    return make_painted_compound_clipped(clipper, clippee);
}

painted_compound clip(const stencil_compound &clipper, const il_painted &clippee) {
    return clip({ clipper }, clippee);
}

painted_compound clip(const il_stencil &clipper, const painted_compound &clippee) {
    return clip(clipper, { clippee });
}

painted_compound clip(const stencil_compound &clipper, const painted_compound &clippee) {
    return clip({ clipper }, { clippee });
}

stencil_compound clip(const il_stencil &clipper, const il_stencil &clippee) {
    return make_stencil_compound_clipped(clipper, clippee);
}

stencil_compound clip(const vec_stencil &clipper, const vec_stencil &clippee) {
    return make_stencil_compound_clipped(clipper, clippee);
}

stencil_compound clip(const stencil_compound &clipper, const il_stencil &clippee) {
    return clip({ clipper }, clippee);
}

stencil_compound clip(const il_stencil &clipper, const stencil_compound &clippee) {
    return clip(clipper, { clippee });
}

stencil_compound clip(const stencil_compound &clipper, const stencil_compound &clippee) {
    return clip({ clipper }, { clippee });
}

stencil_compound transform(const rvg::xform &xf, const il_stencil &stencil) {
    return make_stencil_compound_xformed(xf, stencil);
}

stencil_compound transform(const rvg::xform &xf, const vec_stencil &stencil) {
    return make_stencil_compound_xformed(xf, stencil);
}

rvg::scene scene(rvg::scene_data::const_ptr s) {
    return rvg::scene{s};
}

rvg::scene scene(const il_painted &painted_compound) {
    return rvg::scene{rvg::make_intrusive<rvg::scene_data>(
        make_scene_data(painted_compound))};
}

rvg::scene scene(const std::vector<painted_compound> &painted_compound) {
    return rvg::scene{rvg::make_intrusive<rvg::scene_data>(
        make_scene_data(painted_compound))};
}

rvg::scene scene(const painted_compound &painted_compound) {
    return rvg::scene{rvg::make_intrusive<rvg::scene_data>(
        make_scene_data(painted_compound))};
}

rvg::paint solid_color(const rvg::RGBA8 &color, uint8_t opacity) {
    return rvg::paint{color, rvg::unorm8{opacity}};
}

rvg::color_ramp::const_ptr color_ramp(rvg::e_spread spread,
    const std::initializer_list<rvg::color_stop> &stops) {
    return rvg::make_intrusive<rvg::color_ramp>(spread, stops);
}

rvg::color_ramp::const_ptr color_ramp(const std::initializer_list<rvg::color_stop> &stops) {
    return rvg::make_intrusive<rvg::color_ramp>(rvg::e_spread::clamp, stops);
}

rvg::paint texture(rvg::e_spread spread, rvg::i_image::const_ptr image_ptr,
    uint8_t opacity) {
    return rvg::paint(rvg::make_intrusive<rvg::texture_data>(spread, image_ptr),
        rvg::unorm8{opacity});
}

rvg::paint texture(rvg::i_image::const_ptr image_ptr, uint8_t opacity) {
    return rvg::paint(rvg::make_intrusive<rvg::texture_data>(rvg::e_spread::clamp, image_ptr),
        rvg::unorm8{opacity});
}

rvg::paint linear_gradient(rvg::color_ramp::const_ptr ramp_ptr,
    float x1, float y1, float x2, float y2, uint8_t opacity) {
    return rvg::paint(rvg::make_intrusive<rvg::linear_gradient_data>(ramp_ptr,
        x1, y1, x2, y2), rvg::unorm8{opacity});
}

rvg::paint radial_gradient(rvg::color_ramp::const_ptr ramp_ptr,
    float cx, float cy, float fx, float fy, float r, uint8_t opacity) {
    return rvg::paint(rvg::make_intrusive<rvg::radial_gradient_data>(ramp_ptr,
        cx, cy, fx, fy, r), rvg::unorm8{opacity});
}

rvg::window window(rvgf xl, rvgf yb, rvgf xr, rvgf yt) {
    return rvg::make_window(xl, yb, xr, yt);
}

rvg::viewport viewport(rvgi xl, rvgi yb, rvgi xr, rvgi yt) {
    return rvg::make_viewport(xl, yb, xr, yt);
}

} } // namespace rvg_facade::driver
