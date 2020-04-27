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
#ifndef RVG_INPUT_PATH_F_XFORM_H
#define RVG_INPUT_PATH_F_XFORM_H

#include <type_traits>
#include <utility>

#include "rvg-i-sink.h"
#include "rvg-i-input-path-f-forwarder.h"
#include "rvg-xform.h"

namespace rvg {

template <typename XF, typename SINK>
class input_path_f_xform final:
    public i_sink<input_path_f_xform<XF,SINK>>,
    public i_input_path_f_forwarder<input_path_f_xform<XF,SINK>> {

    XF m_xf;
    SINK m_sink;

    rvgf m_x, m_y;

public:

    explicit input_path_f_xform(XF &&xf, SINK &&sink):
        m_xf(std::forward<XF>(xf)),
        m_sink(std::forward<SINK>(sink)) {
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
    }

private:

friend i_sink<input_path_f_xform<XF,SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

friend i_input_path<input_path_f_xform<XF,SINK>>;

    void do_begin_contour(rvgf x0, rvgf y0) {
        std::tie(x0, y0, std::ignore) = m_xf.apply(x0, y0);
        m_x = x0; m_y = y0;
        return m_sink.begin_contour(x0, y0);
    }

    void do_end_open_contour(rvgf x0, rvgf y0) {
        x0 = m_x; y0 = m_y;
        return m_sink.end_open_contour(x0, y0);
    }

    void do_end_closed_contour(rvgf x0, rvgf y0) {
        x0 = m_x; y0 = m_y;
        return m_sink.end_closed_contour(x0, y0);
    }

    void do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
        x0 = m_x; y0 = m_y;
        std::tie(x1, y1, std::ignore) = m_xf.apply(x1, y1);
        m_x = x1; m_y = y1;
        return m_sink.linear_segment(x0, y0, x1, y1);
    }

    void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2) {
        x0 = m_x; y0 = m_y;
        std::tie(x1, y1, std::ignore) = m_xf.apply(x1, y1);
        std::tie(x2, y2, std::ignore) = m_xf.apply(x2, y2);
        m_x = x2; m_y = y2;
        return m_sink.quadratic_segment(x0, y0, x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2) {
        x0 = m_x; y0 = m_y;
        std::tie(x1, y1, w1) = m_xf.apply(x1, y1, w1);
        std::tie(x2, y2, std::ignore) = m_xf.apply(x2, y2);
        m_x = x2; m_y = y2;
        return m_sink.rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2);
    }

    void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        x0 = m_x; y0 = m_y;
        std::tie(x1, y1, std::ignore) = m_xf.apply(x1, y1);
        std::tie(x2, y2, std::ignore) = m_xf.apply(x2, y2);
        std::tie(x3, y3, std::ignore) = m_xf.apply(x3, y3);
        m_x = x3; m_y = y3;
        return m_sink.cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3);
    }

};

template <typename XF, typename SINK>
auto make_input_path_f_xform(XF &&xf, SINK &&sink) {
    return input_path_f_xform<XF,SINK>{std::forward<XF>(xf), 
        std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
