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
#ifndef RVG_INPUT_PATH_F_MONOTONIZE_H
#define RVG_INPUT_PATH_F_MONOTONIZE_H

#include "rvg-floatint.h"
#include "rvg-i-point-input-path-f-forwarder.h"
#include "rvg-input-path-f-find-monotonic-parameters.h"
#include "rvg-input-path-f-find-cubic-parameters.h"
#include "rvg-canonize-rational-quadratic-bezier.h"

#include <boost/container/small_vector.hpp>
#include <boost/range/adaptor/sliced.hpp>

namespace rvg {
template<typename SINK>
class input_path_f_monotonize :
    public i_point_input_path_f_forwarder<input_path_f_monotonize<SINK>>,
    public i_sink<input_path_f_monotonize<SINK>>,
    public i_monotonic_parameters<input_path_f_monotonize<SINK>>,
    public i_cubic_parameters<input_path_f_monotonize<SINK>>{

    boost::container::small_vector<rvgf, 8> m_roots;
    SINK m_sink;
public:

    input_path_f_monotonize(SINK &&s) : m_sink{std::forward<SINK>(s)} {
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
    }

friend i_sink<input_path_f_monotonize<SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

    void do_root_dx_parameter(rvgf t) {
        m_roots.push_back(t);
    }

    void do_root_dy_parameter(rvgf t) {
        m_roots.push_back(t);
    }

    void do_root_dw_parameter(rvgf t) {
        (void)t;
    }

    void do_inflection_parameter(rvgf t) {
        m_roots.push_back(t);
    }

    void do_double_point_parameter(rvgf t) {
        m_roots.push_back(t);
    }

    template<typename S, typename SINK_SEG>
    void process_segment(const S &s, SINK_SEG sink_seg) {
        using namespace boost::adaptors;
        m_roots.push_back(0);
        std::sort(m_roots.begin(), m_roots.end());
        m_roots.push_back(1);

        rvgf prev_t = m_roots[0];
        auto p = std::get<0>(s);
        for (auto t: m_roots | sliced(1, m_roots.size())) {
            if (!util::is_almost_equal(t, prev_t)) {
                auto ts = bezier_cut(s, prev_t, t);
                sink_seg(p, ts);

                prev_t = t;
                p = std::get<std::tuple_size<decltype(ts)>::value - 1>(ts);
            }
        }
        m_roots.clear();
    }

friend i_point_input_path<input_path_f_monotonize<SINK>>;
    void do_quadratic_segment(const R2 &p0, const R2 &p1, const R2 &p2) {
        auto ps = std::tie(p0, p1, p2);
        process_segment(ps, [&](const R2 &p0, const auto &ts) {
            m_sink.quadratic_segment(p0, std::get<0>(ts), std::get<1>(ts));
        });
    }

    void do_rational_quadratic_segment(const R3 &p0, const R3 &p1, const R3 &p2) {
        auto ps = std::tie(p0, p1, p2);

        process_segment(ps, [&](const R3 &p0, const auto &ts) {
            auto a = canonize_rational_quadratic_bezier(p0, std::get<0>(ts), std::get<1>(ts));
            m_sink.rational_quadratic_segment(R3(std::get<0>(a)), std::get<1>(a), R3(std::get<2>(a)));
        });
    }

    void do_cubic_segment(const R2 &p0, const R2 &p1, const R2 &p2, const R2 &p3) {
        auto ps = std::tie(p0, p1, p2, p3);

        process_segment(ps, [&](const R2 &p0, const auto &ts) {
            m_sink.cubic_segment(p0, std::get<0>(ts), std::get<1>(ts), std::get<2>(ts));
        });
    }
};

template <typename SINK>
inline auto make_input_path_f_monotonize(SINK &&sink) {
    return make_input_path_f_find_monotonic_parameters(
        make_input_path_f_find_cubic_parameters(
            input_path_f_monotonize<SINK>{std::forward<SINK>(sink)}
        )
    );
}

}
#endif
