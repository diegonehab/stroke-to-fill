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
#ifndef RVG_FACADE_H
#define RVG_FACADE_H

#include <vector>
#include <initializer_list>

#include "rvg-floatint.h"
#include "rvg-shape.h"
#include "rvg-paint.h"
#include "rvg-xform.h"
#include "rvg-window.h"
#include "rvg-viewport.h"
#include "rvg-rgba.h"
#include "rvg-named-colors.h"
#include "rvg-stroke-style.h"
#include "rvg-i-image.h"
#include "rvg-pngio.h"
#include "rvg-base64.h"
#include "rvg-scene.h"
#include "rvg-svg-path-commands.h"
#include "rvg-facade-painted-compound.h"
#include "rvg-facade-stencil-compound.h"
#include "rvg-facade-scene-data.h"

namespace rvg_facade {

using description = std::tuple<
    rvg::scene,
    rvg::window,
    rvg::viewport
>;

namespace driver {

using il_painted = std::initializer_list<painted_compound>;
using vec_painted = std::vector<painted_compound>;
using il_stencil = std::initializer_list<stencil_compound>;
using vec_stencil = std::vector<stencil_compound>;

using rvg::rvgf;
using rvg::rvgi;
using rvg::R2;

using namespace rvg::svg_path_commands;

namespace color {
    using namespace rvg::named_colors;
}

using stroke_cap = rvg::e_stroke_cap;
using stroke_join = rvg::e_stroke_join;
using spread = rvg::e_spread;

rvg::RGBA8 rgba(float r, float g, float b, float a);
rvg::RGBA8 rgba8(int r, int g, int b, int a);
rvg::RGBA8 rgb(float r, float g, float b);
rvg::RGBA8 rgb8(int r, int g, int b);

rvg::window window(rvgf xl, rvgf yb, rvgf xr, rvgf yt);
rvg::viewport viewport(rvgi xl, rvgi yb, rvgi xr, rvgi yt);

rvg::shape path(const char *svg);
rvg::shape path(const std::initializer_list<rvg::svg_path_token> &token_list);
rvg::shape polygon(const std::initializer_list<float> &point_list);
rvg::shape circle(rvgf x, rvgf y, rvgf r);
rvg::shape triangle(rvgf x1, rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3);
rvg::shape rect(rvgf x, rvgf y, rvgf width, rvgf height);

rvg::patch<16,4> tensor_product_patch(const R2 (&points)[16],
    const rvg::RGBA8 (&colors)[4], uint8_t opacity = 255);

rvg::patch<12,4> coons_patch(const R2 (&points)[12],
    const rvg::RGBA8 (&colors)[4], uint8_t opacity = 255);

rvg::patch<3,3> gouraud_triangle(const R2 (&points)[3],
    const rvg::RGBA8 (&colors)[3], uint8_t opacity = 255);

painted_compound nzfill(const rvg::shape &s, const rvg::paint &p);
painted_compound  zfill(const rvg::shape &s, const rvg::paint &p);
painted_compound  ofill(const rvg::shape &s, const rvg::paint &p);
painted_compound  efill(const rvg::shape &s, const rvg::paint &p);
painted_compound   fill(const rvg::shape &s, const rvg::paint &p);
painted_compound eofill(const rvg::shape &s, const rvg::paint &p);

painted_compound nzfill(const rvg::shape &s, const rvg::RGBA8 &c);
painted_compound  zfill(const rvg::shape &s, const rvg::RGBA8 &c);
painted_compound  ofill(const rvg::shape &s, const rvg::RGBA8 &c);
painted_compound  efill(const rvg::shape &s, const rvg::RGBA8 &c);
painted_compound   fill(const rvg::shape &s, const rvg::RGBA8 &c);
painted_compound eofill(const rvg::shape &s, const rvg::RGBA8 &c);

painted_compound transform(const rvg::xform &xf, const il_painted &p);
painted_compound transform(const rvg::xform &xf, const vec_painted &p);

painted_compound blur(float radius, const painted_compound &p);
painted_compound blur(float radius, const vec_painted &p);

painted_compound fade(uint8_t opacity, const painted_compound &p);
painted_compound fade(uint8_t opacity, const vec_painted &p);

stencil_compound nzpunch(const rvg::shape &s);
stencil_compound  zpunch(const rvg::shape &s);
stencil_compound  opunch(const rvg::shape &s);
stencil_compound  epunch(const rvg::shape &s);
stencil_compound   punch(const rvg::shape &s);
stencil_compound eopunch(const rvg::shape &s);

painted_compound clip(const vec_stencil &clipper, const vec_painted &clipped);
painted_compound clip(const il_stencil &clipper, const il_painted &clippee);
painted_compound clip(const stencil_compound &clipper, const il_painted &clippee);
painted_compound clip(const il_stencil &clipper, const painted_compound &clippee);
painted_compound clip(const stencil_compound &clipper, const painted_compound &clippee);

stencil_compound clip(const vec_stencil &clipper, const vec_stencil &clipped);
stencil_compound clip(const il_stencil &clipper, const il_stencil &clippee);
stencil_compound clip(const stencil_compound &clipper, const il_stencil &clippee);
stencil_compound clip(const il_stencil &clipper, const stencil_compound &clippee);
stencil_compound clip(const stencil_compound &clipper, const stencil_compound &clippee);

stencil_compound transform(const rvg::xform &xf, const vec_stencil &stencil_compound);
stencil_compound transform(const rvg::xform &xf, const il_stencil &stencil_compound);

rvg::scene scene(const painted_compound &p);
rvg::scene scene(const il_painted &p);
rvg::scene scene(const std::vector<painted_compound> &p);
rvg::scene scene(rvg::scene_data::const_ptr s);

rvg::paint solid_color(const rvg::RGBA8 &color, uint8_t opacity = 255);

rvg::color_ramp::const_ptr color_ramp(rvg::e_spread spread, const std::initializer_list<rvg::color_stop> &stops);
rvg::color_ramp::const_ptr color_ramp(const std::initializer_list<rvg::color_stop> &stops);

rvg::paint linear_gradient(rvg::color_ramp::const_ptr ramp_ptr,
    float x1, float y1, float x2, float y2,
    uint8_t opacity = 255);

rvg::paint radial_gradient(rvg::color_ramp::const_ptr ramp_ptr,
    float cx, float cy, float fx, float fy, float r,
    uint8_t opacity = 255);

rvg::paint texture(rvg::e_spread spread, rvg::i_image::const_ptr image_ptr,
    uint8_t opacity = 255);

rvg::paint texture(rvg::i_image::const_ptr image_ptr,
    uint8_t opacity = 255);

} } // namespace rvg_facade::driver

#endif
