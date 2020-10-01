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
#ifndef RVG_INPUT_PATH_F_FIND_MONOTONIC_PARAMETERS_H
#define RVG_INPUT_PATH_F_FIND_MONOTONIC_PARAMETERS_H

#include <type_traits>
#include <utility>

#include <boost/range/adaptor/sliced.hpp>

#include "rvg-tuple.h"
#include "rvg-bezier.h"
#include "rvg-i-input-path-f-forwarder.h"
#include "rvg-i-parameters-f-forwarder.h"
#include "rvg-i-monotonic-parameters.h"

namespace rvg {

template <typename SINK>
class input_path_f_find_monotonic_parameters final:
    public i_sink<input_path_f_find_monotonic_parameters<SINK>>,
    public i_input_path_f_forwarder<
        input_path_f_find_monotonic_parameters<SINK>>,
    public i_parameters_f_forwarder<SINK,
        input_path_f_find_monotonic_parameters<SINK>> {

    SINK m_sink;

public:

    explicit input_path_f_find_monotonic_parameters(SINK &&sink):
        m_sink(std::forward<SINK>(sink)) {
        static_assert(meta::is_an_i_monotonic_parameters<SINK>::value,
            "sink is not an i_monotonic_parameters");
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
    }

private:

friend i_sink<input_path_f_find_monotonic_parameters<SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

friend i_input_path<input_path_f_find_monotonic_parameters<SINK>>;

    void quadratic_segment_extremes(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2) {
        using namespace boost::adaptors;
        auto x = std::make_tuple(x0, x1, x2);
        auto dx = bezier_differences(x);
        auto tdx = bezier_roots<rvgf>(dx);
        for (auto t: tdx | sliced(1, tdx.size()-1)) {
            m_sink.root_dx_parameter(t);
        }
        auto y = std::make_tuple(y0, y1, y2);
        auto dy = bezier_differences(y);
        auto tdy = bezier_roots<rvgf>(dy);
        for (auto t: tdy | sliced(1, tdy.size()-1)) {
            m_sink.root_dy_parameter(t);
        }
    }

    void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2) {
        quadratic_segment_extremes(x0, y0, x1, y1, x2, y2);
        return m_sink.quadratic_segment(x0, y0, x1, y1, x2, y2);
    }

    // To find the zeros of the derivatie of a rational curve, we can
    // ignore the denominator w^2 in x' = (w u' - u w')/w^2
    // and y' = (w v' - v w')/w^2
    // When computing tangents, we only care about the ratio
    // x':y', and therefore we can again ignore it w^2
    // So we always work with (w u' - u w') and (w v' - v w').
    void rational_quadratic_segment_extremes(rvgf u0, rvgf v0,
        rvgf u1, rvgf v1, rvgf w1, rvgf u2, rvgf v2) {
        using namespace boost::adaptors;
        auto w = std::make_tuple(1.f, w1, 1.f);
        auto dw = bezier_differences(w);

        auto u = std::make_tuple(u0, u1, u2);
        auto du = bezier_differences(u);
        auto dx = bezier_lower_degree(
            tuple_zip_map(
                bezier_product<rvgf>(w, du),
                bezier_product<rvgf>(u, dw),
                std::minus<rvgf>{}
            )
        );
        auto tdx = bezier_roots<rvgf>(dx);
        for (auto t: tdx | sliced(1, tdx.size()-1)) {
            m_sink.root_dx_parameter(t);
        }

        auto v = std::make_tuple(v0, v1, v2);
        auto dv = bezier_differences(v);
        auto dy = bezier_lower_degree(
            tuple_zip_map(
                bezier_product<rvgf>(w, dv),
                bezier_product<rvgf>(v, dw),
                std::minus<rvgf>{}
            )
        );
        auto tdy = bezier_roots<rvgf>(dy);
        for (auto t: tdy | sliced(1, tdy.size()-1)) {
            m_sink.root_dy_parameter(t);
        }
    }

    void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2) {
        rational_quadratic_segment_extremes(x0, y0, x1, y1, w1, x2, y2);
        return m_sink.rational_quadratic_segment(x0, y0, x1, y1, w1,
            x2, y2);
    }

    void cubic_segment_extremes(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        using namespace boost::adaptors;
        auto dx = bezier_differences(std::make_tuple(x0, x1, x2, x3));
        auto tdx = bezier_roots<rvgf>(dx);
        for (auto t: tdx | sliced(1, tdx.size()-1)) {
            m_sink.root_dx_parameter(t);
        }
        auto dy = bezier_differences(std::make_tuple(y0, y1, y2, y3));
        auto tdy = bezier_roots<rvgf>(dy);
        for (auto t: tdy | sliced(1, tdy.size()-1)) {
            m_sink.root_dy_parameter(t);
        }
    }

    void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        cubic_segment_extremes(x0, y0, x1, y1, x2, y2, x3, y3);
        return m_sink.cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3);
    }

};

template <typename SINK>
inline auto make_input_path_f_find_monotonic_parameters(SINK &&sink) {
    return input_path_f_find_monotonic_parameters<SINK>{std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
