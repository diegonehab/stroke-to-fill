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
#ifndef RVG_BEZIER_H
#define RVG_BEZIER_H

#include <tuple>
#include <algorithm>
#include <array>
#include <iostream>
#include <boost/container/static_vector.hpp>

#include "rvg-util.h"
#include "rvg-tuple.h"
#include "rvg-meta.h"
#include "rvg-bisect.h"

namespace rvg {

using boost::container::static_vector;

// Compute the derivative of a Bezier segment s(t) using N-1 muls and adds,
// where N is the degree of the curve segment
//
// Let S[i] be the ith control point, i = 0..N.  Then,
//
// S'[i] = N*(S[i+1]-S[i])
//
namespace detail {
    template <typename BEZIER_TUPLE, size_t... Is>
    auto bezier_derivative_helper(const BEZIER_TUPLE &B,
        std::index_sequence<Is...>) {
        const unsigned short DEGREE = static_cast<unsigned short>(std::tuple_size<BEZIER_TUPLE>::value-1);
        return std::make_tuple(DEGREE*(std::get<Is+1>(B)-std::get<Is>(B))...);
    }
}

template <typename BEZIER_TUPLE>
auto bezier_derivative(const BEZIER_TUPLE &B) {
    const auto DEGREE = std::tuple_size<BEZIER_TUPLE>::value-1;
    return detail::bezier_derivative_helper(
        B, std::make_index_sequence<DEGREE>{});
}

// Same as derivative, but do not multiply by N
namespace detail {
    template <typename BEZIER_TUPLE, size_t... Is>
    auto bezier_differences_helper(const BEZIER_TUPLE &B,
        std::index_sequence<Is...>) {
        return std::make_tuple((std::get<Is+1>(B)-std::get<Is>(B))...);
    }
}

template <typename BEZIER_TUPLE>
auto bezier_differences(const BEZIER_TUPLE &B) {
    const auto DEGREE = std::tuple_size<BEZIER_TUPLE>::value-1;
    return detail::bezier_differences_helper(
        B, std::make_index_sequence<DEGREE>{});
}

// Evaluate Bezier segment s(t) by Horner's algorithm,
// using about 4N muls and N adds,
// where N is the degree of the curve segment
//
// Let S[i] be the ith control point, i = 0..N. Let u = 1-t.
// The idea is to compute
// s(t) = (...(S[0] choose(N,0) u + S[1] choose(N,1) t) u +
//   S[2] choose(N,2) t^2) u + S[3] choose(N, 3) t^3) u...) u
//      + S[N] choose(N,N) t^N
//
// We will incrementally compute values
//    p_k, for k = 0..N, so that in the end s(t) = p_N
//    c_k, for k = 0..N, so that c_k = choose(N, k)
//    t_k, for k = 0..N, so that t_k = t^k
//
// The basic case is
// p_0 = S[0];
// t_0 = 1.f;
// c_0 = 1;
//
// The iteration is
// t_k = t_{k-1}*t,
// c_k = (c_{k-1}*(N-(k-1)))/k, and
// p_k = p_{k-1}*u + S[k]*c_k*t_k
//
// We want the compiler to expand all code in-line for
// us, without the need for any loop. So we first rewrite
// the iteration as a recursive function
//
// f(p_{k-1}, k, N, c_k, t_k, t, u)
//
// If k > N, we simply return p_{k-1}. Otherwise, we recursively
// invoke the function with p_k and other updated arguments
//
// f(p_{k-1}, k, N, c_k, t_k, t, u) :=
//   k > N ? p_{k-1}:
//     f(p_{k-1}*u + S[k]*c_k*t_k, k+1, N, (c_k*(N-k))/(k+1), t_k*t, t, u)
//
// To obtain s(t), we invoke
//
// f(p_0 = S[0], k = 1, N, t_1 = t, c_1 = N, t, u = 1-t)
//
// This helper template matches the end of the recursion, where we
// simply return p_k
//
namespace detail {
    template <typename T, size_t K, size_t CK, size_t DEGREE,
        typename BEZIER_TUPLE, typename POINT,
        typename = typename std::enable_if<(K > DEGREE)>::type>
    POINT bezier_evaluate_horner_helper(const BEZIER_TUPLE &, const POINT &pk1,
        T, T, T, void * = nullptr) {
        return pk1;
    }
}

//
// This helper template instantiates the next
// step in the recursive computation
//
namespace detail {
    template <typename T, size_t K, size_t CK, size_t DEGREE,
        typename BEZIER_TUPLE, typename POINT,
        typename = typename std::enable_if<(K <= DEGREE)>::type>
    auto bezier_evaluate_horner_helper(const BEZIER_TUPLE &B, const POINT &pk1,
        T tk, T t, T u) {
        return detail::bezier_evaluate_horner_helper<T, K+1,
            (CK*(DEGREE-K))/(K+1), DEGREE>(B, pk1*u + std::get<K>(B)*T{CK}*tk,
            tk*t, t, u);
    }
}

//
// This template simply calls the helper template with initial conditions
//
template <typename T, typename BEZIER_TUPLE>
auto bezier_evaluate_horner(const BEZIER_TUPLE& B, T t, T u) {
    const auto DEGREE = std::tuple_size<BEZIER_TUPLE>::value-1;
    return detail::bezier_evaluate_horner_helper<T, 1, DEGREE, DEGREE>(
        B, std::get<0>(B), t, t, u);
}

template <typename T, typename BEZIER_TUPLE>
auto bezier_evaluate_horner(const BEZIER_TUPLE &B, T t) {
    const auto DEGREE = std::tuple_size<BEZIER_TUPLE>::value-1;
    return detail::bezier_evaluate_horner_helper<T, 1, DEGREE, DEGREE>(
        B, std::get<0>(B), t, t, T{1.}-t);
}

// Evaluate one step of the blossom for a Bezier curve at t.
// Let S[i] be the ith control point, i = 0..N. Let u = 1-t.
// We simply compute the control points R[j], j=0..N-1
// such that R[j] = S[j]*u + S[j+1]*t
//
namespace detail {
    template <typename T, typename BEZIER_TUPLE, size_t... Is>
    auto bezier_blossom_helper(const BEZIER_TUPLE &B,
        T t, T u, std::index_sequence<Is...>) {
        return std::make_tuple((u*std::get<Is>(B)+t*std::get<Is+1>(B))...);
    }
}

template <typename T, typename BEZIER_TUPLE>
auto bezier_blossom(const BEZIER_TUPLE &B, T t, T u) {
    const auto DEGREE = std::tuple_size<BEZIER_TUPLE>::value-1;
    return detail::bezier_blossom_helper<T,BEZIER_TUPLE>(B, t, u,
        std::make_index_sequence<DEGREE>{});
}

template <typename T, typename BEZIER_TUPLE>
auto bezier_blossom(const BEZIER_TUPLE &B, T t) {
    const auto DEGREE = std::tuple_size<BEZIER_TUPLE>::value-1;
    return detail::bezier_blossom_helper<T, BEZIER_TUPLE>(B, t, T{1}-t,
        std::make_index_sequence<DEGREE>{});
}

namespace detail {
    template <typename T, typename BEZIER_TUPLE,
        typename = typename std::enable_if<
            (std::tuple_size<BEZIER_TUPLE>::value <= 1)
        >::type
    >
    auto bezier_evaluate_decasteljau_helper(const BEZIER_TUPLE &B, T t, T u,
        void * = nullptr) {
        (void) t; (void) u;
        return std::get<0>(B);
    }

