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
#include "rvg-shape.h"
#include "rvg-input-path-f-stroke.h"
#include "rvg-input-path-f-xform.h"
#include "strokers/rvg-stroker-rvg.h"

namespace rvg {

path_data::const_ptr
shape::as_path_data_ptr(const xform &post_xf) const {
    xform pxf = get_xf().transformed(post_xf);
    switch (m_type) {
        case e_type::path:
            return m_union.path_ptr;
        case e_type::circle:
            return m_union.circle_ptr->as_path_data_ptr(pxf);
        case e_type::triangle:
            return m_union.triangle_ptr->as_path_data_ptr(pxf);
        case e_type::rect:
            return m_union.rect_ptr->as_path_data_ptr(pxf);
        case e_type::polygon:
            return m_union.polygon_ptr->as_path_data_ptr(pxf);
        case e_type::stroke: {
            const auto &s = m_union.stroke;
            return stroker::rvg(s.get_shape(), pxf, s.get_width(),
                s.get_style_ptr()).as_path_data_ptr(pxf);
        }
        case e_type::blend: // ??D not implemented yet
        case e_type::empty:
        default:
            return make_intrusive<path_data>();
    }
}

const stroke_style::const_ptr
    shape::stroke_data::default_stroke_style_ptr =
        make_intrusive<stroke_style>();

} // namespace rvg
