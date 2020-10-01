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
#ifndef RVG_BISECT_H
#define RVG_BISECT_H

#include <limits>
#include <boost/container/static_vector.hpp>

#include "rvg-adjacent-range.h"
#include "rvg-bisect.h"

namespace rvg {

// Use bisection to narrow down on a root for f(x) = z for x
// in the interval [a,b] (or [b,a]). We assume values f(a) and
// f(b) are *not* on the same side of z, so there is at least
// one root in [a,b] (or [b,a]). We also assume f(a) <= f(b).
//
namespace detail {
template <typename T, typename F>
T bisect_helper(F f, T a, T b, T z, int it = 0,
    int maxit = std::numeric_limits<T>::digits) {
    T m = T(.5)*(a+b);
    if (m == a || m == b || it >= maxit) return m;
    if (f(m) > z) {
        return bisect_helper<T>(f, a, m, z, it+1, maxit);
    } else {
        return bisect_helper<T>(f, m, b, z, it+1, maxit);
    }
}
}

template <typename T, typename F>
bool bisect(F f, T a, T b, T z, T *root) {
    T fa = f(a);
    if (fa == z) {
        *root = a;
        return true;
    }
    T fb = f(b);
    if (fb == z) {
        *root = b;
        return true;
    }
    if (fa < z && fb > z) {
        *root = detail::bisect_helper<T>(f, a, b, z);
        return true;
    }
    if (fa > z && fb < z) {
        *root = detail::bisect_helper<T>(f, b, a, z);
        return true;
    }
    return false;
}

//??D Should we use F &&f instead? To prevent copying the
// functor?
template <typename T, typename F, size_t N>
auto refine_roots(
    F f,
    const boost::container::static_vector<T, N> &critical,
    T z = T(0)
) {
    boost::container::static_vector<T, N+1> roots;
    roots.push_back(critical.front());
    for (const auto &adj: make_adjacent_range(critical)) {
        T root = T(0);
        if (bisect<T>(f, adj.first, adj.second, z, &root)) {
            roots.push_back(root);
        }
    }
    roots.push_back(critical.back());
    return roots;
}

template <typename T, typename F, typename DF>
bool safe_newton(const F &f, const DF &df, T a, T b, T z, T *root) {
    T fa = static_cast<T>(f(a));
    T fb = static_cast<T>(f(b));
    if ((fa > z && fb > z) || (fa < z && fb < z)) {
        return false;
    }
    if (fa == z) {
        *root = a;
        return true;
    }
    if (fb == z) {
        *root = b;
        return true;
    }
    T r0 = a;
    T r1 = b;
    if (fa >= z) {
        r0 = b;
        r1 = a;
    }
    T r = T(0.5)*(r0+r1);
    T old_dr = std::fabs(r1-r0);
    T dr = old_dr;
    T fr = static_cast<T>(f(r));
    T dfr = static_cast<T>(df(r));
    for (int i = 0; i < std::numeric_limits<T>::digits; i++) {
        if ((((r-r0)*dfr+z-fr)*((r-r1)*dfr+z-fr) > T(0)) ||
            (std::fabs(T(2)*fr) > std::fabs(old_dr*dfr))) {
            old_dr = dr;
            dr = T(0.5)*(r1-r0);
            r = r0+dr;
            if (r0 == r) {
                *root = r;
                return true;
            }
        } else {
            old_dr = dr;
            dr = (z-fr)/dfr;
            T t = r;
            r += dr;
            if (t == r) {
                *root = r;
                return true;
            }
        }
        if (util::is_almost_zero(dr))
            break;
        fr = static_cast<T>(f(r));
        dfr = static_cast<T>(df(r));
        if (fr < T(0)) {
            r0 = r;
        } else {
            r1 = r;
        }
    }
    *root = r;
    return true;
}


template <typename T, typename F, typename DF, size_t N>
auto refine_roots(
    const F &f,
    const DF &df,
    const boost::container::static_vector<T, N> &critical,
    T z = T(0)
) {
    boost::container::static_vector<T, N+1> roots;
    roots.push_back(critical.front());
    for (const auto &adj: make_adjacent_range(critical)) {
        T root = T(0);
        if (safe_newton<T>(f, df, adj.first, adj.second, z, &root)) {
            roots.push_back(root);
        }
    }
    roots.push_back(critical.back());
    return roots;
}

} // namespace rvg

#endif
