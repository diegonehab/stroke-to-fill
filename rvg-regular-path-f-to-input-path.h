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
#ifndef RVG_REGULAR_PATH_F_TO_INPUT_PATH_H
#define RVG_REGULAR_PATH_F_TO_INPUT_PATH_H

#include "rvg-i-point-regular-path.h"
#include "rvg-i-input-path.h"
#include "rvg-bezier.h"
#include "rvg-i-parameters-f-hold.h"

namespace rvg {

template <typename SINK>
class regular_path_f_to_input_path final:
    public i_sink<regular_path_f_to_input_path<SINK>>,
    public i_point_regular_path<regular_path_f_to_input_path<SINK>>,
    public i_parameters_f_hold<regular_path_f_to_input_path<SINK>> {

    SINK m_sink;

public:

    regular_path_f_to_input_path(SINK &&sink):
        m_sink(std::forward<SINK>(sink)) {
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is an i_input_path");
    }

private:

friend i_sink<regular_path_f_to_input_path<SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

friend i_point_regular_path<regular_path_f_to_input_path<SINK>>;

    void do_begin_regular_contour(const R2 &p, const R2 &) {
        m_sink.begin_contour(p);
    }

    void do_end_regular_closed_contour(const R2 &, const R2 &p) {
        m_sink.end_closed_contour(p);
    }

    void do_end_regular_open_contour(const R2 &, const R2 &p) {
        m_sink.end_open_contour(p);
    }

    void do_cusp(const R2 &, const R2 &, const R2 &, rvgf) {
        ;
    }

    void do_inner_cusp(const R2 &, const R2 &, const R2 &, rvgf) {
        ;
    }

    void do_degenerate_segment(const R2 &p0, const R2 &, const R2 &p1) {
        m_sink.linear_segment(p0, p1);
    }

    void do_begin_segment_piece(const R2 &, const R2 &, const R2 &,
        const R2 &) {
        ;
    }

    void do_end_segment_piece(const R2 &, const R2 &, const R2 &,
        const R2 &) {
        ;
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1) {
        m_sink.linear_segment(p0*(1-ti)+p1*ti, p0*(1-tf)+p1*tf);
        this->clear_parameters();
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
        const R2 &p1, const R2 &p2) {
        auto s = std::tie(p0, p1, p2);
        auto q0 = bezier_evaluate_horner<rvgf>(s, ti);
        R2 q1, q2;
        std::tie(q1, q2) = bezier_cut(s, ti, tf);
        m_sink.quadratic_segment(q0, q1, q2);
        this->clear_parameters();
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, const R3 &p0,
        const R3 &p1, const R3 &p2) {
        auto s = std::tie(p0, p1, p2);
        auto q0 = bezier_evaluate_horner<rvgf>(s, ti);
        R3 q1, q2;
        std::tie(q1, q2) = bezier_cut(s, ti, tf);
        m_sink.rational_quadratic_segment(q0, q1, q2);
        this->clear_parameters();
    }

    void do_regular_cubic_segment(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1,
        const R2 &p2, const R2 &p3) {
        auto s = std::tie(p0, p1, p2, p3);
        auto q0 = bezier_evaluate_horner<rvgf>(s, ti);
        R2 q1, q2, q3;
        std::tie(q1, q2, q3) = bezier_cut(s, ti, tf);
        m_sink.cubic_segment(q0, q1, q2, q3);
        this->clear_parameters();
    }

};

template <typename SINK>
static auto make_regular_path_f_to_input_path(SINK &&sink) {
    return regular_path_f_to_input_path<SINK>(std::forward<SINK>(sink));
}

} // namespace rvg

#endif
