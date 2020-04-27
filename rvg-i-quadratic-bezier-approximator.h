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
#ifndef RVG_I_QUADRATIC_BEZIER_APPROXIMATOR_H
#define RVG_I_QUADRATIC_BEZIER_APPROXIMATOR_H

#include <boost/range/adaptor/sliced.hpp>

#include "rvg-meta.h"
#include "rvg-floatint.h"
#include "rvg-bisect.h"

#define RVG_QUADRATIC_BEZIER_APPROXIMATION_TOLERANCE (1.e-2f)
#define RVG_QUADRATIC_BEZIER_APPROXIMATION_MAX_SUBDIVS (10)

namespace rvg {

template <typename DERIVED>
class i_quadratic_bezier_approximator {
protected:
    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

    rvgf dist_to_line2(const R2 &p, const R2 &q, const R2 &dq) {
        rvgf d = dot(p-q, perp(dq));
        return d*d/len2(dq);
    }

    R2 subdiv_or_line(
        rvgf ta, const R2 &pa, const R2 &da,
        rvgf tb, const R2 &pb, const R2 &db,
        rvgf tol, int depth) {
        if (depth < RVG_QUADRATIC_BEZIER_APPROXIMATION_MAX_SUBDIVS) {
            // Find middle point in the curve
            auto tm = rvgf{0.5}*(ta+tb);
            R2 pm, dm;
            std::tie(pm, dm) = derived().sample_and_tangent_direction(tm);
            R2 ab = pa-pb;
            if (len2(ab) > tol*tol || dist_to_line2(pm, pa, ab) < tol*tol) {
                this->approximate(ta, pa, da, tm, pm, dm, tol, depth+1);
                return this->approximate(tm, pm, dm, tb, pb, db, tol, depth+1);
            } else {
                derived().sink().linear_segment(pa, pb);
                return pb;
            }
        } else {
            derived().sink().linear_segment(pa, pb);
            return pb;
        }
    }

    R2 subdiv_or_quad(
        R2 q,
        rvgf ta, const R2 &pa, const R2 &da,
        rvgf tb, const R2 &pb, const R2 &db,
        rvgf tol, int depth) {
        if (depth < RVG_QUADRATIC_BEZIER_APPROXIMATION_MAX_SUBDIVS) {
            // Find middle point in the curve
            auto tm = rvgf{0.5}*(ta+tb);
            R2 pm, dm;
            std::tie(pm, dm) = derived().sample_and_tangent_direction(tm);
            R2 ab = pa-pb;
            if (len2(ab) > tol*tol || dist_to_line2(pm, pa, ab) < tol*tol) {
                this->approximate(ta, pa, da, tm, pm, dm, tol, depth+1);
                return this->approximate(tm, pm, dm, tb, pb, db, tol, depth+1);
            } else {
                derived().sink().quadratic_segment(pa, q, pb);
                return pb;
            }
        } else {
            derived().sink().quadratic_segment(pa, q, pb);
            return pb;
        }
    }

public:
    // This user-provided function return a point and it's tangent direction
    // at a parameter t
    std::pair<R2, R2> sample_and_tangent_direction(rvgf t) {
        return derived().do_sample_and_tangent_direction(t);
    }

    R2 sample(rvgf t) {
        return derived().do_sample(t);
    }

    R2 get_tangent_direction(rvgf t) {
        return derived().do_get_tangent_direction(t);
    }

    R2 approximate(
        rvgf ta, const R2 &pa, const R2 &da,
        rvgf tb, const R2 &pb, const R2 &db,
        rvgf tol, int depth = 0) {
        using namespace boost::adaptors;
        // If parameter space collapsed, connect with line
        if (util::is_almost_equal(ta, tb)) {
            derived().sink().linear_segment(pa, pb);
            return pb;
        }
        // If tangents are parallel, connect with line
        auto denom = cross(da,db);
        if (util::is_almost_zero(denom)) {
            if (dot(da, db) > 0) {
                derived().sink().linear_segment(pa, pb);
                return pb;
            } else {
                return subdiv_or_line(ta, pa, da, tb, pb, db, tol, depth);
            }
        }
        auto ab = pa-pb;
        auto numerA = cross(ab,db);
        auto numerB = cross(da,ab);
        // Intersection is outside chord
        if ((numerA >= 0) == (numerB <= 0)) {
            auto d1 = dist_to_line2(pa, pb, db);
            auto d2 = dist_to_line2(pb, pa, da);
            // If distance to tangent lines is small enough, connect with line
            if (std::fmax(d1, d2) < tol*tol) {
                derived().sink().linear_segment(pa, pb);
                return pb;
            //  Otherwise subdivide
            } else {
                return subdiv_or_line(ta, pa, da, tb, pb, db, tol, depth);
            }
        }
        numerA /= denom;
        bool validDivide = numerA > numerA - 1;
        if (!validDivide) {
            derived().sink().linear_segment(pa, pb);
            return pb;
        }
        auto q = pa - numerA*da;
        // If angle is less than pi/2
        if (dot(pa-q, pb-q) > 0) {
            return subdiv_or_line(ta, pa, da, tb, pb, db, tol, depth);
        }
        auto s = std::make_tuple(pa, q, pb);
        auto ds = bezier_differences(s);
        const int n = 1; // Increase n for robustness
        bool close_enough = true;
        for (rvgf t = 0.5/n; t < 1; t += rvgf{1}/n) {
            R2 pt = sample(t);
            auto st = bezier_evaluate_horner<rvgf>(s, t);
            // If quadratic approximation and curve at t are close enough
            if (len2(st-pt) < tol*tol) {
                continue;
            }
            auto dst = bezier_evaluate_horner<rvgf>(ds, t);
            // Find parameter of curve prallel to dst
            auto cr = [&](rvgf u) -> rvgf {
                return dot(perp(dst), get_tangent_direction(u));
            };
            rvgf tm = t;
            // If not found or quadratic approximation and curve are too far
            if (!bisect(cr, ta, tb, rvgf{0}, &tm) ||
                len2(sample(tm) - st) > tol*tol) {
                close_enough = false;
                break;
            }
        }
        if (close_enough) {
            derived().sink().quadratic_segment(pa, q, pb);
            return pb;
        } else {
            return subdiv_or_quad(q, ta, pa, da, tb, pb, db, tol, depth);
        }
    }

    // This function calculates the approximation
    template <typename CONTAINER>
    R2 approximate_partition(rvgf ta, const R2 &pa, const R2 &da,
        rvgf tb, const R2 &pb, const R2 &db,
        const CONTAINER &c, rvgf tol) {
        rvgf t0 = ta;
        R2 p0 = pa;
        R2 d0 = da;
        for (auto t1: c) {
            if (t1 > t0 && t1 < tb) {
                R2 p1, d1;
                std::tie(p1, d1) = derived().sample_and_tangent_direction(t1);
                this->approximate(t0, p0, d0, t1, p1, d1, tol);
                t0 = t1;
                p0 = p1;
                d0 = d1;
            }
        }
        return this->approximate(t0, p0, d0, tb, pb, db, tol);
    }

};

} // namespace rvg

#endif
