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
#ifndef RVG_SCENE_F_NULL_H
#define RVG_SCENE_F_NULL_H

#include "rvg-lua.h"
#include "rvg-i-scene-data.h"

namespace rvg {

class scene_f_null: public i_scene_data<scene_f_null> {

friend i_scene_data<scene_f_null>;

	void do_painted_shape(e_winding_rule, const shape &, const paint &) { }
	void do_tensor_product_patch(const patch<16,4> &) { }
	void do_coons_patch(const patch<12,4> &) { }
	void do_gouraud_triangle(const patch<3,3> &) { }

	void do_stencil_shape(e_winding_rule, const shape &) { }
	void do_begin_clip(uint16_t) { }
	void do_end_clip(uint16_t) { }
	void do_activate_clip(uint16_t) { }

    void do_begin_fade(uint16_t, unorm8) { }
    void do_end_fade(uint16_t, unorm8) { }

    void do_begin_blur(uint16_t, float) { }
    void do_end_blur(uint16_t, float) { }

    void do_begin_transform(uint16_t, const xform &) { }
    void do_end_transform(uint16_t, const xform &) { }
};

static inline auto
make_scene_f_null(void) {
    return scene_f_null{};
}

} // namespace rvg

#endif
