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
#ifndef RVG_TRIANGLE_DATA_H
#define RVG_TRIANGLE_DATA_H

#include "rvg-ptr.h"
#include "rvg-path-data.h"
#include "rvg-xform.h"

namespace rvg {

class triangle_data:
    public boost::intrusive_ref_counter<triangle_data> {

    rvgf m_x1, m_y1, m_x2, m_y2, m_x3, m_y3;

public:

    using ptr = boost::intrusive_ptr<triangle_data>;
    using const_ptr = boost::intrusive_ptr<const triangle_data>;

    triangle_data(rvgf x1, rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3):
        m_x1(x1),
        m_y1(y1),
        m_x2(x2),
        m_y2(y2),
        m_x3(x3),
        m_y3(y3) { ; }

    rvgf get_x1(void) const { return m_x1; }
    rvgf get_y1(void) const { return m_y1; }
    rvgf get_x2(void) const { return m_x2; }
    rvgf get_y2(void) const { return m_y2; }
    rvgf get_x3(void) const { return m_x3; }
    rvgf get_y3(void) const { return m_y3; }

    path_data::ptr as_path_data_ptr(const xform &post_xf) const {
        (void) post_xf;
        auto p = make_intrusive<path_data>();
        p->begin_contour(m_x1, m_y1);
        p->linear_segment(m_x1, m_y1, m_x2, m_y2);
        p->linear_segment(m_x2, m_y2, m_x3, m_y3);
        p->linear_segment(m_x3, m_y3, m_x1, m_y1);
        p->end_closed_contour(m_x1, m_y1);
        return p;
    }
};

} // namespace rvg

#endif
