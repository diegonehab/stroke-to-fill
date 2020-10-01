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
#ifndef RVG_GOLDEN_SECTION_SEARCH_H
#define RVG_GOLDEN_SECTION_SEARCH_H

#include <limits>

namespace rvg {

template <typename F, typename T>
float golden_section_search(F f, T a, T b, T tol,
    int niter = std::numeric_limits<T>::digits) {
    constexpr T gr = T(1.618033988749895);
    T c = b - (b-a)/gr;
    T d = a + (b-a)/gr;
    int iter = 0;
    while (std::fabs(c-d) > tol && iter < niter) {
        if (f(c) < f(d)) {
            b = d;
        } else {
            a = c;
        }
        c = b - (b-a)/gr;
        d = a + (b-a)/gr;
        iter++;
    }
    return T(0.5)*(b+a);
}

} // namespace rvg

#endif
