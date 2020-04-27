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
#ifndef RVG_DECORATED_PATH_F_SIMPLIFY_JOINS_H
#define RVG_DECORATED_PATH_F_SIMPLIFY_JOINS_H

#include <cassert>

#include "rvg-i-point-regular-path-f-forwarder.h"
#include "rvg-i-point-decorated-path-f-forwarder.h"
#include "rvg-regular-path-f-find-join-parameters.h"
#include "rvg-path-f-null.h"
#include "rvg-i-parameters-f-forwarder.h"
#include "rvg-i-offsetting-parameters.h"
#include "rvg-i-join-parameters.h"
#include "rvg-path-data.h"

namespace rvg {

namespace detail {

    class path_f_get_join_or_cusp:
        public i_point_regular_path_f_forwarder<path_f_get_join_or_cusp>,
        public i_point_decorated_path_f_forwarder<path_f_get_join_or_cusp>,
        public i_sink<path_f_get_join_or_cusp> {

        bool m_is_join, m_is_cusp;
        R2 m_d0, m_p, m_d1;
        rvgf m_w;

        path_f_null m_sink;

    public:

        path_f_get_join_or_cusp(void):
            m_is_join{false},
            m_is_cusp{false} { ; }

        const R2 &get_d0(void) const {
            return m_d0;
        }

        const R2 &get_p(void) const {
            return m_p;
        }

        const R2 &get_d1(void) const {
            return m_d1;
        }

        rvgf get_w(void) const {
            return m_w;
        }

        bool is_join(void) const {
            return m_is_join;
        }

        bool is_cusp(void) const {
            return m_is_cusp;
        }

    private:

    friend i_sink<path_f_get_join_or_cusp>;

        const auto &do_sink(void) const {
            return m_sink;
        }

        auto &do_sink(void) {
            return m_sink;
        }

    friend i_point_regular_path<path_f_get_join_or_cusp>;

        void do_cusp(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
            m_d0 = d0;
            m_p = p;
            m_d1 = d1;
            m_w = w;
            m_is_cusp = true;
        }

        void do_inner_cusp(const R2 &, const R2 &, const R2 &, rvgf) {
            assert(0);
        }

    friend i_point_decorated_path<path_f_get_join_or_cusp>;

        void do_join(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
            m_d0 = d0;
            m_p = p;
            m_d1 = d1;
            m_w = w;
            m_is_join = true;
        }

        void do_inner_join(const R2 &, const R2 &, const R2 &, rvgf) {
            assert(0);
        }

    };

    static inline path_f_get_join_or_cusp
    make_path_f_get_join_or_cusp(void) {
        return path_f_get_join_or_cusp{};
    }

