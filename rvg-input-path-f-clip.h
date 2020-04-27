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
#ifndef RVG_INPUT_PATH_F_CLIP_H
#define RVG_INPUT_PATH_F_CLIP_H

#include "rvg-util.h"
#include "rvg-i-sink.h"
#include "rvg-i-point-input-path-f-forwarder.h"
#include "rvg-floatint.h"
#include "rvg-bezier.h"

namespace rvg {

// Assumes segments have already been monotonized
template<typename SINK, typename EVENT_CB, int IDX, bool LESS_THAN>
class input_path_f_clip :
    public i_point_input_path_f_forwarder<input_path_f_clip<SINK, EVENT_CB, IDX, LESS_THAN>>,
    public i_sink<input_path_f_clip<SINK, EVENT_CB, IDX, LESS_THAN>> {

    rvgf m_value;
    SINK m_sink;
    const EVENT_CB on_event;
public:

    input_path_f_clip(rvgf value, SINK &&s, const EVENT_CB &e):
        m_value(value),
        m_sink(std::forward<SINK>(s)),
        on_event(e) {
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
    }

friend i_sink<input_path_f_clip<SINK, EVENT_CB, IDX, LESS_THAN>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

    R2 proj(const R2 &p) {
        return p;
    }

    R2 proj(const R3 &p) {
        return R2{p[0]/p[2], p[1]/p[2]};
    }

    template<typename S1, typename S2, typename SINK_SEG>
    bool process_segment(const S1 &s, const S2 &s2, SINK_SEG sink_seg) {
        const auto &p0 = std::get<0>(s)[IDX];
        const auto &p1 = std::get<std::tuple_size<S1>::value - 1>(s)[IDX];

        bool a = LESS_THAN ? (p0 < m_value) : (p0 > m_value);
        bool b = LESS_THAN ? (p1 < m_value) : (p1 > m_value);

        if (a != b) {
            const auto roots = bezier_roots(s2, rvgf(0), rvgf(1), rvgf(0));
            rvgf root = roots[1];
            if (a) {
                auto ts = bezier_cut(s, root, rvgf(1));
                typename std::tuple_element<0, S1>::type t0 = bezier_evaluate_horner(s, root);
                sink_seg(t0, ts);
                R2 p = proj(t0);
                on_event(p, -1);
            } else {
                auto t0 = std::get<0>(s);
                auto ts = bezier_cut(s, rvgf(0), root);
                sink_seg(t0, ts);

                typename std::tuple_element<0, S1>::type tn = std::get<std::tuple_size<decltype(ts)>::value - 1>(ts);
                R2 p = proj(tn);
                on_event(p, 1);
            }
        } else {
            if (a == false)
                return false;
            else {
            }
        }
        return true;
    }

friend i_point_input_path<input_path_f_clip<SINK, EVENT_CB, IDX, LESS_THAN>>;

    void do_linear_segment(const R2 &p0, const R2 &p1) {
        bool a = LESS_THAN ? (p0[IDX] < m_value) : (p0[IDX] > m_value);
        bool b = LESS_THAN ? (p1[IDX] < m_value) : (p1[IDX] > m_value);
        if (a != b) {
            auto bz = std::tuple(p0[IDX], p1[IDX]);
            const auto roots = bezier_roots(bz, rvgf(0), rvgf(1), m_value);

            auto s = std::tie(p0, p1);
            if (a) {
                auto ts = bezier_cut(s, roots[1], roots[2]);
                auto p0 = bezier_evaluate_horner(s, roots[1]);
                m_sink.linear_segment(p0, std::get<0>(ts));
                on_event(p0, -1);
            } else {
                auto ts = bezier_cut(s, roots[0], roots[1]);
                m_sink.linear_segment(p0, std::get<0>(ts));
                on_event(std::get<0>(ts), 1);
            }
        } else {
            if (a == false)
                m_sink.linear_segment(p0, p1);
            else {
            }
        }
    }

    void do_quadratic_segment(const R2 &p0, const R2 &p1, const R2 &p2) {
        auto bz = std::tuple(p0[IDX] - m_value, p1[IDX] - m_value, p2[IDX] - m_value);
        auto s = std::tie(p0, p1, p2);

        auto push = [&](const R2 &p0, const std::tuple<const R2, const R2> &ts) {
            m_sink.quadratic_segment(p0, std::get<0>(ts), std::get<1>(ts));
        };

        if (!process_segment(s, bz, push))
            m_sink.quadratic_segment(p0, p1, p2);

    }

    void do_rational_quadratic_segment(const R3 &p0, const R3 &p1, const R3 &p2) {
        auto bz = std::tuple(p0[IDX] - m_value, p1[IDX] - m_value * p1[2], p2[IDX] - m_value);
        auto s = std::tie(p0, p1, p2);

        auto push = [&](const R3 &p0, const std::tuple<const R3, const R3> &ts) {
            auto a = canonize_rational_quadratic_bezier(p0, std::get<0>(ts), std::get<1>(ts));
            m_sink.rational_quadratic_segment(R3(std::get<0>(a)), std::get<1>(a), R3(std::get<2>(a)));
        };

        if (!process_segment(s, bz, push))
            m_sink.rational_quadratic_segment(p0, p1, p2);
    }

    void do_cubic_segment(const R2 &p0, const R2 &p1, const R2 &p2, const R2 &p3) {
        auto bz = std::tuple(p0[IDX] - m_value, p1[IDX] - m_value, p2[IDX] - m_value, p3[IDX] - m_value);
        auto s = std::tie(p0, p1, p2, p3);
        auto push = [&](const R2 &p0, const std::tuple<const R2, const R2, const R2> &ts) {
            m_sink.cubic_segment(p0, std::get<0>(ts), std::get<1>(ts), std::get<2>(ts));
        };

        if (!process_segment(s, bz, push))
            m_sink.cubic_segment(p0, p1, p2, p3);
    }
};

template <typename SINK, typename EVENT_CB>
inline auto make_input_path_f_clip_h_lt(rvgf value, const EVENT_CB &e, SINK &&sink) {
    return input_path_f_clip<SINK, EVENT_CB, 1, true>{value, std::forward<SINK>(sink), (e)};
}

template <typename SINK, typename EVENT_CB>
inline auto make_input_path_f_clip_h_gt(rvgf value, const EVENT_CB &e, SINK &&sink) {
    return input_path_f_clip<SINK, EVENT_CB, 1, false>{value, std::forward<SINK>(sink), (e)};
}

template <typename SINK, typename EVENT_CB>
inline auto make_input_path_f_clip_v_lt(rvgf value, const EVENT_CB &e, SINK &&sink) {
    return input_path_f_clip<SINK, EVENT_CB, 0, true>{value, std::forward<SINK>(sink), (e)};
}

template <typename SINK, typename EVENT_CB>
inline auto make_input_path_f_clip_v_gt(rvgf value, const EVENT_CB &e, SINK &&sink) {
    return input_path_f_clip<SINK, EVENT_CB, 0, false>{value, std::forward<SINK>(sink), (e)};
}

}
#endif