    template <typename T, typename BEZIER_TUPLE,
        typename = typename std::enable_if<
            (std::tuple_size<BEZIER_TUPLE>::value > 1)
        >::type
    >
    auto bezier_evaluate_decasteljau_helper(const BEZIER_TUPLE &B, T t, T u) {
        return bezier_evaluate_decasteljau_helper(
            bezier_blossom(B, t, u), t, u);
    }
}

//
// Evaluate Bezier segment s(t) by De Casteljau's algorithm
// using about N*(N-1) muls and N*(N-1)/2 adds,
// where N is the degree of the curve segment
//
template <typename T, typename BEZIER_TUPLE>
auto bezier_evaluate_decasteljau(const BEZIER_TUPLE& B, T t, T u) {
    return detail::bezier_evaluate_decasteljau_helper<T>(B, t, u);
}

template <typename T, typename BEZIER_TUPLE>
auto bezier_evaluate_decasteljau(const BEZIER_TUPLE &B, T t) {
    return bezier_evaluate_decasteljau<T>(B, t, T{1.}-t);
}

// Subdivide a Bezier segment s(u) at u = t, using about
// (N-1)(N-2) muls and (N-1)(N-2)/2 adds
// where N is the degree of the curve segment
//
// Let S[i] be the ith control point, i = 0..N.
// Let S1 and S2 be the results of subdivision
//
// If S(u, u, u,..., u) = s(u) is the polar form of the segment,
// we know that
//
// S1[0] = S(0, 0, 0, ..., 0, 0, 0)
// S1[1] = S(t, 0, 0, ..., 0, 0, 0)
// S1[2] = S(t, t, 0, ..., 0, 0, 0)
// ...
// S1[N] = S(t, t, t, ..., t, t, t)
//
// S2[0] = S(t, t, t, ..., t, t, t)
// S2[1] = S(t, t, t, ..., t, t, 1)
// S2[2] = S(t, t, t, ..., t, 1, 1)
// ...
// S2[N] = S(1, 1, 1, ..., 1, 1, 1)
//
// So right away we know S1[0] = S[0] and S2[N] = S[N].
// We don't even bother returning them.
// We then perform a blossom S = blossom(S, t);
// This gives us S1[1] = S[0] and S2[N-1] = S[N-1]
// We then perform another blossom S = blossom(S, t);
// This gives us S1[2] = S[0] and S2[N-2] = S[N-2]
// And so on.
// At some point we reach a single point, whereupon
// This gives us S1[N] = S2[0] = S[0]
// We return a tuple with only the internal 2N-1 control points //
//   S1[1], S1[2], ... S1[N] = S2[0], ... S2[N-1]
//
// omitting S1[0] and S2[N], as well as using a single point
// to represent the common S1[N] = S2[0]
//
// This template helper matches the end of the recursion
//
namespace detail {
    template <typename T, typename BEZIER_TUPLE,
        typename = typename std::enable_if<
            (std::tuple_size<BEZIER_TUPLE>::value <= 1)>::type>
    auto bezier_split_helper(const BEZIER_TUPLE &B, T, T, void* = nullptr) {
        return B;
    }
}

//
// This template helper concatenates S1[K], the recursive
// invokation in the blossom, and S2[N-k].
//
namespace detail {
    template <typename T, typename BEZIER_TUPLE,
        typename = typename std::enable_if<
        (std::tuple_size<BEZIER_TUPLE>::value > 1)>::type>
    auto bezier_split_helper(const BEZIER_TUPLE &B, T t, T u) {
        const auto DEGREE = std::tuple_size<BEZIER_TUPLE>::value-1;
        return std::tuple_cat(
            std::make_tuple(std::get<0>(B)),
            detail::bezier_split_helper(bezier_blossom(B, t, u), t, u),
            std::make_tuple(std::get<DEGREE>(B))
        );
    }
}

template <typename T, typename BEZIER_TUPLE>
auto bezier_split(const BEZIER_TUPLE &B, T t, T u) {
    return detail::bezier_split_helper(bezier_blossom(B, t, u), t, u);
}

template <typename T, typename BEZIER_TUPLE>
auto bezier_split(const BEZIER_TUPLE &B, T t) {
    return bezier_split(B, t, T{1}-t);
}

// Cut a segment (or extend it) from [0,1] to [a,b]
// Works in a similar way to split
// We return a tuple with all control points except the first.
// This is because this function is usually used within a
// loop to partition a segment into connected pieces.
// Therefore, the first control point is the last control
// point of the previous piece.
// If needed, the first control point is simply the result
// of evaluating the segment at a.
namespace detail {
    template <typename T, typename BEZIER_TUPLE,
        typename = typename std::enable_if<
            (std::tuple_size<BEZIER_TUPLE>::value <= 1)>::type>
    auto bezier_cut_helper(const BEZIER_TUPLE &B, T, T, T, T,
        void * = nullptr) {
        return B;
    }

