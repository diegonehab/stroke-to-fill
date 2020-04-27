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
#ifndef RVG_CIRCLE_DATA_H
#define RVG_CIRCLE_DATA_H

#include "rvg-ptr.h"
#include "rvg-floatint.h"
#include "rvg-path-data.h"
#include "rvg-xform.h"

namespace rvg {

class circle_data:
    public boost::intrusive_ref_counter<circle_data> {

    rvgf m_cx, m_cy, m_r;

public:

    using ptr = boost::intrusive_ptr<circle_data>;
    using const_ptr = boost::intrusive_ptr<const circle_data>;

    circle_data(rvgf cx, rvgf cy, rvgf r):
        m_cx(cx),
        m_cy(cy),
        m_r(r) { ; }

    rvgf get_cx(void) const { return m_cx; }

    rvgf get_cy(void) const { return m_cy; }

    rvgf get_r(void) const { return m_r; }

    path_data::ptr as_path_data_ptr(const xform &post_xf) const {
        (void) post_xf;
        // we start with a unit circle centered at the origin
        // it is formed by 3 arcs covering each third of the unit circle
        // we then scale it by r and translate it by cx,cy
        static constexpr rvgf s = rvgf(0.5);                // sin(pi/6)
        static constexpr rvgf c = rvgf(0.8660254037844386); // cos(pi/6)
        static constexpr rvgf w = s;
        auto p = make_intrusive<path_data>();
        const rvgf x1 = m_cx,          y1 = m_r+m_cy;
        const rvgf x2 = -c*m_r+m_cx*w, y2 = s*m_r+m_cy*w, w2 = w;
        const rvgf x3 = -c*m_r+m_cx,   y3 = -s*m_r+m_cy;
        const rvgf x4 = m_cx*w,        y4 = -m_r+m_cy*w,  w4 = w;
        const rvgf x5 = c*m_r+m_cx,    y5 = -s*m_r+m_cy;
        const rvgf x6 = c*m_r+m_cx*w,  y6 = s*m_r+m_cy*w, w6 = w;
        p->begin_contour(x1, y1);
        p->rational_quadratic_segment(x1, y1, x2, y2, w2, x3, y3);
        p->rational_quadratic_segment(x3, y3, x4, y4, w4, x5, y5);
        p->rational_quadratic_segment(x5, y5, x6, y6, w6, x1, y1);
        p->end_closed_contour(x1, y1);
        return p;
    }
};

} // namespace rvg

#endif
