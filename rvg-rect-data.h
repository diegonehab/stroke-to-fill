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
#ifndef RVG_RECT_DATA_H
#define RVG_RECT_DATA_H

#include "rvg-ptr.h"
#include "rvg-path-data.h"
#include "rvg-xform.h"

namespace rvg {

class rect_data:
    public boost::intrusive_ref_counter<rect_data> {

    rvgf m_x, m_y, m_width, m_height;

public:

    using ptr = boost::intrusive_ptr<rect_data>;
    using const_ptr = boost::intrusive_ptr<const rect_data>;

    rect_data(rvgf x, rvgf y, rvgf width, rvgf height):
        m_x(x),
        m_y(y),
        m_width(width),
        m_height(height) { ; }

    rvgf get_x(void) const { return m_x; }
    rvgf get_y(void) const { return m_y; }
    rvgf get_width(void) const { return m_width; }
    rvgf get_height(void) const { return m_height; }

    path_data::ptr as_path_data_ptr(const xform &post_xf) const {
        (void) post_xf;
        rvgf x2 = m_x + m_width;
        rvgf y2 = m_y + m_height;
        auto p = make_intrusive<path_data>();
        p->begin_contour(m_x, m_y);
        p->linear_segment(m_x, m_y, x2, m_y);
        p->linear_segment(x2, m_y, x2, y2);
        p->linear_segment(x2, y2, m_x, y2);
        p->linear_segment(m_x, y2, m_x, m_y);
        p->end_closed_contour(m_x, m_y);
        return p;
    }
};

} // namespace rvg

#endif
