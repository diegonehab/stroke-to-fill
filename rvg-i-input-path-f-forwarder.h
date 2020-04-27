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
#ifndef RVG_I_INPUT_PATH_F_FORWARDER_H
#define RVG_I_INPUT_PATH_F_FORWARDER_H

#include "rvg-i-sink.h"
#include "rvg-i-input-path.h"

namespace rvg {

template <typename DERIVED>
class i_input_path_f_forwarder: public i_input_path<DERIVED> {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    i_input_path_f_forwarder(void) {
        static_assert(meta::is_an_i_sink<DERIVED>::value,
            "derived class is not an i_sink");
    }

private:

// i_input_path interface
friend i_input_path<DERIVED>;

    void do_begin_contour(rvgf x0, rvgf y0) {
        return derived().sink().begin_contour(x0, y0);
    }

    void do_end_open_contour(rvgf x0, rvgf y0) {
        return derived().sink().end_open_contour(x0, y0);
    }

    void do_end_closed_contour(rvgf x0, rvgf y0) {
        return derived().sink().end_closed_contour(x0, y0);
    }

    void do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
        return derived().sink().linear_segment(x0, y0, x1, y1);
    }

    void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2) {
        return derived().sink().quadratic_segment(x0, y0, x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2) {
        return derived().sink().rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2);
    }

    void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        return derived().sink().cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3);
    }

};

} // namespace rvg

#endif