    template <typename T, typename BEZIER_TUPLE,
         typename = typename std::enable_if<
            (std::tuple_size<BEZIER_TUPLE>::value > 1)>::type>
    auto bezier_cut_helper(const BEZIER_TUPLE &B, T a, T a1, T b, T b1) {
        return std::tuple_cat(
            std::make_tuple(bezier_evaluate_horner(B, a, a1)),
            bezier_cut_helper(bezier_blossom(B, b, b1), a, a1, b, b1));
    }
}

template <typename T, typename BEZIER_TUPLE>
auto bezier_cut(const BEZIER_TUPLE &B, T a, T b) {
    T a1 = T{1}-a;
    T b1 = T{1}-b;
    return detail::bezier_cut_helper(bezier_blossom(B, b, b1), a, a1, b, b1);
}

// Same as cut, but for a prefix [0,t]
//
namespace detail {
    template <typename T, typename BEZIER_TUPLE,
        typename = typename std::enable_if<
            (std::tuple_size<BEZIER_TUPLE>::value <= 1)>::type>
    auto bezier_prefix_helper(const BEZIER_TUPLE &B, T, T, void * = nullptr) {
        return B;
    }

    template <typename T, typename BEZIER_TUPLE,
        typename = typename std::enable_if<
            (std::tuple_size<BEZIER_TUPLE>::value > 1)>::type>
    auto bezier_prefix_helper(const BEZIER_TUPLE &B, T t, T t1) {
        return std::tuple_cat(
            std::make_tuple(bezier_evaluate_horner(B, t, t1)),
            bezier_prefix_helper(tuple_drop<-1>(B), t, t1));
    }
}

template <typename T, typename BEZIER_TUPLE>
auto bezier_prefix(const BEZIER_TUPLE &B, T t) {
    T t1 = T{1}-t;
    return detail::bezier_prefix_helper(tuple_drop<-1>(B), t, t1);
}

// Same as cut, but for a suffix [t,1]
//
namespace detail {
    template <typename T, typename BEZIER_TUPLE,
        typename = typename std::enable_if<
            (std::tuple_size<BEZIER_TUPLE>::value <= 1)>::type>
    auto bezier_suffix_helper(const BEZIER_TUPLE &B, T, T, void * = nullptr) {
        return B;
    }

