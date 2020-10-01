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
#ifndef RVG_I_CUBIC_BEZIER_APPROXIMATOR_H
#define RVG_I_CUBIC_BEZIER_APPROXIMATOR_H

#include "rvg-meta.h"
#include "rvg-floatint.h"
#include "rvg-cubic-bezier-approximation.h"

#define RVG_CUBIC_BEZIER_APPROXIMATION_SAMPLES (16)
#define RVG_CUBIC_BEZIER_APPROXIMATION_TOLERANCE (1.e-2f)
#define RVG_CUBIC_BEZIER_APPROXIMATION_MAX_SUBDIVS (5)

namespace rvg {

template <size_t N, typename DERIVED>
class i_cubic_bezier_approximator {
protected:
    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:
    // This user-provided function must return an array with N+1 points that
    // approximate the desired curve at values of t in
    // ta + (0, d1, d2, ..., 1)*(tb-ta)
    // It should return false if just by sampling we already know we have to
    // subdivide
    bool sample(rvgf ta, const R2 &da, rvgf tb, const R2 &db,
        std::array<rvgf, N+1> &us, std::array<R2, N+1> &qs) {
        return derived().do_sample(ta, da, tb, db, us, qs);
    }

    // This user-provided function must take a cubic piece of the
    // approximation result
    void cubic_segment(const R2 &p0, const R2 &p1, const R2 &p2, const R2 &p3) {
        return derived().do_cubic_segment(p0, p1, p2, p3);
    }

    // This user-provided function must take a linear piece of the
    // approximation result
    void linear_segment(const R2 &p0, const R2 &p1) {
        return derived().do_linear_segment(p0, p1);
    }

    // This user-provided function must return the tangent direction at t
    R2 get_tangent_direction(rvgf t) {
        return derived().do_get_tangent_direction(t);
    }

    // This function calculates the approximation
    R2 approximate(rvgf ta, const R2 &da, rvgf tb, const R2 &db,
        rvgf tol, std::array<rvgf, N+1> &us, std::array<R2, N+1> &qs,
        int depth) {
        if (this->sample(ta, da, tb, db, us, qs)) {
            if (util::is_almost_equal(ta, tb)) {
                this->linear_segment(qs[0], qs[N]);
                return qs[N];
            }
            R2 q1, q2;
            rvgf err;
            std::tie(q1, q2, err) =
                cubic_bezier_approximation(qs, ta, da, tb, db, us, tol);
            // If error is small enough or we subdivided enough, output cubic
            if (err < tol ||
                depth >= RVG_CUBIC_BEZIER_APPROXIMATION_MAX_SUBDIVS) {
                // Forward cubic as approximation for interval
                this->cubic_segment(qs[0], q1, q2, qs[N]);
                return qs[N];
            }
        }
        // Otherwise, split in the middle
        rvgf tm = rvgf(0.5)*(ta+tb);
        auto dm = this->get_tangent_direction(tm);
        this->approximate(ta, da, tm, dm, tol, us, qs, depth+1);
        return this->approximate(tm, dm, tb, db, tol, us, qs, depth+1);
    }

    R2 approximate(rvgf ta, const R2 &da, rvgf tb, const R2 &db,
        rvgf tol) {
        std::array<rvgf, N+1> us;
        std::array<R2, N+1> qs;
        return this->approximate(ta, da, tb, db, tol, us, qs, 0);
    }

    // This function calculates the approximation
    template <typename CONTAINER>
    R2 approximate_partition(rvgf ta, const R2 &pa, const R2 &da,
        rvgf tb, const R2 &pb, const R2 &db,
        const CONTAINER &c, rvgf tol) {
        (void) pa;
        (void) pb;
        rvgf t0 = ta;
        R2 d0 = da;
        for (auto t1: c) {
            if (t1 > t0 && t1 < tb) {
                R2 d1 = this->get_tangent_direction(t1);
                this->approximate(t0, d0, t1, d1, tol);
                t0 = t1;
                d0 = d1;
            }
        }
        return this->approximate(t0, d0, tb, db, tol);
    }

};

} // namespace rvg

#endif
