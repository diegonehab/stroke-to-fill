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
#ifndef RVG_INPLACE_POLYNOMIAL_H
#define RVG_INPLACE_POLYNOMIAL_H

namespace rvg {

// A polynomial a_0 + a_1 x + a_2 x^2 + ... + a_N x^N
// is stored in the last N+1 elements of an array with S elements
// a_i is stored at position S-N-1+i
//
// Compute the derivative of polynomial, in place
namespace detail {
template <size_t N, size_t S, size_t I, typename T>
void poly_differentiate_helper(std::array<T, S> &coefs,
    std::index_sequence<I>) {
    coefs[S-N+I+1] *= I+2;
}

template <size_t N, size_t S, size_t I, size_t... Is, typename T, 
    typename = typename std::enable_if<(sizeof...(Is) > 0)>::type>
void poly_differentiate_helper(std::array<T, S> &coefs,
    std::index_sequence<I, Is...>) {
    coefs[S-N+I+1] *= I+2;
    poly_differentiate_helper<N>(coefs, std::index_sequence<Is...>{});
}
}

template <size_t N, size_t S, typename T,
    typename = typename std::enable_if<(N > 1)>::type>
void poly_differentiate(std::array<T, S> &coefs) {
    return detail::poly_differentiate_helper<N>(coefs,
        std::make_index_sequence<N-1>());
}

template <size_t N, size_t S, typename T,
    typename = typename std::enable_if<(N <= 1)>::type>
void poly_differentiate(std::array<T, S> &, void * = nullptr) {
    ;
}

// Compute integral of polynomial, in place
//
namespace detail {
template <size_t N, size_t S, size_t I, typename T>
void poly_integrate_helper(std::array<T, S> &coefs, std::index_sequence<I>) {
    coefs[S-N+I] /= I+2;
}

template <size_t N, size_t S, size_t I, size_t... Is, typename T,
    typename = typename std::enable_if<(sizeof...(Is) > 0)>::type>
void poly_integrate_helper(std::array<T, S> &coefs,
    std::index_sequence<I, Is...>) {
    coefs[S-N+I] /= I+2;
    poly_integrate_helper<N>(coefs, std::index_sequence<Is...>{});
}
}

template <size_t N, size_t S, typename T,
    typename = typename std::enable_if<(N >= 1)>::type>
void poly_integrate(std::array<T, S> &coefs) {
    return detail::poly_integrate_helper<N>(coefs, std::make_index_sequence<N>{});
}

template <size_t N, size_t S, typename T,
    typename = typename std::enable_if<(N <= 0)>::type>
void poly_integrate(std::array<T, S> &, void * = nullptr) {
    ;
}

// ??D implement inplace root finding

} // namespace rvg

#endif // RVG_INPLACE_POLYNOMIAL