    template <typename T, typename BEZIER_TUPLE,
        typename = typename std::enable_if<
            (std::tuple_size<BEZIER_TUPLE>::value > 1)>::type>
    auto bezier_suffix_helper(const BEZIER_TUPLE &B, T t, T t1) {
        return std::tuple_cat(
            std::make_tuple(bezier_evaluate_horner(B, t, t1)),
            bezier_suffix_helper(tuple_drop<1>(B), t, t1));
    }
}

template <typename T, typename BEZIER_TUPLE>
auto bezier_suffix(const BEZIER_TUPLE &B, T t) {
    T t1 = T{1}-t;
    return detail::bezier_suffix_helper(tuple_drop<1>(B), t, t1);
}


// Elevate the degree of a Bezier curve
//
// Let S[i] be the ith control point of the input, i = 0..N.
// Let R[i] be the ith control point of the output, i = 0..N+1.
// Then, we have that R[0] = S[0] and R[N+1] = S[N];
// Additionally,
// R[i] = i/(N+1) S[i-1] + (N+1-i)/(N+1) S[i], i = 1..N
//

namespace detail {
    template <typename T, typename BEZIER_TUPLE, size_t... Is>
    auto bezier_elevate_degree_helper(const BEZIER_TUPLE &B,
        std::index_sequence<Is...>) {
        const auto DEGREE = std::tuple_size<BEZIER_TUPLE>::value-1;
        return std::make_tuple(
            std::get<0>(B),
            (
              (T{Is+1}/T{DEGREE+1})*std::get<Is>(B) +
              (T{DEGREE-Is}/T{DEGREE+1})*std::get<Is+1>(B)
            )...,
            std::get<DEGREE>(B)
        );
    }
}

template <typename T, typename BEZIER_TUPLE>
auto bezier_elevate_degree(const BEZIER_TUPLE &B) {
    const auto DEGREE = std::tuple_size<BEZIER_TUPLE>::value-1;
    return detail::bezier_elevate_degree_helper<T>(B,
        std::make_index_sequence<DEGREE>{});
}

// Lower the degree of a Bezier curve, ASSUMING it is indeed
// of lower degree
//
// Let S[i] be the ith control point of the input, i = 0..N.
// Let R[i] be the ith control point of the output, i = 0..N-1.
// Then, we have that R[0] = S[0] and R[N-1] = S[N];
// Additionally,
// R[i] = (N S[i] - i R[i-1]) / (N-i),   i = 1..N-2
//

namespace detail {

