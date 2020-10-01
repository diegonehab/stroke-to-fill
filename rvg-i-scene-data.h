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
#ifndef RVG_I_SCENE_DATA_H
#define RVG_I_SCENE_DATA_H

#include <cstdint>

#include "rvg-xform.h"
#include "rvg-meta.h"
#include "rvg-shape.h"
#include "rvg-paint.h"
#include "rvg-unorm.h"
#include "rvg-winding-rule.h"
#include "rvg-patch.h"

namespace rvg {

template <typename DERIVED>
class i_scene_data {

protected:

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    // add a new painted shape to the scene
    void painted_shape(e_winding_rule rule, const shape &s, const paint &p) {
        return derived().do_painted_shape(rule, s, p);
    }

    // add a new tensor_product_patch to the scene
    void tensor_product_patch(const patch<16,4> &tpp) {
        return derived().do_tensor_product_patch(tpp);
    }

    // add a new coons_patch to the scene
    void coons_patch(const patch<12,4> &cp) {
        return derived().do_coons_patch(cp);
    }

    // add a new gouraud_triangle to the scene
    void gouraud_triangle(const patch<3,3> &gt) {
        return derived().do_gouraud_triangle(gt);
    }

    // add a new stencil shape to the scene
    void stencil_shape(e_winding_rule rule, const shape &shape) {
        return derived().do_stencil_shape(rule, shape);
    }

    // clip and transparency group control
    void begin_clip(uint16_t depth) {
        return derived().do_begin_clip(depth);
    }

    void activate_clip(uint16_t depth) {
        return derived().do_activate_clip(depth);
    }

    void end_clip(uint16_t depth) {
        return derived().do_end_clip(depth);
    }

    void begin_fade(uint16_t depth, unorm8 opacity) {
        return derived().do_begin_fade(depth, opacity);
    }

    void end_fade(uint16_t depth, unorm8 opacity) {
        return derived().do_end_fade(depth, opacity);
    }

    void begin_blur(uint16_t depth, float radius) {
        return derived().do_begin_blur(depth, radius);
    }

    void end_blur(uint16_t depth, float radius) {
        return derived().do_end_blur(depth, radius);
    }

    void begin_transform(uint16_t depth, const xform &xf) {
        return derived().do_begin_transform(depth, xf);
    }

    void end_transform(uint16_t depth, const xform &xf) {
        return derived().do_end_transform(depth, xf);
    }

};

} // namespace rvg

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_scene_data = std::integral_constant<
    bool,
    is_template_base_of<
        rvg::i_scene_data,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

} } // namespace rvg::meta

#endif
