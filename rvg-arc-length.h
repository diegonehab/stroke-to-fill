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
#ifndef RVG_ARC_LENGTH_H
#define RVG_ARC_LENGTH_H

#include <array>
#include <iostream>
#include <tuple>

#include "rvg-util.h"
#include "rvg-point.h"
#include "rvg-gaussian-quadrature.h"

#define RVG_ARC_LENGTH_INTERVALS (5)

namespace rvg {

// An implementation of
// Juttler, B. "A vegetarian approach to optimal parameterizations",
// Computer Aided Geometric Design, 14(9):887–890, 1997.
//
template <typename T, size_t N = RVG_ARC_LENGTH_INTERVALS>
class arc_length {
    T m_length;
    std::array<T, N+1> m_us;
    std::array<T, N> m_alphas;
    T m_a, m_b;

    void init_identity(void) {
        m_length = T(1);
        T dt = T(1)/N;
        m_us[0] = T(0);
        for (int i = 1; i < (int) N; i++) {
            m_us[i] = i*dt;
        }
        m_us[N] = T(1);
        for (int i = 0; i < (int) N; i++) {
            m_alphas[i] = 0.5f;
        }
    }

public:

    // Initialize with identity
    arc_length(void) {
        init_identity();
    }

    void print(void) const {
        std::cerr << "us: ";
        for (auto u: m_us) {
            std::cerr << u << ", ";
        }
        std::cerr << "\nas: ";
        for (auto a: m_alphas) {
            std::cerr << a << ", ";
        }
        std::cerr << "\n";
    }

    // Initialize with optimized parameterization
    template <typename DS2>
    arc_length(T a, T b, const DS2 &ds2, int q = 15) {
        optimize(a, b, ds2, q);
    }

    // finds the optimal parameterization
    // ds2 is the square norm of the derivative for the curve
    // [a,b] defines the piece of the curve we are interested in
    template <typename DS2>
    void optimize(T a, T b, const DS2 &ds2, int q = 15) {
        // Avoid degenerate interval
        if (util::is_almost_equal(a, b)) {
            return init_identity();
        }
        // Compute arc length of segment in [a,b]
        m_length = std::fabs(gaussian_quadrature<T>(
            [&](T t) -> T {
			    return T(std::sqrt(ds2(t)));
		    },
            a, b, q
        ));
        T n = T(N);
        T dt = (b-a)/n;
        int i = 1;
        auto dpi = [&](T t) -> T {
            auto ti = (t-a)/dt-i+1;
            return (1-ti)*(1-ti)*ds2(t);
        };
        auto dri = [&](T t) -> T {
            auto ti = (t-a)/dt-i+1;
            return 2*ti*(1-ti)*ds2(t);
        };
        auto dqi = [&](T t) -> T {
            auto ti = (t-a)/dt-i+1;
            return ti*ti*ds2(t);
        };
        T s = static_cast<T>(util::sgn(b-a));
        m_us[0] = T(0);
        while (i <= (int) N) {
            T pi = gaussian_quadrature(dpi, a+(i-1)*dt, a+i*dt, q);
            T ri = gaussian_quadrature(dri, a+(i-1)*dt, a+i*dt, q);
            T qi = gaussian_quadrature(dqi, a+(i-1)*dt, a+i*dt, q);
            T ai = pi/(pi + s*std::sqrt(pi*qi));
            assert(!std::isnan(ai));
            m_alphas[i-1] = ai;
            T ki = ((T(1)-ai)*(T(1)-ai)*pi+T(0.5)*ai*(T(1)-ai)*ri+ai*ai*qi) /
                (n*ai*(T(1)-ai));
            assert(!std::isnan(ki));
            m_us[i] = m_us[i-1] + std::sqrt(std::fabs(ki));
            assert(!std::isnan(m_us[i]));
            i = i + 1;
        }
        auto ul = m_us[N];
        for (auto &u: m_us) {
            u /= ul;
            assert(!std::isnan(u));
        }
        m_a = a;
        m_b = b;
    }

    T get_length(void) const {
        return m_length;
    }

    // For u in [0,1], the function returns the parameter t in [0,1]
    // so that the length of the curve piece in [a, a+(b-a)t] divided
    // by the length of the curve in [a,b] is u
    T get_relative_parameter_for_length_fraction(T u) const {
        int i = 0;
        for (; i < (int)(N+1); i++) {
            if (m_us[i] > u) break;
        }
        i = std::min((int)(N), std::max(1, i));
        T a = m_alphas[i-1];
        T du = m_us[i]-m_us[i-1];
        T ui = (u-m_us[i-1])/du;
        T ti = ui*(1-a)/((1-a)*ui + (1-ui)*a);
        auto r = (ti+i-T(1))/N;
        assert(!std::isnan(r));
        return r;
    }

    // For t in [0,1], the function returns the length of the
    // curve in [a, a+(b-a)t] divided by the length of the
    // curve in [a,b]
    T get_length_fraction_for_relative_parameter(T t) const {
        int i = static_cast<int>(N*t)+1;
        i = std::min((int)(N), std::max(1, i));
        T ti = N*t-i+1;
        T a = m_alphas[i-1];
        T ui = a*ti/(1-(1-a)*ti-a*(1-ti));
        T du = m_us[i]-m_us[i-1];
        auto r = ui*du+m_us[i-1];
        assert(!std::isnan(r));
        return r;
    }

    T get_absolute_parameter(T t) const {
        return m_a*(T{1}-t) + m_b*t;
    }

};

template <typename T, size_t N, typename DS2>
auto make_arc_length(T a, T b, const DS2 &ds2, int q = 15) {
    return arc_length<T, N>{a, b, ds2, q};
}


} // namespace rvg

#endif
