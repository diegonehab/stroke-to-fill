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
#ifndef RVG_I_SCENE_DATA_F_FORWARDER_H
#define RVG_I_SCENE_DATA_F_FORWARDER_H

#include "rvg-i-scene-data.h"

namespace rvg {

template <typename DERIVED>
class i_scene_data_f_forwarder: public i_scene_data<DERIVED> {


    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }


public:

    i_scene_data_f_forwarder(void) {
        static_assert(meta::is_an_i_sink<DERIVED>::value,
            "derived class is not an i_sink");
    }

private:

friend i_scene_data<DERIVED>;

    // add a new painted shape to the scene
    void do_painted_shape(e_winding_rule rule, const shape &s, const paint &p) {
        return derived().sink().painted_shape(rule, s, p);
    }

    // add a new tensor_product_patch to the scene
    void do_tensor_product_patch(const patch<16,4> &tpp) {
        return derived().sink().tensor_product_patch(tpp);
    }

    // add a new coons_patch to the scene
    void do_coons_patch(const patch<12,4> &cp) {
        return derived().sink().coons_patch(cp);
    }

    // add a new gouraud_triangle to the scene
    void do_gouraud_triangle(const patch<3,3> &gt) {
        return derived().sink().gouraud_triangle(gt);
    }

    // add a new stencil shape to the scene
    void do_stencil_shape(e_winding_rule rule, const shape &shape) {
        return derived().sink().stencil_shape(rule, shape);
    }

    // clip and transparency group control
    void do_begin_clip(uint16_t depth) {
        return derived().sink().begin_clip(depth);
    }

    void do_activate_clip(uint16_t depth) {
        return derived().sink().activate_clip(depth);
    }

    void do_end_clip(uint16_t depth) {
        return derived().sink().end_clip(depth);
    }

    void do_begin_fade(uint16_t depth, unorm8 opacity) {
        return derived().sink().begin_fade(depth, opacity);
    }

    void do_end_fade(uint16_t depth, unorm8 opacity) {
        return derived().sink().end_fade(depth, opacity);
    }

    void do_begin_blur(uint16_t depth, float radius) {
        return derived().sink().begin_blur(depth, radius);
    }

    void do_end_blur(uint16_t depth, float radius) {
        return derived().sink().end_blur(depth, radius);
    }

    void do_begin_transform(uint16_t depth, const xform &xf) {
        return derived().sink().begin_transform(depth, xf);
    }

    void do_end_transform(uint16_t depth, const xform &xf) {
        return derived().sink().end_transform(depth, xf);
    }

};

} // namespace rvg

#endif
