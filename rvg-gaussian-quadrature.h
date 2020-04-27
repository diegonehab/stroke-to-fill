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
#ifndef RVG_GAUSSIAN_QUADRATURE_H
#define RVG_GAUSSIAN_QUADRATURE_H

#include <array>
#include <cmath>

namespace rvg  {

extern const std::array<std::pair<double,double>, 525>
    gaussian_quadrature_weights;

template <typename T, typename F>
static T gaussian_quadrature(F f, T a, T b, int n = 5) {
    n = std::min(32, std::max(3, n));
    unsigned first_sample = n*(n-1)/2-3;
    unsigned last_sample = n*(n+1)/2-3;
    T s = 0.f;
    for (unsigned i = first_sample; i < last_sample; i++) {
        const auto &sample = gaussian_quadrature_weights[i];
		T t = T(sample.first);
		T w = T(sample.second);
		s += f(a*(T(1)-t)+b*t)*w;
    }
    return (b-a)*s;
}

} // namespace

#endif
