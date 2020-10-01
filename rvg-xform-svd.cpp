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
#ifdef USE_LAPACK
#include <lapacke.h>
#endif

#include "rvg-util.h"
#include "rvg-point.h"

#include "rvg-xform-svd.h"

//??D maybe copy code from gsl's svd2 auxiliary function instead?
//    https://fossies.org/dox/gsl-2.4/svdstep_8c_source.html
namespace rvg {

#ifdef USE_LAPACK
void asvd(const linearity &A, rotation &U, scaling &S, linearity &Vt) {
    char jobu = 'A';
    char jobvt = 'A';
    lapack_int m = 2;
    lapack_int n = 2;
    // lapack uses row-major-order
    double a[4] = {A[0][0], A[1][0], A[0][1], A[1][1]};
    lapack_int lda = 2;
    double s[2];
    double u[4];
    lapack_int ldu = 2;
    double vt[4];
    lapack_int ldvt = 2;
    double work[16];
    lapack_int lwork = 16;
    lapack_int info;
    LAPACK_dgesvd(&jobu, &jobvt, &m, &n, a, &lda, s, u, &ldu, vt, &ldvt,
        work, &lwork, &info);
    U = rotation{static_cast<rvgf>(u[0]), static_cast<rvgf>(u[1])};
    // If U has negative determinant, flip sign of U's
    // second column and of the second line of S
    if (u[0]*u[3]-u[2]*u[1] < 0) {
        S = scaling{static_cast<rvgf>(s[0]), -static_cast<rvgf>(s[1])};
    } else {
        S = scaling{static_cast<rvgf>(s[0]), static_cast<rvgf>(s[1])};
    }
    Vt = linearity{static_cast<rvgf>(vt[0]), static_cast<rvgf>(vt[2]),
        static_cast<rvgf>(vt[1]), static_cast<rvgf>(vt[3])};
}

void asvd(const linearity &A, rotation &U, scaling &S) {
    char jobu = 'A';
    char jobvt = 'N';
    lapack_int m = 2;
    lapack_int n = 2;
    // lapack uses row-major-order
    double a[4] = {A[0][0], A[1][0], A[0][1], A[1][1]};
    lapack_int lda = 2;
    double s[2];
    double u[4];
    lapack_int ldu = 2;
    lapack_int ldvt = 2;
    double work[16];
    lapack_int lwork = 16;
    lapack_int info;
    LAPACK_dgesvd(&jobu, &jobvt, &m, &n, a, &lda, s, u, &ldu, nullptr, &ldvt,
        work, &lwork, &info);
    U = rotation{static_cast<rvgf>(u[0]), static_cast<rvgf>(u[1])};
    // If U has negative determinant, flip sign of U's
    // second column and of the second line of S
    if (u[0]*u[3]-u[2]*u[1] < 0) {
        S = scaling{static_cast<rvgf>(s[0]), -static_cast<rvgf>(s[1])};
    } else {
        S = scaling{static_cast<rvgf>(s[0]), static_cast<rvgf>(s[1])};
    }
}
#else

// build a rotation from one of the
// vectors in the nullspace of a symmetric matrix
// {{r, s}, {s, t}}, which is known to be rank defficient
static rotation rotator(rvgf r, rvgf s, rvgf t) {
    using rvg::util::is_almost_zero;
    if (std::fabs(r) > std::fabs(t)) {
        rvgf h = std::hypot(r,s);
        if (!is_almost_zero(h)) {
            rvgf inv_h = 1.f/h;
            return rotation{s*inv_h,-r*inv_h};
        } else {
            return rotation{1.f,0.f};
        }
    } else {
        rvgf h = std::hypot(t,s);
        if (!is_almost_zero(h)) {
            rvgf inv_h = 1.f/h;
            return rotation{t*inv_h,-s*inv_h};
        } else {
            return rotation{1.f,0.f};
        }
    }
}

void asvd(const linearity &A, rotation &U, scaling &S) {
    using rvg::util::is_almost_zero;
    auto sq = [](rvgf a) { return a*a; };
    // we start computing the two roots of the characteristic
    // polynomial of AAt as t^2 -m t + p == 0
    const rvgf a = A[0][0], b = A[0][1], c = A[1][0], d = A[1][1];
    const rvgf a2 = sq(a), b2 = sq(b), c2 = sq(c), d2 = sq(d);
    const rvgf m = a2+b2+c2+d2;
    const rvgf p = sq(b*c-a*d);
    // sqrt of discriminant
    const rvgf D = std::hypot(b+c,a-d)*std::hypot(b-c,a+d);
    if (!is_almost_zero(m)) {
        // get two roots
        const rvgf el0 = .5f*(m+D);
        const rvgf el1 = p/el0;
        // so now we have the largest singular value
        const rvgf s0 = std::sqrt(el0);
        // get rotator from AAt - el0*I
        U = rotator(a2+b2-el0, a*c+b*d, c2+d2-el0);
        // we will also use the smallest singular value
        const rvgf s1 = std::sqrt(el1);
        if (!is_almost_zero(s1)) { // both singular values are above threshold
            S = scaling{s0, s1};
        } else { // only largest is above threshold
            S = scaling{s0, 0.f};
        }
    } else {
        // both roots are zero and so is the resulting matrix
        U = identity{};
        S = scaling{0.f,0.f};
    }
}

// analytic singular value decomposition of 2D matrix
void asvd(const linearity &A, rotation &U, scaling &S, linearity &Vt) {
    using rvg::util::is_almost_zero;
    auto sq = [](rvgf a) { return a*a; };
    // we start computing the two roots of the characteristic
    // polynomial of AAt as t^2 -m t + p == 0
    const rvgf a = A[0][0], b = A[0][1], c = A[1][0], d = A[1][1];
    const rvgf a2 = sq(a), b2 = sq(b), c2 = sq(c), d2 = sq(d);
    const rvgf m = a2+b2+c2+d2;
    const rvgf p = sq(b*c-a*d);
    // sqrt of discriminant
    const rvgf D = std::hypot(b+c,a-d)*std::hypot(b-c,a+d);
    if (!is_almost_zero(m)) {
        // get two roots
        const rvgf el0 = .5f*(m+D);
        const rvgf el1 = p/el0;
        // so now we have the largest singular value
        const rvgf s0 = std::sqrt(el0);
        // get rotator from AAt - el0*I
        U = rotator(a2+b2-el0, a*c+b*d, c2+d2-el0);
        // we will also use the smallest singular value
        const rvgf s1 = std::sqrt(el1);
        if (!is_almost_zero(s1)) { // both singular values are above threshold
            S = scaling{s0, s1};
            Vt = scaling{1.f/s0, 1.f/s1}*transpose(U)*A;
        } else { // only largest is above threshold
            S = scaling{s0, 0.f};
            R2 v0 = (1.f/s0)*transpose(A)*R2{U.get_cos(), U.get_sin()};
            Vt = linearity{v0[0], v0[1], -v0[1], v0[0]};
        }
    } else {
        // both roots are zero and so is the resulting matrix
        U = identity{};
        S = scaling{0.f,0.f};
        Vt = identity{};
    }
}
#endif

} // rvg