    template <size_t I, typename BEZIER_TUPLE, typename P,
        typename = typename std::enable_if<
            (I >= std::tuple_size<BEZIER_TUPLE>::value-2)>::type>
    auto bezier_lower_degree_helper(const BEZIER_TUPLE &B, const P &,
        void * = nullptr) {
        const auto DEGREE = std::tuple_size<BEZIER_TUPLE>::value-1;
        return std::make_tuple(std::get<DEGREE>(B));
    }

    template <size_t I, typename BEZIER_TUPLE, typename P,
        typename = typename std::enable_if<
            (I < std::tuple_size<BEZIER_TUPLE>::value-2)>::type>
    auto bezier_lower_degree_helper(const BEZIER_TUPLE &B, const P &p) {
        const auto DEGREE = std::tuple_size<BEZIER_TUPLE>::value-1;
        auto q = (DEGREE*std::get<I>(B)-I*p)/(DEGREE-I);
        return std::tuple_cat(
            std::make_tuple(q),
            bezier_lower_degree_helper<I+1>(B, q)
        );
    }
}

template <typename BEZIER_TUPLE,
    typename = typename std::enable_if<
        (std::tuple_size<BEZIER_TUPLE>::value > 2)>::type>
auto bezier_lower_degree(const BEZIER_TUPLE &B) {
    return std::tuple_cat(
        std::make_tuple(std::get<0>(B)),
        detail::bezier_lower_degree_helper<1>(B, std::get<0>(B))
    );
}

template <typename BEZIER_TUPLE,
    typename = typename std::enable_if<
        (std::tuple_size<BEZIER_TUPLE>::value == 2)>::type>
auto bezier_lower_degree(const BEZIER_TUPLE &B, void * = nullptr) {
    return std::make_tuple(std::get<0>(B));
}

// Compute the product between two Bezier curves
//
// Let P[i] be the ith control point of one of the curves, i = 0..M.
// Let Q[j] be the ith control point of the other curve, j = 0..N.
// Let R[k] be the ith control point of the product curve,  k = 0..M+N.
// Then, the output
// R[k] = sum_{i=max(0,k-N)}^{min(M,k)}
//      P[i]Q[k-i] choose(M,i) choose(N,k-i) / choose(M+N,k)
//

namespace detail {

    template <typename T>
    auto bezier_product_sum_args_helper(T t) {
        return t;
    }

    template <typename T, typename ... Ts>
    auto bezier_product_sum_args_helper(T t, Ts ... ts) {
        return t + bezier_product_sum_args_helper(ts...);
    }

    template <typename T, size_t M, size_t N, size_t K,
        typename P_BEZIER_TUPLE, typename Q_BEZIER_TUPLE, size_t... Is>
    auto bezier_product_sum_helper(const P_BEZIER_TUPLE &P,
        const Q_BEZIER_TUPLE &Q, std::index_sequence<Is...>) {
        constexpr const size_t O = std::max((int)0, (int)(K-N));
        return bezier_product_sum_args_helper(
            (std::get<O+Is>(P) * std::get<K-Is-O>(Q)* T{util::choose(M,O+Is)} *
                T{util::choose(N,K-Is-O)})...
        );
    }

