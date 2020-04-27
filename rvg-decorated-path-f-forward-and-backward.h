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
#ifndef RVG_DECORATED_PATH_F_FORWARD_AND_BACKWARD_H
#define RVG_DECORATED_PATH_F_FORWARD_AND_BACKWARD_H

#include <utility>

#include "rvg-i-point-regular-path.h"
#include "rvg-i-point-decorated-path.h"
#include "rvg-i-monotonic-parameters.h"
#include "rvg-i-dashing-parameters.h"
#include "rvg-i-offsetting-parameters.h"
#include "rvg-path-data.h"

namespace rvg {

template <typename SINK>
class decorated_path_f_forward_and_backward final:
    public i_sink<decorated_path_f_forward_and_backward<SINK>>,
    public i_point_regular_path<decorated_path_f_forward_and_backward<SINK>>,
    public i_parameters_f_hold<decorated_path_f_forward_and_backward<SINK>>,
    public i_point_decorated_path<decorated_path_f_forward_and_backward<SINK>> {

    path_data m_saved;
    SINK m_sink;

public:

    explicit decorated_path_f_forward_and_backward(SINK &&sink):
        m_sink(std::forward<SINK>(sink)) {
        static_assert(rvg::meta::is_an_i_regular_path<SINK>::value,
            "sink is not an i_regular_path");
        static_assert(rvg::meta::is_an_i_decorated_path<SINK>::value,
            "sink is not an i_decorated_path");
        static_assert(rvg::meta::is_an_i_dashing_parameters<SINK>::value,
            "sink is not an i_dashing_parameters");
    }

    void flush(void) {
        m_saved.iterate(m_sink);
        m_saved.riterate(m_sink);
        m_saved.clear();
    }

private:

friend i_sink<decorated_path_f_forward_and_backward<SINK>>;

    path_data &do_sink(void) {
        return m_saved; // so parameters_f_hold sends parameters to m_saved
    }

    const path_data &do_sink(void) const {
        return m_saved; // so parameters_f_hold sends parameters to m_saved
    }

friend i_point_decorated_path<decorated_path_f_forward_and_backward<SINK>>;

    void do_initial_cap(const R2 &p, const R2 &d) {
        m_saved.initial_cap(p, d);
    }

    void do_terminal_cap(const R2 &d, const R2 &p) {
        m_saved.terminal_cap(d, p);
        flush();
    }

    void do_backward_initial_cap(const R2 &p, const R2 &d) {
        (void) p; (void) d;
        assert(0);
    }

    void do_backward_terminal_cap(const R2 &d, const R2 &p) {
        (void) p; (void) d;
        assert(0);
    }

    void do_initial_butt_cap(const R2 &p, const R2 &d) {
        m_saved.initial_butt_cap(p, d);
    }

    void do_terminal_butt_cap(const R2 &d, const R2 &p) {
        m_saved.terminal_butt_cap(d, p);
        flush();
    }

    void do_backward_initial_butt_cap(const R2 &p, const R2 &d) {
        (void) p; (void) d;
        assert(0);
    }

    void do_backward_terminal_butt_cap(const R2 &d, const R2 &p) {
        (void) p; (void) d;
        assert(0);
    }

    void do_join(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        m_saved.join(d0, p, d1, w);
    }

    void do_inner_join(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        m_saved.inner_join(d0, p, d1, w);
    }

friend i_point_regular_path<decorated_path_f_forward_and_backward<SINK>>;

    void do_begin_regular_contour(const R2 &, const R2 &) {
        assert(0); // should have been filtered out
    }

    void do_end_regular_open_contour(const R2 &, const R2 &) {
        assert(0); // should have been filtered out
    }

    void do_end_regular_closed_contour(const R2 &, const R2 &) {
        assert(0); // should have been filtered out
    }

    void do_begin_segment_piece(const R2 &, const R2 &) {
        assert(0); // should have been filtered out
    }

    void do_end_segment_piece(const R2 &, const R2 &) {
        assert(0); // should have been filtered out
    }

    void do_degenerate_segment(const R2 &p0, const R2 &d, const R2 &p1) {
        m_saved.degenerate_segment(p0, d, p1);
    }

    void do_cusp(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        m_saved.cusp(d0, p, d1, w);
    }

    void do_inner_cusp(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        m_saved.inner_cusp(d0, p, d1, w);
    }

    template <typename PIECE>
    void process_segment(rvgf ti, rvgf tf, const PIECE &piece) {
        this->sort_parameters();
        auto t0 = ti;
        for (auto &p: this->parameters()) {
            const auto &t = p.second.f;
            if (p.first == path_instruction::end_dash_parameter) {
                this->forward_parameters(t0, t);
                m_saved.end_dash_parameter(t);
                piece();
                flush();
                t0 = tf;
            } else if (p.first == path_instruction::begin_dash_parameter) {
                m_saved.begin_dash_parameter(t);
                t0 = t;
            }
        }
        if (t0 < tf) {
            this->forward_parameters(t0, tf);
            piece();
        }
        this->clear_parameters();
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1) {
        process_segment(ti, tf, [&](void) {
            m_saved.linear_segment_piece(ti, tf, p0, p1);
        });
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
        const R2 &p1, const R2 &p2) {
        process_segment(ti, tf, [&](void) {
            m_saved.quadratic_segment_piece(ti, tf, p0, p1, p2);
        });
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, const R3 &p0,
        const R3 &p1, const R3 &p2) {
        process_segment(ti, tf, [&](void) {
            m_saved.rational_quadratic_segment_piece(ti, tf, p0, p1, p2);
        });
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1,
        const R2 &p2, const R2 &p3) {
        process_segment(ti, tf, [&](void) {
            m_saved.cubic_segment_piece(ti, tf, p0, p1, p2, p3);
        });
    }
};


template <typename SINK>
static auto
make_decorated_path_f_forward_and_backward(SINK &&sink) {
    return decorated_path_f_forward_and_backward<SINK>{std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
