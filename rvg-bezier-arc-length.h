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
#ifndef RVG_BEZIER_ARC_LENGTH_H
#define RVG_BEZIER_ARC_LENGTH_H

#include <array>
#include <tuple>

#include "rvg-meta.h"
#include "rvg-point.h"
#include "rvg-bezier.h"
#include "rvg-arc-length.h"

namespace rvg {

// Special case for linear segments
template <typename T>
class linear_segment_arc_length {

    T m_a, m_b;
    T m_length;

public:

    linear_segment_arc_length(T a, T b, T length):
        m_a(a), m_b(b), m_length(length) { ; }

    // Optimization does nothing.
    void optimize(T a, T b, T length, int q = 15) {
        (void) q;
        m_a = a;
        m_b = b;
        m_length = length;
    }

    T get_length(void) const {
        return m_length*std::fabs(m_b-m_a);
    }

    T get_relative_parameter_for_length_fraction(T u) const {
        return u;
    }

    T get_length_fraction_for_relative_parameter(T t) const {
        return t;
    }

    T get_absolute_parameter(T t) const {
        return m_a*(T{1}-t) + m_b*t;
    }
};


template <typename T>
static inline T integral_segment_ds2(const R2 &dp) {
    return len2(dp);
}

template <typename T>
auto make_linear_segment_arc_length(T a, T b, const R2 &p0, const R2 &p1) {
    return linear_segment_arc_length<T>{a, b, len(p1-p0)};
}

template <
    typename T,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE
>
static auto make_quadratic_segment_ds2_from_tuples(BEZIER_TUPLE &&s,
    DBEZIER_TUPLE &&ds) {
    (void) s;
    return [ds = meta::make_capture(std::forward<DBEZIER_TUPLE>(ds))] (T t) -> T {
        return integral_segment_ds2<T>(bezier_evaluate_horner<T>(ds.get(), t));
    };
}

template <typename T>
static auto make_quadratic_segment_ds2(const R2 &p0, const R2 &p1,
    const R2 &p2) {
    auto s = std::make_tuple(p0, p1, p2);
    auto ds = bezier_derivative(s);
    return make_quadratic_segment_ds2_from_tuples<T>(std::move(s),
        std::move(ds));
}

template <typename T>
static T rational_segment_ds2(const R3 &p, const R3 &dp) {
    auto n = nrd(p, dp);
    auto w2 = p[2]*p[2];
    return T(len2(n)/(w2*w2));
}

// In the case of the rational quadratic, we unfortunately need to
// deal with its derivatives, and not simply tangent directions
//
// Let
//
//   a(t) = (u(t), v(t), w(t))
//
//       and
//
//   c(t) = (u(t)/w(t), v(t)/w(t))
//
// Then,
//
//   c' = nrd(a, a')/w^2
//
// where
//
//    nrd((x, y, w), (x', y', w')) = (w x' - x w', w y' - y w')
//
template <
    typename T,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE
>
static auto make_rational_quadratic_segment_ds2_from_tuples(BEZIER_TUPLE &&s,
    DBEZIER_TUPLE &&ds) {
    return [s = meta::make_capture(std::forward<BEZIER_TUPLE>(s)),
            ds = meta::make_capture(std::forward<DBEZIER_TUPLE>(ds))](T t) {
        return rational_segment_ds2<T>(
            bezier_evaluate_horner<T>(s.get(), t),
            bezier_evaluate_horner<T>(ds.get(), t)
        );
    };
}

template <typename T>
static auto make_rational_quadratic_segment_ds2(const R3 &p0, const R3 &p1,
    const R3 &p2) {
    auto s = std::make_tuple(p0, p1, p2);
    auto ds = bezier_derivative(s);
    return make_rational_quadratic_segment_ds2_from_tuples<T>(std::move(s),
        std::move(ds));
}

template <
    typename T,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE
>
static auto make_cubic_segment_ds2_from_tuples(BEZIER_TUPLE &&s,
    DBEZIER_TUPLE &&ds) {
    (void) s;
    return [ds = meta::make_capture(std::forward<DBEZIER_TUPLE>(ds))](T t) -> T {
        return integral_segment_ds2<T>(bezier_evaluate_horner<T>(ds.get(), t));
    };
}

template <typename T>
static auto make_cubic_segment_ds2(const R2 &p0, const R2 &p1,
    const R2 &p2, const R2 &p3) {
    auto s = std::make_tuple(p0, p1, p2, p3);
    auto ds = bezier_derivative(s);
    return make_cubic_segment_ds2_from_tuples<T>(std::move(s), std::move(ds));
}

template <typename T>
static T integral_offset_ds2(T w, const R2 &p, const R2 &dp, const R2 &d2p) {
    T k = project<T>(inverse(radius_of_curvature(p, dp, d2p)));
    T v = (1-w*k);
    return T(len2(dp)*v*v);
}

// Given a curve c(t), the offset curve o(t) at distance w is given by
//
//     o = c + w N
//
// where
//
//      N(t) = perp(c')/len(c')
//
// is the normal to the curve
//
// The derivative of the offset curve is
//
//     o' = (1 - w k) c'
//
// where
//
//     k(t) = dot(perp(c'),c'') / |c'|^3
//
// is the curvature of the curve
//
// So the norm squared of the derivative is
//
//    ds2 = (1 - w k)^2 |c'|^2
//
template <
    typename T,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE,
    typename DDBEZIER_TUPLE
>
static auto make_quadratic_offset_ds2_from_tuples(T w, BEZIER_TUPLE &&s,
    DBEZIER_TUPLE &&ds, DDBEZIER_TUPLE &&dds) {
    (void) s;
    return [ds = meta::make_capture(std::forward<DBEZIER_TUPLE>(ds)),
            dds = meta::make_capture(std::forward<DDBEZIER_TUPLE>(dds)),
            w](T t) -> T {
        R2 st{};
        auto dst = bezier_evaluate_horner<T>(ds.get(), t);
        auto ddst = bezier_evaluate_horner<T>(dds.get(), t);
        return integral_offset_ds2<T>(w, st, dst, ddst);
    };
}

template <typename T>
static auto make_quadratic_offset_ds2(T w, const R2 &p0, const R2 &p1,
    const R2 &p2) {
    auto s = std::make_tuple(p0, p1, p2);
    auto ds = bezier_derivative(s);
    auto dds = bezier_derivative(ds);
    return make_quadratic_offset_ds2_from_tuples<T>(w, std::move(s),
        std::move(ds), std::move(dds));
}

template <typename T>
static T rational_offset_ds2(T w, const R3 &p, const R3 &dp, const R3 &d2p) {
    auto pdst = perp(nrd(p, dp));
    auto pddst = nrd(p, d2p);
    auto w2 = p[2]*p[2];
    auto ldst2 = len2(pdst);
    T k =  w2*dot(pdst, pddst)/(ldst2*std::sqrt(ldst2));
    T v = (1-w*k);
    return T(len2(pdst)*v*v/(w2*w2));
}

// In the case of the rational quadratic, we unfortunately need to
// deal with its derivatives, and not simply tangent directions
//
// Let
//
//   a(t) = (u(t), v(t), w(t))
//
//       and
//
//   c(t) = (u(t)/w(t), v(t)/w(t))
//
// Then,
//
//   c' = nrd(a, a')/w^2
//
//   and
//
//   c'' = nrd(a, a'')/w^2 - 2 w' c'/ w
//
// where
//
//    nrd((x, y, w), (x', y', w')) = (w x' - x w', w y' - y w')
//
//
template <
    typename T,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE,
    typename DDBEZIER_TUPLE
>
static auto make_rational_quadratic_offset_ds2_from_tuples(T w,
    BEZIER_TUPLE &&s, DBEZIER_TUPLE &&ds, DDBEZIER_TUPLE &&dds) {
    return [s = meta::make_capture(std::forward<BEZIER_TUPLE>(s)),
            ds = meta::make_capture(std::forward<DBEZIER_TUPLE>(ds)),
            dds = meta::make_capture(std::forward<DDBEZIER_TUPLE>(dds)),
            w](T t) -> T {
        auto st = bezier_evaluate_horner<T>(s.get(), t);
        auto dst = bezier_evaluate_horner<T>(ds.get(), t);
        auto ddst = bezier_evaluate_horner<T>(dds.get(), t);
        return rational_offset_ds2<T>(w, st, dst, ddst);
    };
}

template <typename T>
static auto make_rational_quadratic_offset_ds2(T w, const R3 &p0,
    const R3 &p1, const R3 &p2) {
    auto s = std::make_tuple(p0, p1, p2);
    auto ds = bezier_derivative(s);
    auto dds = bezier_derivative(ds);
    return make_rational_quadratic_offset_ds2_from_tuples<T>(w, std::move(s),
        std::move(ds), std::move(dds));
}

template <
    typename T,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE,
    typename DDBEZIER_TUPLE
>
static auto make_cubic_offset_ds2_from_tuples(T w, BEZIER_TUPLE &&s,
    DBEZIER_TUPLE &&ds, DDBEZIER_TUPLE &&dds) {
    (void) s;
    return [s = meta::make_capture(std::forward<BEZIER_TUPLE>(s)),
            ds = meta::make_capture(std::forward<DBEZIER_TUPLE>(ds)),
            dds = meta::make_capture(std::forward<DDBEZIER_TUPLE>(dds)),
            w](T t) -> T {
        R2 st{};
        auto dst = bezier_evaluate_horner<T>(ds.get(), t);
        auto ddst = bezier_evaluate_horner<T>(dds.get(), t);
        return integral_offset_ds2<T>(w, st, dst, ddst);
    };
}

template <typename T>
static auto make_cubic_offset_ds2(T w, const R2 &p0, const R2 &p1,
    const R2 &p2, const R2 &p3) {
    auto s = std::make_tuple(p0, p1, p2, p3);
    auto ds = bezier_derivative(s);
    auto dds = bezier_derivative(ds);
    return make_cubic_offset_ds2_from_tuples<T>(w, std::move(s), std::move(ds),
        std::move(dds));
}

// Given a curve c(t), the evolute e(t) is given by
//
//     e = c + N/k
//
// where
//
//      N = perp(c')/|c'|
//
// is the normal to the curve and
//
//     k(t) = dot(perp(c'),c'') / |c'|^3
//
// is the curvature of the curve
//
// Its derivative is
//
//     e' = - k' / k^2 N
//
// Here,
//
//     k'(t)  = dot(perp(c'), (c''' dot(c',c') - 3 c'' dot(c',c''))) / |c'|^5
//
// is the derivative of the curvature
//
// So the norm squared of the derivative is
//
//    ds2 = (k')^2/k^4
//
//        = |c'|^2 dot(perp(c'), (c''' dot(c',c') - 3 c'' dot(c',c'')))^2 /
//                                  dot(perp(c'),c'')^4
//
// In the case of the integral quadratics, c''' is zero, so the
// expression simplifies to
//
//    ds2 = (k')^2/k^4
//
//        = |c'|^2 dot(perp(c'), (3 c'' dot(c',c'')))^2 /
//                        dot(perp(c'),c'')^4
//
template <
    typename T,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE,
    typename DDBEZIER_TUPLE
>
static auto make_quadratic_evolute_ds2_from_tuples(T max_radius,
    BEZIER_TUPLE &&s, DBEZIER_TUPLE &&ds, DDBEZIER_TUPLE &&dds) {
    (void) s;
    (void) max_radius;
    return [ds = meta::make_capture(std::forward<DBEZIER_TUPLE>(ds)),
            dds = meta::make_capture(std::forward<DDBEZIER_TUPLE>(dds))](T t)
        -> T {
        auto sq = [](T v) -> T { return v*v; };
        auto sq2 = [](T v) -> T { return (v*v)*(v*v); };
        R2 st{};
        auto dst = bezier_evaluate_horner<T>(ds.get(), t);
        auto ddst = bezier_evaluate_horner<T>(dds.get(), t);
        return T(len2(dst) *
            sq(dot(perp(dst),3*ddst*dot(dst, ddst))) /
                    sq2(dot(perp(dst), ddst)));
    };
}

template <typename T>
static auto make_quadratic_evolute_ds2(T max_radius, const R2 &p0, const R2 &p1,
    const R2 &p2) {
    auto s = std::make_tuple(p0, p1, p2);
    auto ds = bezier_derivative(s);
    auto dds = bezier_derivative(ds);
    return make_quadratic_evolute_ds2_from_tuples<T>(max_radius, std::move(s),
        std::move(ds), std::move(dds));
}

// In the case of the rational quadratic, we unfortunately need to
// deal with its derivatives, and not simply tangent directions
//
// Let
//
//    a(t) = (u(t), v(t), w(t))
//
//       and
//
//    c(t) = (u(t)/w(t), v(t)/w(t))
//
// Then,
//
//    c' = nrd(a, a')/w^2
//
//    c'' = nrd(a, a'')/w^2 - 2 w' c'/ w
//
//    and
//
//    c''' = (nrd(a, a''') + nrd(a', a''))/w^2 - 4 w' c'' / w
//         - 2 (w'^2 + w'' w) c' / w^2
//
//  In the case of quadratics, a''' vanishes, so the
//  expression becomes
//
//     c''' = nrd(a', a'')/w^2 - 4 w' c'' / w  - 2 (w'^2 + w'' w) c' / w^2
//
//   And
//
//     ds2 = (k')^2/k^4
//
//         = |c'|^2 dot(perp(c'), (c''' dot(c',c') - 3 c'' dot(c',c'')))^2 /
//                                  dot(perp(c'),c'')^4
template <
    typename T,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE,
    typename DDBEZIER_TUPLE
>
static auto make_rational_quadratic_evolute_ds2_from_tuples(
    T max_radius,
    BEZIER_TUPLE &&s,
    DBEZIER_TUPLE &&ds,
    DDBEZIER_TUPLE &&dds
) {
    (void) max_radius;
    return [s = meta::make_capture(std::forward<BEZIER_TUPLE>(s)),
            ds = meta::make_capture(std::forward<DBEZIER_TUPLE>(ds)),
            dds = meta::make_capture(std::forward<DDBEZIER_TUPLE>(dds))](T t)
        -> T {
        auto sq = [](T v) -> T { return v*v; };
        auto sq2 = [](T v) -> T { return (v*v)*(v*v); };
        auto st = bezier_evaluate_horner<T>(s.get(), t);
        auto dst = bezier_evaluate_horner<T>(ds.get(), t);
        auto ddst = bezier_evaluate_horner<T>(dds.get(), t);
        auto w = st[2];
        auto w2 = w*w;
        auto dc = nrd(st, dst)/w2;
        auto dw = dst[2];
        auto ddc = nrd(st, ddst)/w2 - (2*dw)*dc/w;
        auto ddw = ddst[2];
        auto d3c = (nrd(dst, ddst) - 2*(dw*dw + ddw*w)*dc)/w2 - (4*dw)*ddc/w;
        return T(len2(dc) *
            sq(dot(perp(dc),d3c*dot(dc,dc) - 3*ddc*dot(dc, ddc))) /
                    sq2(dot(perp(dc), ddc)));
    };
}

template <typename T>
static auto make_rational_quadratic_evolute_ds2(T max_radius, const R3 &p0,
    const R3 &p1, const R3 &p2) {
    auto s = std::make_tuple(p0, p1, p2);
    auto ds = bezier_derivative(s);
    auto dds = bezier_derivative(ds);
    return make_rational_quadratic_evolute_ds2_from_tuples<T>(max_radius,
        std::move(s), std::move(ds), std::move(dds));
}

template <
    typename T,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE,
    typename DDBEZIER_TUPLE,
    typename D3BEZIER_TUPLE
>
static auto make_cubic_evolute_ds2_from_tuples(T max_radius,
    BEZIER_TUPLE &&s, DBEZIER_TUPLE &&ds,
    DDBEZIER_TUPLE &&dds, D3BEZIER_TUPLE &&d3s) {
    return [max_radius,
              s  = meta::make_capture(std::forward<BEZIER_TUPLE>(s)),
             ds  = meta::make_capture(std::forward<DBEZIER_TUPLE>(ds)),
            dds  = meta::make_capture(std::forward<DDBEZIER_TUPLE>(dds)),
            d3s  = meta::make_capture(std::forward<D3BEZIER_TUPLE>(d3s))](T t)
        -> T {
        auto sq = [](T v) -> T { return v*v; };
        auto sq2 = [](T v) -> T { return (v*v)*(v*v); };
        auto st = bezier_evaluate_horner<T>(s.get(), t);
        auto dst = bezier_evaluate_horner<T>(ds.get(), t);
        auto ddst = bezier_evaluate_horner<T>(dds.get(), t);
        auto d3st = bezier_evaluate_horner<T>(d3s.get(), t);
        auto r = bounded_radius_of_curvature(max_radius, st, dst, ddst);
        if (r < max_radius) {
            return T(len2(dst) *
                sq(dot(perp(dst), d3st*dot(dst,dst) - 3*ddst*dot(dst, ddst))) /
                        sq2(dot(perp(dst), ddst)));
        } else {
            auto ods2 = make_cubic_offset_ds2_from_tuples(
                max_radius, s.get(), ds.get(), dds.get());
            return ods2(t);
        }
    };
}

template <typename T>
static auto make_cubic_evolute_ds2(T max_radius, const R2 &p0, const R2 &p1,
    const R2 &p2, const R2 &p3) {
    auto s = std::make_tuple(p0, p1, p2, p3);
    auto ds = bezier_derivative(s);
    auto dds = bezier_derivative(ds);
    auto d3s = bezier_derivative(dds);
    return make_cubic_evolute_ds2_from_tuples<T>(max_radius, std::move(s),
        std::move(ds), std::move(dds), std::move(d3s));
}

} // namespace rvg

#endif
