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
#ifndef RVG_INPUT_PATH_F_CLOSE_CONTOURS_H
#define RVG_INPUT_PATH_F_CLOSE_CONTOURS_H

#include <string>
#include <iosfwd>
#include <type_traits>
#include <utility>

#include "rvg-i-input-path-f-forwarder.h"
#include "rvg-i-sink.h"

namespace rvg {

// Make sure closed contours contain the segment connecting
// the last point to the first point
// Can also transform all open contours into closed contours.
template <typename SINK, bool CLOSE_ALL>
class input_path_f_close_contours final:
    public i_sink<input_path_f_close_contours<SINK,CLOSE_ALL>>,
    public i_input_path_f_forwarder<
        input_path_f_close_contours<SINK,CLOSE_ALL>> {


    rvgf m_first_x, m_first_y;
    bool m_open;
    SINK m_sink;

public:

    explicit input_path_f_close_contours(SINK &&sink):
        m_first_x(0), m_first_y(0),
        m_open(false),
        m_sink(std::forward<SINK>(sink)) {
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
    }

private:

friend i_sink<input_path_f_close_contours<SINK,CLOSE_ALL>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

friend i_input_path<input_path_f_close_contours<SINK,CLOSE_ALL>>;

    void do_begin_contour(rvgf x0, rvgf y0) {
        m_first_x = x0;
        m_first_y = y0;
        m_open = true;
        return do_sink().begin_contour(x0, y0);
    }

    void do_end_closed_contour(rvgf x0, rvgf y0) {
        if (m_open) {
            if (m_first_x != x0 || m_first_y != y0) {
                do_sink().linear_segment(x0, y0, m_first_x, m_first_y);
            }
            m_open = false;
            do_sink().end_closed_contour(m_first_x, m_first_y);
        }
    }

    void do_end_open_contour(rvgf x0, rvgf y0) {
        if (m_open) {
            if (CLOSE_ALL) {
                if (m_first_x != x0 || m_first_y != y0) {
                    do_sink().linear_segment(x0, y0, m_first_x, m_first_y);
                }
                do_sink().end_closed_contour(m_first_x, m_first_y);
            } else {
                do_sink().end_open_contour(x0, y0);
            }
            m_open = false;
        }
    }

};

template <typename SINK>
inline auto
make_input_path_f_close_contours(SINK &&sink) {
    return input_path_f_close_contours<SINK,false>{std::forward<SINK>(sink)};
}

template <typename SINK>
inline auto
make_input_path_f_close_all_contours(SINK &&sink) {
    return input_path_f_close_contours<SINK,true>{std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
