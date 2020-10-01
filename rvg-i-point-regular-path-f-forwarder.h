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
#ifndef RVG_I_POINT_REGULAR_PATH_F_FORWARDER_H
#define RVG_I_POINT_REGULAR_PATH_F_FORWARDER_H

#include "rvg-i-sink.h"
#include "rvg-i-point-regular-path.h"

namespace rvg {

template <typename DERIVED>
class i_point_regular_path_f_forwarder:
    public i_point_regular_path<DERIVED> {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    i_point_regular_path_f_forwarder(void) {
        static_assert(meta::is_an_i_sink<DERIVED>::value,
            "derived class is not an i_sink");
    }

private:

// i_regular_path interface
friend i_point_regular_path<DERIVED>;

    void do_begin_regular_contour(const R2 &pi, const R2 &di) {
        return derived().sink().begin_regular_contour(pi, di);
    }

    void do_end_regular_open_contour(const R2 &df, const R2 &pf) {
        return derived().sink().end_regular_open_contour(df, pf);
    }

    void do_end_regular_closed_contour(const R2 &df, const R2 &pf) {
        return derived().sink().end_regular_closed_contour(df, pf);
    }

    void do_degenerate_segment(const R2 &pi, const R2 &d, const R2 &pf) {
        return derived().sink().degenerate_segment(pi, d, pf);
    }

    void do_cusp(const R2 &di, const R2 &p, const R2 &df, rvgf w) {
        return derived().sink().cusp(di, p, df, w);
    }

    void do_inner_cusp(const R2 &di, const R2 &p, const R2 &df, rvgf w) {
        return derived().sink().inner_cusp(di, p, df, w);
    }

    void do_begin_segment_piece(const R2 &pi, const R2 &di) {
        return derived().sink().begin_segment_piece(pi, di);
    }

    void do_end_segment_piece(const R2 &di, const R2 &pi) {
        return derived().sink().end_segment_piece(di, pi);
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1) {
        return derived().sink().linear_segment_piece(ti, tf, p0, p1);
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
        const R2 &p1, const R2 &p2) {
        return derived().sink().quadratic_segment_piece(ti, tf, p0, p1, p2);
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, const R3 &p0,
        const R3 &p1, const R3 &p2) {
        return derived().sink().rational_quadratic_segment_piece(ti, tf, p0,
            p1, p2);
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
        const R2 &p1, const R2 &p2, const R2 &p3) {
        return derived().sink().cubic_segment_piece(ti, tf, p0, p1, p2, p3);
    }
};

} // namespace rvg

#endif
