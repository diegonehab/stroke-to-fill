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
#include "rvg-stroker-rvg.h"
#include "rvg-input-path-f-xform.h"
#include "rvg-input-path-f-stroke.h"

namespace rvg {
    namespace stroker {

shape rvg(const shape &input_shape, const xform &screen_xf, float width,
    stroke_style::const_ptr style) {
    auto output_path = make_intrusive<path_data>();
    input_shape.as_path_data_ptr(input_shape.get_xf().
        transformed(screen_xf))->iterate(
            make_input_path_f_xform(input_shape.get_xf(),
                make_input_path_f_stroke(
                    width, style, *output_path)));
    return shape{output_path};
}

} }
