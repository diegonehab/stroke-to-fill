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
#ifndef RVG_I_POINT_INPUT_PATH_F_FORWARDER_H
#define RVG_I_POINT_INPUT_PATH_F_FORWARDER_H

#include "rvg-i-point-input-path.h"
#include "rvg-i-sink.h"

namespace rvg {

template <typename DERIVED>
class i_point_input_path_f_forwarder: public i_point_input_path<DERIVED> {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    i_point_input_path_f_forwarder(void) {
        static_assert(meta::is_an_i_sink<DERIVED>::value,
            "derived class is not an i_sink");
    }

private:

// i_point_input_path interface
friend i_point_input_path<DERIVED>;

    void do_begin_contour(const R2 &p0) {
        return derived().sink().begin_contour(p0);
    }

    void do_end_open_contour(const R2 &p0) {
        return derived().sink().end_open_contour(p0);
    }

    void do_end_closed_contour(const R2 &p0) {
        return derived().sink().end_closed_contour(p0);
    }

    void do_linear_segment(const R2 &p0, const R2 &p1) {
        return derived().sink().linear_segment(p0, p1);
    }

    void do_quadratic_segment(const R2 &p0, const R2 &p1, const R2 &p2) {
        return derived().sink().quadratic_segment(p0, p1, p2);
    }

    void do_rational_quadratic_segment(const R3 &p0, const R3 &p1,
        const R3 &p2) {
        return derived().sink().rational_quadratic_segment(p0, p1, p2);
    }

    void do_cubic_segment(const R2 &p0, const R2 &p1, const R2 &p2,
        const R2 &p3) {
        return derived().sink().cubic_segment(p0, p1, p2, p3);
    }

};

} // namespace rvg

#endif