    class path_f_covers_inner_join_or_cusp:
        public i_offsetting_parameters<path_f_covers_inner_join_or_cusp>,
        public i_join_parameters<path_f_covers_inner_join_or_cusp>,
        public i_point_regular_path_f_forwarder<path_f_covers_inner_join_or_cusp>,
        public i_sink<path_f_covers_inner_join_or_cusp> {

        rvgf m_offset;
        R2 m_vertex;
        R2 m_d0, m_center, m_d1;
        rvgf &m_w;
        rvgf m_join_vertex_parameter;
        rvgf m_join_tangent_parameter;
        rvgf m_offset_cusp_parameter;

        path_f_null m_sink;

    public:

        path_f_covers_inner_join_or_cusp(rvgf offset, const R2 &vertex,
            const R2 &d0, const R2 &p, const R2 &d1, rvgf &w):
            m_offset{offset},
            m_vertex{vertex},
            m_d0{d0}, m_center{p}, m_d1{d1},
            m_w{w},
            m_join_vertex_parameter{2.f},
            m_join_tangent_parameter{2.f},
            m_offset_cusp_parameter{1.f}
            { ; }

    private:

    friend i_join_parameters<path_f_covers_inner_join_or_cusp>;

        void do_join_tangent_parameter(rvgf t) {
            m_join_tangent_parameter = std::fmin(t, m_join_tangent_parameter);
        }

        void do_join_vertex_parameter(rvgf t) {
            m_join_vertex_parameter = std::fmin(t, m_join_vertex_parameter);
        }

    friend i_offsetting_parameters<path_f_covers_inner_join_or_cusp>;

        void do_offset_cusp_parameter(rvgf t) {
            m_offset_cusp_parameter = std::fmin(t, m_offset_cusp_parameter);
        }

        void do_evolute_cusp_parameter(rvgf) {
            ;
        }

    friend i_sink<path_f_covers_inner_join_or_cusp>;

        const auto &do_sink(void) const {
            return m_sink;
        }

        auto &do_sink(void) {
            return m_sink;
        }

    friend i_point_regular_path<path_f_covers_inner_join_or_cusp>;

        bool vertex_covers_by_normal(const R2 &normal) const {
            // If parameter where normal goes through the vertex happens
            // after an offset cusp, we cannot consider it
            if (m_join_vertex_parameter > m_offset_cusp_parameter) {
                return false;
            }
            // Compute the direction from center to the other
            // intersection between the normal and the circle
            R2 other_direction = m_vertex - m_center -
                2*dot(normal, m_vertex-m_center)/dot(normal, normal)*normal;
            // Now check if that intersection is inside the join or not
            // If it is outside, the stroke covers the join
            if (dot(perp(m_d0), m_d1) < 0) {
                return dot(other_direction, -m_d1) > 0 ||
                    dot(other_direction, -m_d0) < 0;
            } else {
                return dot(other_direction, m_d1) < 0 ||
                    dot(other_direction, m_d0) > 0;
            }
        }

        void do_linear_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
            const R2 &p1) {
            m_offset_cusp_parameter = std::fmin(tf, m_offset_cusp_parameter);
            (void) ti; (void) tf;
            if (tangent_covers()) {
                m_w += 1;
            } else if (vertex_covers_by_normal(perp(p1-p0))) {
                m_w += 1;
            }
        }

        template <typename S, typename DS>
        bool offset_starts_retrograde(rvgf ti, const S &s, const DS &ds) const {
            auto d2s = bezier_derivative(ds);
            auto tm = rvgf{0.5}*(ti+m_offset_cusp_parameter);
            auto st = bezier_evaluate_horner(s, tm);
            auto dst = bezier_evaluate_horner(ds, tm);
            auto d2st = bezier_evaluate_horner(d2s, tm);
            auto r = radius_of_curvature(st, dst, d2st);
            return (r[1] > 0 && r[0] < r[1]*m_offset);
        }

        template <typename DS>
        bool vertex_covers(const DS &ds) const {
            return vertex_covers_by_normal(perp(project<R2>(
                bezier_evaluate_horner<rvgf>(ds, m_join_vertex_parameter))));
        }

        bool tangent_covers(void) const {
            // If the parameter where normal is tangent to circle happens
            // before an offset cusp, the stroke covers any possible join
            return m_join_tangent_parameter <=
                m_offset_cusp_parameter;
        }

        template <typename S>
        void segment_piece(rvgf ti, rvgf tf, const S &s) {
            (void) ti; (void) tf;
            m_offset_cusp_parameter = std::fmin(tf, m_offset_cusp_parameter);
            auto ds = bezier_derivative(s);
            if (!offset_starts_retrograde(ti, s, ds)) {
                if (tangent_covers()) {
                    m_w += 1;
                } else if (vertex_covers(ds)) {
                    m_w += 1;
                }
            }
        }

        void do_quadratic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
            const R2 &p1, const R2 &p2) {
            auto s = std::make_tuple(p0, p1, p2);
            segment_piece(ti, tf, s);
        }

