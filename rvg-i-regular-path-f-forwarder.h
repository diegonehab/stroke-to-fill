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
#ifndef RVG_I_REGULAR_PATH_F_FORWARDER_H
#define RVG_I_REGULAR_PATH_F_FORWARDER_H

#include "rvg-i-sink.h"
#include "rvg-i-regular-path.h"
#include "rvg-i-point-regular-path.h"

namespace rvg {

template <typename DERIVED>
class i_regular_path_f_forwarder: public i_regular_path<DERIVED> {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    i_regular_path_f_forwarder(void) {
        static_assert(meta::is_an_i_sink<DERIVED>::value,
            "derived class is not an i_sink");
    }

private:

// i_regular_path interface
friend i_regular_path<DERIVED>;

    void do_begin_regular_contour(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi) {
        return derived().sink().begin_regular_contour(xi, yi, dxi, dyi);
    }

    void do_end_regular_open_contour(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
        return derived().sink().end_regular_open_contour(dxf, dyf, xf, yf);
    }

    void do_end_regular_closed_contour(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
        return derived().sink().end_regular_closed_contour(dxf, dyf, xf, yf);
    }

    void do_degenerate_segment(rvgf xi, rvgf yi, rvgf dx, rvgf dy,
        rvgf xf, rvgf yf) {
        return derived().sink().degenerate_segment(xi, yi, dx, dy, xf, yf);
    }

    void do_cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w) {
        return derived().sink().cusp(dxi, dyi, x, y, dxf, dyf, w);
    }

    void do_inner_cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w) {
        return derived().sink().inner_cusp(dxi, dyi, x, y, dxf, dyf, w);
    }

    void do_begin_segment_piece(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi) {
        return derived().sink().begin_segment_piece(xi, yi, dxi, dyi);
    }

    void do_end_segment_piece(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
        return derived().sink().end_segment_piece(dxf, dyf, xf, yf);
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1,
        rvgf y1) {
        return derived().sink().linear_segment_piece(ti, tf, x0, y0, x1, y1);
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        return derived().sink().quadratic_segment_piece(ti, tf, x0, y0, x1, y1,
            x2, y2);
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        return derived().sink().rational_quadratic_segment_piece(ti, tf, x0, y0,
            x1, y1, w1, x2, y2);
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1,
        rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        return derived().sink().cubic_segment_piece(ti, tf, x0, y0, x1, y1,
            x2, y2, x3, y3);
    }

};

} // namespace rvg

#endif