    template <typename T, size_t M, size_t N, typename P_BEZIER_TUPLE,
        typename Q_BEZIER_TUPLE, size_t... Ks>
    auto bezier_product_helper(const P_BEZIER_TUPLE &P,
        const Q_BEZIER_TUPLE &Q, std::index_sequence<Ks...>) {
        return std::make_tuple(
            T{1}/T{util::choose(M+N, Ks)} *
            bezier_product_sum_helper<T, M, N, Ks>(P, Q,
                std::make_index_sequence<
                    std::min((int)M,(int)Ks)-std::max(0,(int)(Ks-N))+1>{})...
        );
    }
}

template <typename T, typename P_BEZIER_TUPLE, typename Q_BEZIER_TUPLE>
auto bezier_product(const P_BEZIER_TUPLE &P, const Q_BEZIER_TUPLE &Q) {
    constexpr size_t M = std::tuple_size<P_BEZIER_TUPLE>::value-1;
    constexpr size_t N = std::tuple_size<Q_BEZIER_TUPLE>::value-1;
    return detail::bezier_product_helper<T,M,N>(P, Q,
        std::make_index_sequence<M+N+1>{});
}

namespace detail {

template <typename T, typename BEZIER_TUPLE,
    typename = typename std::enable_if<
        (std::tuple_size<BEZIER_TUPLE>::value == 1)
    >::type
>
static auto minmax(const BEZIER_TUPLE &B, void * = nullptr) {
    const T t = static_cast<T>(std::get<0>(B));
    return std::make_pair(t, t);
}

template <typename T, typename BEZIER_TUPLE,
    typename = typename std::enable_if<
        (std::tuple_size<BEZIER_TUPLE>::value > 1)>::type>
static auto minmax(const BEZIER_TUPLE &B) {
    const T t = static_cast<T>(std::get<0>(B));
    auto m = minmax<T>(tuple_drop<1>(B));
    return std::make_pair(
        std::min(t, std::get<0>(m)),
        std::max(t, std::get<1>(m))
    );
}

}

// Finds all real roots of the Bezier segment s(t) = z for t in [a,b].
// If the Bezier has n roots r_1, r_2, ... r_n, in the interval,
// the function returns a container with values
// {a, r_1, r_2, ..., r_n, b}
// I.e., the interval endpoints are included as well.
// They surround the roots themselves.
//
template <typename T, typename BEZIER_TUPLE,
    typename = typename std::enable_if<
        (std::tuple_size<BEZIER_TUPLE>::value == 2)>::type>
static_vector<T, std::tuple_size<BEZIER_TUPLE>::value+1>
bezier_roots(const BEZIER_TUPLE &B, T a = T{0}, T b = T{1}, T z = T{0}) {
    static_vector<T, std::tuple_size<BEZIER_TUPLE>::value+1> roots;
    roots.push_back(a);
    T d = static_cast<T>(std::get<1>(B)-std::get<0>(B));
    T n = static_cast<T>(z-std::get<0>(B));
    T s = static_cast<T>(util::sgn(d));
    d *= s; n *= s;
    if (d != 0 && n >= a*d && n <= b*d) {
        roots.push_back(n/d);
    }
    roots.push_back(b);
    return roots;
}

template <typename T, typename BEZIER_TUPLE,
    typename = typename std::enable_if<
        (std::tuple_size<BEZIER_TUPLE>::value > 2)>::type>
auto bezier_roots(const BEZIER_TUPLE &B, T a = T{0}, T b = T{1}, T z = T{0}) {
        auto DB = bezier_derivative(B);
    return refine_roots<T>(
        [&B](T t) { return bezier_evaluate_horner<T>(B, t); },
        [&DB](T t) { return bezier_evaluate_horner<T>(DB, t); },
        bezier_roots<T>(DB, a, b, z),
        z
    );
}

// Finds all real roots of the Bezier segment s(t) = z for t in [a,b].
// If the Bezier has n roots r_1, r_2, ... r_n, in the interval,
// the function returns a container with values
// {a, r_1, r_2, ..., r_n, b}
// I.e., the interval endpoints are included as well.
// They surround the roots themselves.
//
template <typename T, typename BEZIER_TUPLE,
    typename = typename std::enable_if<
        (std::tuple_size<BEZIER_TUPLE>::value == 2)>::type>
static_vector<T, std::tuple_size<BEZIER_TUPLE>::value+1>
bezier_chull_roots(const BEZIER_TUPLE &B) {
    static_vector<T, std::tuple_size<BEZIER_TUPLE>::value+1> roots;
    roots.push_back(0);
    T d = std::get<1>(B)-std::get<0>(B);
    T n = -std::get<0>(B);
    T s = util::sgn(d);
    d *= s; n *= s;
    if (d != 0 && n >= 0 && n <= d) {
        roots.push_back(n/d);
    }
    roots.push_back(1);
    return roots;
}

template <typename T, typename BEZIER_TUPLE,
    typename = typename std::enable_if<
        (std::tuple_size<BEZIER_TUPLE>::value > 2)>::type>
auto bezier_chull_roots(const BEZIER_TUPLE &B) {
    auto m = detail::minmax<T>(B);
    // no roots
    if (std::get<0>(m) > 0 || std::get<1>(m) < 0) {
        constexpr size_t DEGREE = static_cast<T>(
            std::tuple_size<BEZIER_TUPLE>::value-1);
        return static_vector<T, DEGREE+2>({0,1});
    // maybe roots
    } else {
        auto DB = bezier_derivative(B);
        return refine_roots<T>(
            [&B](T t) { return bezier_evaluate_horner<T>(B, t); },
            [&DB](T t) { return bezier_evaluate_horner<T>(DB, t); },
            bezier_chull_roots<T>(DB)
        );
    }
}

template <typename T, typename BEZIER_TUPLE,
    typename = typename std::enable_if<
        (std::tuple_size<BEZIER_TUPLE>::value > 2)>::type>
auto bezier_chull_roots(const BEZIER_TUPLE &B, T a, T b) {
    auto C = std::tuple_cat(
        std::make_tuple(bezier_evaluate_horner<T>(B, a)),
        bezier_cut<T>(B, a, b));
    auto roots = bezier_chull_roots<T>(C);
    for (auto &r: roots) {
        r = (1-r)*a+r*b;
    }
    return roots;
}

template <typename T, typename BEZIER_TUPLE,
    typename = typename std::enable_if<
        (std::tuple_size<BEZIER_TUPLE>::value > 2)>::type>
auto bezier_chull_roots(const BEZIER_TUPLE &B, T a, T b, T z) {
    auto C = tuple_map(std::tuple_cat(
        std::make_tuple(bezier_evaluate_horner<T>(B, a)),
        bezier_cut<T>(B, a, b)),
        [z](T t) { return t-z; });
    auto roots = bezier_chull_roots<T>(C);
    for (auto &r: roots) {
        r = (1-r)*a+r*b;
    }
    return roots;
}

// Finds all real roots of the Bezier segment s(t) = z for t in [a,b].
// If the Bezier has n roots r_1, r_2, ... r_n, in the interval,
// the function returns a container with values
// {a, r_1, r_2, ..., r_n, b}
// I.e., the interval endpoints are included as well.
// They surround the roots themselves.
// Use algorithm from Sedeberg's book CAGD 2014 chapter 9.1
//
namespace detail {

