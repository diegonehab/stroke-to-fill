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
#ifndef RVG_INPUT_PATH_F_FIXER_H
#define RVG_INPUT_PATH_F_FIXER_H

#include "rvg-util.h"
#include "rvg-i-sink.h"
#include "rvg-i-point-input-path-f-forwarder.h"
#include "rvg-floatint.h"

namespace rvg {

template<typename SINK>
class input_path_f_fixer :
    public i_point_input_path_f_forwarder<input_path_f_fixer<SINK>>,
    public i_sink<input_path_f_fixer<SINK>> {

    rvgf EPS;
    SINK m_sink;
public:

    input_path_f_fixer(SINK &&s): EPS(1e-4),
        m_sink(std::forward<SINK>(s)) {
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
    }

friend i_sink<input_path_f_fixer<SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

friend i_point_input_path<input_path_f_fixer<SINK>>;

    void fix(rvgf &val, rvgf vmin, rvgf vmax) {
        if (val < vmin) val = vmin;
        if (val > vmax) val = vmax;
    }

    void do_quadratic_segment(const R2 &p0, const R2 &p1, const R2 &p2) {
        R2 a0 = p0, a1 = p1, a2 = p2;
        rvgf xmin, xmax, ymin, ymax;
        minmax(a0, a2, xmin, ymin, xmax, ymax);

        fix(a1[0], xmin, xmax);
        fix(a1[1], ymin, ymax);

        m_sink.quadratic_segment(a0, a1, a2);
    }

    void do_rational_quadratic_segment(const R3 &p0, const R3 &p1, const R3 &p2) {
        rvgf xmin, xmax, ymin, ymax;
        R3 a0 = p0,a1 = p1, a2 = p2;
        minmax(a0, a2, xmin, ymin, xmax, ymax);
        m_sink.rational_quadratic_segment(a0, a1, a2);
    }

    template<typename T>
    void minmax(const T &p0, const T &p3, rvgf &xmin, rvgf &ymin, rvgf &xmax, rvgf &ymax) const {
        if (p0[0] < p3[0]) { xmin = p0[0]; xmax=p3[0]; }
        else { xmin = p3[0]; xmax=p0[0]; }
        if (p0[1] < p3[1]) { ymin = p0[1]; ymax=p3[1]; }
        else { ymin = p3[1]; ymax=p0[1]; }
    }

    void do_cubic_segment(const R2 &p0, const R2 &p1, const R2 &p2, const R2 &p3) {
        R2 a0 = p0, a1 = p1, a2 = p2, a3 = p3;
		rvgf xmin, xmax, ymin, ymax;
        minmax(a0, a3, xmin, ymin, xmax, ymax);

        fix(a1[0], xmin, xmax);
        fix(a1[1], ymin, ymax);

        fix(a2[0], xmin, xmax);
        fix(a2[1], ymin, ymax);

        //bbox diagonal
        double a = a3[1] - a0[1];
        double b = a0[0] - a3[0];
        double c = -(a * a0[0] + b * a0[1]);

        //collapse tanget if it is too small
        {
            auto d1 = a0 - a1;
            if (std::abs(d1[0] / b) < EPS && std::abs(d1[1] / a) < EPS)
                a1 = a0;

            auto d2 = a2 - a3;
            if (std::abs(d2[0] / b) < EPS && std::abs(d2[1] / a) < EPS)
                a2 = a3;
        }

        // make sure tangents are on the same side
        bool t1 = (a * a1[0] + b * a1[1] + c) < 0.0;
        bool t2 = (a * a2[0] + b * a2[1] + c) < 0.0;
        if (t1 != t2) {
            auto d1 = a0 - a1;
            auto d2 = a2 - a3;
            rvgf l1 = d1[0] * d1[0] + d1[1] * d1[1];
            rvgf l2 = d2[0] * d2[0] + d2[1] * d2[1];
            if (l1 < l2) a1 = a0;
            else a2 = a3;
        }


        m_sink.cubic_segment(a0, a1, a2, a3);
    }
};

template <typename SINK>
inline auto make_input_path_f_fixer(SINK &&sink) {
    return input_path_f_fixer<SINK>{std::forward<SINK>(sink)};
}

}
#endif
