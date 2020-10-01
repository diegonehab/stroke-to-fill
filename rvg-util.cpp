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
#include <iostream>

namespace rvg {
    namespace util {

#ifdef USE_LAPACK
bool linear_solve(float a00, float a01, float a10, float a11,
    float b0, float b1, float *x, float *y) {
    lapack_int m = 2;
    lapack_int n = 2;
    lapack_int nrhs = 1;
    float a[4] = {a00, a10, a01, a11};
    lapack_int lda = 2;
    float b[2] = {b0, b1};
    lapack_int ldb = 2;
    lapack_int jpvt[2];
    float rcond = -1;
    lapack_int rank;
    float work[16];
    lapack_int lwork = 16;
    lapack_int info;
    LAPACK_sgelsy(&m, &n, &nrhs, a, &lda, b, &ldb, jpvt, &rcond, &rank,
        work, &lwork, &info);
    *x = b[0];
    *y = b[1];
    return true;
}
#else

template <typename T>
static bool linear_solve_helper(T a00, T a01, T a10, T a11, T b0, T b1,
    T *x, T *y) {
    if (is_almost_zero(a00)) {
        *x = T(0); *y = T(0);
        return false;
    }
    auto s = a10/a00;
    auto d = (a11-a01*s);
    if (is_almost_zero(d)) {
        *x = T(0); *y = T(0);
        return false;
    }
    *y = (b1-b0*s)/d;
    *x = (b0-a01*(*y))/a00;
    return true;
}

//??D This is not a good idea. Should stop writing linear
//    algebra code and use a library.
bool linear_solve(float a00, float a01, float a10, float a11, float b0, float b1, float *x, float *y) {
    // Pivot
    if (std::fabs(a00) < std::fabs(a10))
        return linear_solve_helper(a10, a11, a00, a01, b1, b0, x, y);
    else
        return linear_solve_helper(a00, a01, a10, a11, b0, b1, x, y);
}

bool linear_solve(double a00, double a01, double a10, double a11, double b0, double b1, double *x, double *y) {
    // Pivot
    if (std::fabs(a00) < std::fabs(a10))
        return linear_solve_helper(a10, a11, a00, a01, b1, b0, x, y);
    else
        return linear_solve_helper(a00, a01, a10, a11, b0, b1, x, y);
}
#endif

} } // namespace rvg::util
