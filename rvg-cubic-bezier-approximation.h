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
#ifndef RVG_CUBIC_BEZIER_APPROXIMATION_H
#define RVG_CUBIC_BEZIER_APPROXIMATION_H

#include "rvg-floatint.h"

#define RVG_CUBIC_BEZIER_APPROXIMATION_MAX_ITER (10)

namespace rvg {

#ifdef DUMP

static int dump_i = 0;
static rvgf debug_width = 1;

template <size_t N>
void dump_qs(const std::array<R2, N> &qs, const R2 &q1, const R2 &q2, const std::array<rvgf, N> &us, rvgf ta, rvgf tb) {
    char name[256];
    sprintf(name, "dump-%05d--%g--%g.svg", dump_i++, ta, tb);
    FILE *fp = fopen(name, "w");
    fprintf(fp, R"+(<?xml version="1.0" standalone="no"?>
<svg
   xmlns:xlink="http://www.w3.org/1999/xlink"
   xmlns:dc="http://purl.org/dc/elements/1.1/"
   xmlns:cc="http://creativecommons.org/ns#"
   xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   version="1.1"
   preserveAspectRatio="none"
   width="512" height="371"
   viewBox="0 0 512 371">
   <image opacity="0.2" x="0" y="0" width="512" height="371" xlink:href="a.svg"/>
   <g transform="matrix(1,0,0,-1,0,371)"> <!-- invert y -->
)+");

    fprintf(fp, "<path fill=\"none\" stroke=\"red\" stroke-width=\"%g\" stroke-linecap=\"round\" d=\"", 5*debug_width);
    for (const auto &p: qs) {
        fprintf(fp, "M %g %g %g %g ", p[0], p[1],  p[0]+0.01, p[1]+0.01);
    }
    fprintf(fp, "\"/>\n");

    auto s = std::tie(qs[0], q1, q2, qs[N-1]);
    fprintf(fp, "<path fill=\"none\" stroke=\"blue\" stroke-width=\"%g\" stroke-linecap=\"round\" d=\"", 3*debug_width);
    for (const auto &t: us) {
        auto p = bezier_evaluate_horner(s, t);
        fprintf(fp, "M %g %g %g %g ", p[0], p[1],  p[0]+0.01, p[1]+0.01);
    }
    fprintf(fp, "\"/>\n");

    fprintf(fp, "<path fill=\"none\" stroke=\"black\" stroke-width=\"%g\" d=\"M %g %g C %g %g %g %g %g %g\"/>\n", debug_width, qs[0][0], qs[0][1], q1[0], q1[1], q2[0], q2[1], qs[N-1][0], qs[N-1][1]);

    fprintf(fp, "<path fill=\"none\" stroke=\"black\" stroke-dasharray=\"%g %g\" stroke-width=\"%g\" d=\"M %g %g %g %g %g %g %g %g\"/>\n", 8*debug_width, 2*debug_width, debug_width, qs[0][0], qs[0][1], q1[0], q1[1], q2[0], q2[1], qs[N-1][0], qs[N-1][1]);

    fprintf(fp, "</g></svg>\n");
    fclose(fp);
}
#endif

// Approximate a set of points with a cubic Bezier using
// the method described in
// Hoscheck, J., 1988, "Spline approximation of offset curves",
// CAGD 5:33--40
//
// qs is an array of points to be approximated by the cubic
// qs[0] and qs[N] are interpolated
// d0 and d3 are the tangent directions at qs[0] and qs[N], respectively
// us[i] returns with the parameter that approximates qs[i]
// two control points q1 and q2 are returned, so that the
// cubic is given by q[0], q1, q2, and q[N]
template <typename P, typename T, size_t M>
std::tuple<P, P, T> cubic_bezier_approximation(const std::array<P, M> &qs,
    rvgf t0, const P &d0, rvgf t3, const P &d3, std::array<T, M> &us,
    rvgf tol = 1e-3) {
    (void) t0;
    (void) t3;
    constexpr auto N = M-1;
    // Alternate between finding the cubic that best
    // approximates the points, and finding the parameter
    // values in the approximation that best fit the points.
    // This works a bit like expectation-minimization, in the
    // sense that the error is reduced at each stage of each
    // iteration.
    R2 q0 = qs[0]; // initialize with line
    R2 q3 = qs[N];
    R2 q1 = q0+(q3-q0)/3.f;
    R2 q2 = q0+2.f*(q3-q0)/3.f;
    T last_err = std::numeric_limits<T>::max();
    int iter = 1;

    while (1) {

        // Find cubic that comes nearest to input points
        // at current parameters by solving a linear system
        T a00 = 0.f, a01 = 0.f, a10 = 0.f, a11 = 0.f, b0 = 0.f, b1 = 0.f;
        auto d00 = dot(d0,d0), d03 = dot(d0,d3), d33 = dot(d3,d3);
        for (unsigned i = 1; i < N; i++) {
            // Compute the cubic Bernstein polynomials at each parameter
            T ui = us[i];
            T si = T(1)-ui;
            T B0i = si*si*si;
            T B1i = T(3)*si*si*ui;
            T B2i = T(3)*si*ui*ui;
            T B3i = ui*ui*ui;
            // Add contribution to linear system
            a00 += d00*B1i*B1i;
            a01 += d03*B2i*B1i;
            a10 += d03*B1i*B2i;
            a11 += d33*B2i*B2i;
            auto qm = q0*(B0i+B1i)+q3*(B2i+B3i)-qs[i];
            b0 -= B1i*dot(qm, d0);
            b1 -= B2i*dot(qm, d3);
        }
        T el0, el3;
        // Solve for two optimal intermediate control points
        // If we failed to solve the linear system, abort
        // with current estimate
        if (!util::linear_solve(a00, a01, a10, a11, b0, b1, &el0, &el3)) {
            break;
        }

        //el0 = std::max(0.f, el0);
        //el3 = std::min(0.f, el3);

        q1 = q0+el0*d0;
        q2 = q3+el3*d3;

#ifdef DUMP
        dump_qs(qs, q0+el0*d0, q3+el3*d3, us, t0, t3);
#endif

        // Now find the parameters in the optimized
        // curve closest to each input point
        auto q = std::tie(q0,q1,q2,q3);
        auto dq = bezier_derivative(q);
        for (unsigned i = 1; i < N; i++) {
            const auto &qi = qs[i];
            rvgf ui = us[i];
            rvgf di = 0;
            rvgf err = std::numeric_limits<rvgf>::max();
            for (int j = 0; j < 5; j++) {
                ui += di;
                auto qti = bezier_evaluate_horner(q, ui);
                rvgf new_err = len2(qi-qti);
                if (ui > 0 && ui < 1 && new_err < err) {
                    err = new_err;
                    us[i] = ui;
                    auto dqti = bezier_evaluate_horner(dq, ui);
                    auto den = dot(dqti,dqti);
                    if (!util::is_almost_zero(den)) {
                        di = dot(qi-qti,dqti)/den;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }
        }

        // Compute new error
        T err = 0.f;
        for (unsigned i = 1; i < N; i++) {
            auto qi = bezier_evaluate_horner(q,us[i]);
            err += len2(qi-qs[i]);
        }

#ifdef DUMP
        dump_qs(qs, q0+el0*d0, q3+el3*d3, us, t0, t3);
#endif

        // If error is below tolerance, increased for some numerical
        // reason, or if the change in error is smaller than tolerance,
        // or we already iterated too much, we give up
        if (err < N*tol*tol ||
            iter >= RVG_CUBIC_BEZIER_APPROXIMATION_MAX_ITER ||
            err > last_err ||
            std::fabs(last_err-err) < tol*tol) {
            last_err = err;
            break;
        }
        last_err = err;
        iter++;
    }

    return std::make_tuple(q1, q2, std::sqrt(last_err)/N);
}



} // namespace rvg

#endif