        void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, const R3 &p0,
            const R3 &p1, const R3 &p2) {
            auto s = std::make_tuple(p0, p1, p2);
            segment_piece(ti, tf, s);
        }

        void do_cubic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
            const R2 &p1, const R2 &p2, const R2 &p3) {
            auto s = std::make_tuple(p0, p1, p2, p3);
            segment_piece(ti, tf, s);
        }

    };

    static inline path_f_covers_inner_join_or_cusp
    make_path_f_covers_inner_join_or_cusp(rvgf offset, const R2 &vertex,
        const R2 &d0, const R2 &p, const R2 &d1, rvgf &w) {
        return path_f_covers_inner_join_or_cusp{offset, vertex, d0, p, d1, w};
    }

} // namespace detail



template <typename SINK>
class decorated_path_f_simplify_joins final:
    public i_sink<decorated_path_f_simplify_joins<SINK>>,
    public i_point_regular_path<
        decorated_path_f_simplify_joins<SINK>>,
    public i_point_decorated_path<
        decorated_path_f_simplify_joins<SINK>>,
    public i_parameters_f_forwarder<path_data,
        decorated_path_f_simplify_joins<SINK>> {

    SINK m_sink;
    rvgf m_offset;
    int m_index;
    path_data m_path[3];

public:

    explicit decorated_path_f_simplify_joins(rvgf offset, SINK &&sink):
        m_sink{std::forward<SINK>(sink)},
        m_offset{offset},
        m_index{2} {
        static_assert(rvg::meta::is_an_i_regular_path<SINK>::value,
            "sink is not an i_regular_path");
        static_assert(rvg::meta::is_an_i_decorated_path<SINK>::value,
            "sink is not an i_decorated_path");
    }

    ~decorated_path_f_simplify_joins() {
        flush();
    }

    std::pair<R2, R2> get_inner_join_vertices(rvgf r,
        const R2 &d0, const R2 &p, const R2 &d1) {
        if (dot(perp(d0), d1) < 0) {
            return std::make_pair(p-r*perp(d1)/len(d1), p-r*perp(d0)/len(d0));
        } else {
            return std::make_pair(p+r*perp(d1)/len(d1), p+r*perp(d0)/len(d0));
        }
    }

private:

    path_data &current_path(void) {
        return m_path[(m_index) % 3];
    }

    const path_data &current_path(void) const {
        return m_path[(m_index) % 3];
    }

    rvgf get_join_w(const path_data &first, const R2 &d0, const R2 &p,
        const R2 &d1, const path_data &last) {
        R2 v0, v1;
        std::tie(v0, v1) = get_inner_join_vertices(m_offset, d0, p, d1);
        rvgf w = 0;
        last.iterate(
            make_regular_path_f_find_join_parameters(m_offset, v1,
                detail::make_path_f_covers_inner_join_or_cusp(m_offset, v1,
                    d0, p, d1, w)));
        first.riterate(
            make_regular_path_f_find_join_parameters(m_offset, v0,
                detail::make_path_f_covers_inner_join_or_cusp(m_offset, v0,
                    -d1, p, -d0, w)));
        return w;
    }

    auto get_join_or_cusp(const path_data &path) const {
        auto join = detail::make_path_f_get_join_or_cusp();
        path.iterate(join, std::max(0, (int) path.size()-1), (int) path.size());
        return join;
    }

    void shift(void) {
        auto &first = m_path[(m_index-2) % 3];
        auto &middle = m_path[(m_index-1) % 3];
        auto &last = m_path[(m_index) % 3];
        // if middle is a join or cusp, simplify and classify it
        auto join = get_join_or_cusp(middle);
        if (join.is_join() || join.is_cusp()) {
            middle.clear();
            R2 d0 = join.get_d0();
            R2 p = join.get_p();
            R2 d1 = join.get_d1();
            auto sin = dot(-perp(d0), d1);
            auto cos = dot(d0, d1);
            if (sin < 0.f) { // ??D change to eps
                if (join.is_join()) {
                    auto w = get_join_w(first, d0, p, d1, last);
                    middle.inner_join(d0, p, d1, w);
                } else {
                   // auto w = get_join_w(first, d0, p, d1, last);
                    // middle.inner_cusp(d0, p, d1, w);
                    middle.inner_cusp(d0, p, d1, 0);
                }
            } else if (sin > 0.f || cos < 0) { //??D change to eps
                if (join.is_join()) {
                    auto w = get_join_w(first, d0, p, d1, last);
                    middle.join(d0, p, d1, w);
                } else {
                    //auto w = get_join_w(first, d0, p, d1, last);
                    //middle.cusp(d0, p, d1, w);
                    middle.cusp(d0, p, d1, 0);
                }
            } // else join is not needed
        }
        // first is shifted to sink
        first.iterate(m_sink);
        first.clear();
        // first drops out, middle becomes first, and last becomes middle
        m_index++;
    }

    void flush(void) {
        shift();
        shift();
        shift();
    }

friend i_sink<decorated_path_f_simplify_joins<SINK>>;

    const path_data &do_sink(void) const {
        return m_path[m_index % 3];
    }

    path_data &do_sink(void) {
        return m_path[m_index % 3];
    }

friend i_point_decorated_path<decorated_path_f_simplify_joins<SINK>>;

    void do_initial_cap(const R2 &p, const R2 &d) {
        flush();
        current_path().initial_cap(p, d);
        shift();
    }

    void do_terminal_cap(const R2 &d, const R2 &p) {
        current_path().terminal_cap(d, p);
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
        flush();
        current_path().initial_butt_cap(p, d);
        shift();
    }

    void do_terminal_butt_cap(const R2 &d, const R2 &p) {
        current_path().terminal_butt_cap(d, p);
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
        current_path().join(d0, p, d1, w);
        shift();
    }

    void do_inner_join(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        (void) d0; (void) p; (void) d1; (void) w;
        assert(0);
    }


friend i_point_regular_path<decorated_path_f_simplify_joins<SINK>>;

    void do_begin_regular_contour(const R2 &pi, const R2 &di) {
        (void) di; (void) pi;
        assert(0);
    }

    void do_end_regular_open_contour(const R2 &df, const R2 &pf) {
        (void) df; (void) pf;
        assert(0);
    }

    void do_end_regular_closed_contour(const R2 &df, const R2 &pf) {
        (void) df; (void) pf;
        assert(0);
    }

    void do_degenerate_segment(const R2 &pi, const R2 &d, const R2 &pf) {
        current_path().degenerate_segment(pi, d, pf);
        shift();
    }

    void do_cusp(const R2 &di, const R2 &p, const R2 &df, rvgf w) {
        current_path().cusp(di, p, df, w);
        shift();
    }

    void do_inner_cusp(const R2 &di, const R2 &p, const R2 &df, rvgf w) {
        (void) di; (void) p; (void) df; (void) w;
        assert(0);
    }

    void do_begin_segment_piece(const R2 &pi, const R2 &di) {
        (void) pi; (void) di;
        assert(0);
    }

    void do_end_segment_piece(const R2 &di, const R2 &pi) {
        (void) pi; (void) di;
        assert(0);
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1) {
        current_path().linear_segment_piece(ti, tf, p0, p1);
        shift();
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
        const R2 &p1, const R2 &p2) {
        current_path().quadratic_segment_piece(ti, tf, p0, p1, p2);
        shift();
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, const R3 &p0,
        const R3 &p1, const R3 &p2) {
        current_path().rational_quadratic_segment_piece(ti, tf, p0,
            p1, p2);
        shift();
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
        const R2 &p1, const R2 &p2, const R2 &p3) {
        current_path().cubic_segment_piece(ti, tf, p0, p1, p2, p3);
        shift();
    }

};


template <typename SINK>
static auto
make_decorated_path_f_simplify_joins(rvgf offset, SINK &&sink) {
    return decorated_path_f_simplify_joins<SINK>{offset,
        std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
