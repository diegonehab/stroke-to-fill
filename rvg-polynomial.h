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
#ifndef RVG_POLYNOMIAL_H
#define RVG_POLYNOMIAL_H

#include <array>
#include <boost/container/static_vector.hpp>

#include "rvg-adjacent-range.h"
#include "rvg-bisect.h"
#include "rvg-meta.h"

namespace rvg {

// Return the derivative of the polynomial
//
// A polynomial a_0 + a_1 x + a_2 x^2 + ... + a_N x^N
// is stored in an array with N+1 elements with a_i stored at position i
//
namespace detail {
template <size_t N, size_t... Is, typename T>
std::array<T, N-1> poly_derivative_helper(const std::array<T, N> &p,
    std::index_sequence<Is...>) {
    return std::array<T, N-1>{ {p[Is+1]*(Is+1)...} };
}
}

template <size_t N, typename T>
std::array<T, N-1> poly_derivative(const std::array<T, N> &p) {
    return detail::poly_derivative_helper(p, std::make_index_sequence<N-1>{});
}

// Evaluate polynomial in Horner form
//
// A polynomial a_0 + a_1 x + a_2 x^2 + ... + a_N x^N
// is stored in an array with N+1 elements with a_i stored at position i
//
template <size_t N, size_t I = 0, typename T,
    typename = typename std::enable_if<(I >= N-1)>::type>
T poly_evaluate_horner(const std::array<T, N> &p, T, void * = nullptr) {
    return p[N-1];
}

template <size_t N, size_t I = 0, typename T,
    typename = typename std::enable_if<(I < N-1)>::type>
T poly_evaluate_horner(const std::array<T, N> &p, T x) {
    return p[I]+x*poly_evaluate_horner<N,I+1>(p, x);
}

// Evaluate polynomial in Horner form using a compensated
// algorithm (i.e., using twice the precision of the underlying
// floatint-point representation)
//
// A polynomial a_0 + a_1 x + a_2 x^2 + ... + a_N x^N
// is stored in an array with N+1 elements with a_i stored at position i
//
template <size_t N, typename T>
T poly_evaluate_horner_compensated(const std::array<T, N> &p, T x) {
    T ri = p[N-1];
    T r = T(0.);
    for (int i = N-2; i >= 0; i--) {
        T pi, ppi;
        std::tie(pi, ppi) = util::two_prod(ri, x);
        T rri;
        std::tie(ri, rri) = util::two_sum(pi, p[i]);
        T qi = ppi + rri;
        r = std::fma(r, x, qi);
    }
    return ri + r;
}

// Finds all real roots of polynomial p(t) = z in the interval [a,b].
// If the polynomial has n roots r_1, r_2, ... r_n, in the interval,
// the function returns a container with values
// {a, r_1, r_2, ..., r_n, b}
// I.e., the interval endpoints are included as well.
// They surround the roots themselves.
//
template <typename T>
boost::container::static_vector<T, 3>
poly_roots(const std::array<T, 2> &p, T a, T b, T z = T(0)) {
    boost::container::static_vector<T, 3> roots;
    roots.push_back(a);
    T r = (z-p[0])/p[1];
    if (r >= a && r <= b) {
        roots.push_back(r);
    }
    roots.push_back(b);
    return roots;
}

template <size_t N, typename T,
    typename = typename std::enable_if<(N > 2)>::type>
boost::container::static_vector<T, N+1>
poly_roots(const std::array<T, N> &p, T a, T b, T z = T(0)) {
    return refine_roots([&p](T t) { return poly_evaluate_horner(p, t); },
        poly_roots(poly_derivative(p), a, b), z);
}

} // namespace rvg

#endif
