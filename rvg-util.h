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
#ifndef RVG_UTIL_H
#define RVG_UTIL_H

#include <cmath>
#include <algorithm>
#include <limits>
#include <tuple>
#include <cstdint>

#include "rvg-meta.h"

namespace rvg {
    namespace util {

template <typename F>
static F fmax(F f) {
    return f;
}

template <typename F, typename ... Fs, typename = typename
    std::enable_if<
        meta::is_all_same_or_convertible<F, Fs...>::value
    >::type
>
static F fmax(F f, Fs ... fs) {
    return std::fmax(f, fmax(fs...));
}

template <typename F, typename ... Fs, typename = typename
    std::enable_if<
        meta::is_all_same_or_convertible<F, Fs...>::value
    >::type
>
static F fmaxabs(F f, Fs ... fs) {
    return fmax(std::fabs(f), std::fabs(fs)...);
}

template <typename F>
static F fmin(F f) {
    return f;
}

template <typename F, typename ... Fs, typename = typename
    std::enable_if<
        meta::is_all_same_or_convertible<F, Fs...>::value
    >::type
>
static F fmin(F f, Fs ... fs) {
    return std::fmin(f, fmin(fs...));
}

template <typename F, typename ... Fs, typename = typename
    std::enable_if<
        meta::is_all_same_or_convertible<F, Fs...>::value
    >::type
>
static F fminabs(F f, Fs ... fs) {
    return std::fmin(std::fabs(f), std::fabs(fs)...);
}

template <typename T>
static int sgn(T t) {
    return (T{0} < t) - (t < T{0});
}

static inline constexpr float rad(float deg) {
    return deg*3.14159265358979f/180.f;
}

static inline constexpr float deg(float rad) {
    return rad*180.f/3.14159265358979f;
}

static inline constexpr double rad(double deg) {
    return deg*3.141592653589793238463/180.;
}

static inline constexpr double deg(double rad) {
    return rad*180./3.141592653589793238463;
}

template <typename F>
static constexpr F sq(F f) {
    return f*f;
}

template <typename F>
static
typename std::enable_if<!std::numeric_limits<F>::is_integer, bool>::type
is_relatively_zero(F a, F b, int ulp = 8) {
    return std::fabs(a) <= std::numeric_limits<F>::epsilon() *
        std::fabs(b) * ulp;
}

template <typename F>
static
typename std::enable_if<!std::numeric_limits<F>::is_integer, bool>::type
is_almost_zero(F f) {
    return std::fabs(f) < std::numeric_limits<F>::min(); // denorm
}

template <typename F>
static
typename std::enable_if<!std::numeric_limits<F>::is_integer, bool>::type
is_almost_equal(F a, F b, int ulp = 8) {
    return is_relatively_zero(a-b, a+b, ulp) || is_almost_zero(a-b);
}

template <typename F>
static
typename std::enable_if<!std::numeric_limits<F>::is_integer, bool>::type
is_almost_one(F f, int ulp = 8) {
    return std::fabs(f-1) <= std::numeric_limits<F>::epsilon() *
        std::fabs(f+1) * ulp;
}

template <typename F>
inline F len2(F dx, F dy) {
    return dx*dx+dy*dy;
}

template <typename F>
inline F len(F dx, F dy) {
    return std::hypot(dx, dy);
}

// Determinant of matrix {{x0, y0}, {x1, y1}}
template <typename F>
inline F det(F x0, F y0, F x1, F y1) {
    return x0*y1-x1*y0;
}

// Determinant of matrix {{x0, y0, 1}, {x1, y1, w1}, {x2, y2, 1}}
template <typename F>
inline F det(F x0, F y0, F x1, F y1, F w1, F x2, F y2) {
    return -x1*y0+w1*x2*y0+x0*y1-x2*y1-w1*x0*y2+x1*y2;
}

// Determinant of matrix {{x0, y0, 1}, {x1, y1, 1}, {x2, y2, 1}}
template <typename F>
inline F det(F x0, F y0, F x1, F y1, F x2, F y2) {
    return -x1*y0+x2*y0+x0*y1-x2*y1-x0*y2+x1*y2;
}

// Determinant of matrix {{x0, y0}, {x1, y1}}
// Returns the product of the diagonal elements in R of the
// QR decomposition obtained via Householder reflections.
namespace detail {
template <typename F>
inline F qrdet_helper(F x0, F y0, F n0, F x1, F y1) {
    if (is_almost_zero(n0)) return F{0};
    F s = x0 >= 0? 1: -1;
    F xu = x0+s*n0;
    F yv = y0/xu;
    F t = s*xu/n0;
    F d1 = x1+y1*yv;
    return s*n0*(y1-t*yv*d1);
}
}

template <typename F>
inline F qrdet(F x0, F y0, F x1, F y1) {
    F n0 = len(x0, y0), n1 = len(x1, y1);
    if (n0 > n1) {
        return detail::qrdet_helper(x0, y0, n0, x1, y1);
    } else {
        return -detail::qrdet_helper(x1, y1, n1, x0, y0);
    }
}

// Determinant of matrix {{x0, y0, 1}, {x1, y1, w1}, {x2, y2, 1}}
// Returns the product of the diagonal elements in R of the
// QR decomposition obtained via Householder reflections.
template <typename F>
inline F qrdet(F x0, F y0, F x1, F y1, F w1, F x2, F y2) {
    F n0 = std::sqrt(F{1}+y0*y0+x0*x0);
    F xu = F{1}+n0;
    F yv = y0/xu;
    F xv = x0/xu;
    F t = xu/n0;
    F d1 = w1+yv*y1+xv*x1;
    y1 -= t*yv*d1;
    x1 -= t*xv*d1;
    F d2 = F{1}+yv*y2+xv*x2;
    y2 -= t*yv*d2;
    x2 -= t*xv*d2;
    return n0*qrdet(x1, y1, x2, y2);
}

// Compute choose(n,k).
//
// We use the relation choose(n,k) = (choose(n, k-1) * (n-k+1))/k
// to recursively compute choose(n,0), choose(n,1), ..., and finally
// choose(n,k) using min(n-k,k) muls and divs.
// Since the multiplication is evaluated before the
// division, no floating-point numbers are needed. Since the
// division happens right after the product, no overflow
// happens.  This function is marked constexpr because it is
// evaluated by the compiler when the compiler knows n and
// k. In this way, many other algorithms that depend on
// choose(n,k) can be implemented faster.
//
static inline constexpr int choose(int n, int k) {
    return ((2*k > n)?
        choose(n, n-k):
        ((k > n || k < 0)?
            0:
            ((k == n || k == 0)?
                1:
                ((n-k+1)*(choose(n, k-1)))/k
            )
        )
    );
}

// Computes a pair of numbers (s, t) such that
//   s is the closest floating-point number to a+b
//   s+t = a+b exactly
// See "Handbook of floating-point arithmetic", Muler et al. 2010
//
template <typename T>
static std::pair<T, T> two_sum(T a, T b) {
    T s = a + b;
    T ap = s - b;
    T bp = s - a;
    T da = a - ap;
    T db = b - bp;
    T t = da + db;
    return std::make_pair(s, t);
}

// Computes a pair of numbers (m, n) such that
//   m is the closest floating-point number to a*b
//   m+n = a*b exactly
// See "Handbook of floating-point arithmetic", Muler et al. 2010
//
template <typename T>
static std::pair<T, T> two_prod(T a, T b) {
    T m = a*b;
    T n = std::fma(a, b, -m);
    return std::make_pair(m, n);
}

bool linear_solve(float a00, float a01, float a10, float a11,
    float b0, float b1, float *x, float *y);

bool linear_solve(double a00, double a01, double a10, double a11,
    double b0, double b1, double *x, double *y);

} } // namespace rvg::math

#endif