    template <typename BEZIER_TUPLE, size_t N, typename T,
        typename = typename std::enable_if<
            (std::tuple_size<BEZIER_TUPLE>::value == 2)>::type>
    void bezier_hull_marching_find_next_roots(const BEZIER_TUPLE &B,
        static_vector<T, N> &roots, T a, T b, void * = nullptr) {
        T d = std::get<1>(B)-std::get<0>(B);
        T n = -std::get<0>(B);
        T s = util::sgn(d);
        d *= s; n *= s;
        if (d != 0 && n >= 0 && n <= d) {
            T x = a+n*(b-a)/d;
            roots.push_back(x);
        }
    }

    template <typename BEZIER_TUPLE, size_t N, typename T,
        typename = typename std::enable_if<
            (std::tuple_size<BEZIER_TUPLE>::value > 2)>::type>
    void bezier_hull_marching_find_next_roots(const BEZIER_TUPLE &B,
        static_vector<T, N> &roots, T a, T b) {
        constexpr T DEGREE = static_cast<T>(
            std::tuple_size<BEZIER_TUPLE>::value-1);
        // Initially, y(t) = B(t), and x(t) = a*(1-t) + b*t
        // We want to find the coordinates x(ti) for each
        // root of y(ti) = 0 with ti in [0,1].
        auto C = B;
        T y0 = std::get<0>(C);
        T t0 = 0.f;
        // Repeatedly shrink the interval interval [t0,1] until
        // 1) we find t0 is a root and break
        // 2) we prove there can't be any additional roots and return
        // In the loop, C(u) = B(t0 + u*(1-t0))
        for ( ;; ) {
            T new_y0 = std::get<0>(C);
            // If new_y0 is a root or if we somehow (by numerical
            // error) found that we crossed the u axis between y0
            // and new_y0, we have found a root, so we break
            if (new_y0*y0 <= 0) break;
            y0 = new_y0;
            // Otherwise, (0, y0) is a vertex in the convex hull of C.
            // There are two other control points in the convex hull of that
            // are C adjacent to (0,y0).
            // We take the one for which the ray from (0,y0) through (u1,y1)
            // crosses the u axis (if at all possible)
            T s = util::sgn(y0);
            const auto h = tuple_reduce_indexed(tuple_drop<2>(C),
                [s](const std::pair<T,T> &prev, T y, size_t i) {
                T t = T(i+2);
                if (s*prev.first*t > s*y*prev.second) {
                    return std::make_pair(y, t);
                } else {
                    return prev;
                }
            }, std::make_pair(std::get<1>(C), T(1)));
            const T y1 = h.first;
            const T u1 = h.second/DEGREE;
            // Since the curve lives inside the convex hull of its
            // control points, there can be no root before the
            // intersection of our selected ray with the u axis. We try
            // to compute the u-intersect of our ray.
            T d = y1-y0; // denominator
            T n = -y0*u1; // numerator
            s = util::sgn(d);
            // If the intercept happens outside [0,1], there are no more roots
            if (d == 0 || s*n < 0 || s*n > s*d) {
                return;
            }
            // Otherwise, we advance t0 to the intersection
            T u = n/d;
            T new_t0 = t0 + u*(1-t0);
            if (t0 == new_t0) break;
            t0 = new_t0;
            C = std::tuple_cat(
                std::make_tuple(bezier_evaluate_horner(B, t0)),
                bezier_suffix(B, t0));
        }
        // At this point, we know that x(t0) is a root
        T x = a*(1-t0)+b*t0;
        roots.push_back(x);
        // Moreover C(u) = B(t0+u(1-t0)), so that u=0 is a root of C(u)=0
        // We can therefore factor u out of C and lower the degree of the
        // curve before searching for the remaining roots.
        bezier_hull_marching_find_next_roots(tuple_map_indexed(tuple_drop<1>(C),
            [](T y, size_t i) {
                return y*DEGREE/(i+1);
            }
        ), roots, x, b);
    }
}

template <typename BEZIER_TUPLE, typename T>
auto bezier_hull_marching_roots(const BEZIER_TUPLE &B, T a, T b, T z = T(0)) {
    const unsigned short DEGREE = static_cast<unsigned short>(
        std::tuple_size<BEZIER_TUPLE>::value-1);
    static_vector<T, DEGREE+2> roots;
    roots.push_back(a);
    // reduce to finding roots of s(t)=0 for t in [0,1]
    detail::bezier_hull_marching_find_next_roots(
        tuple_map(
            std::tuple_cat(
                std::make_tuple(bezier_evaluate_horner(B, a)),
                bezier_cut(B, a, b)),
        [z](T t) { return t-z; }), roots, a, b);
    roots.push_back(b);
    return roots;
}

template <typename T, typename P,
    typename BEZIER_TUPLE1, typename BEZIER_TUPLE2>
auto bezier_dot(const BEZIER_TUPLE1 &B, const BEZIER_TUPLE2 &C) {
    return tuple_map(bezier_product<T>(B, C), [](const P &p) {
        return sum(p);
    });
}


} // namespace rvg

#endif
