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
#ifndef RVG_INPUT_PATH_F_DOWNGRADE_DEGENERATES_H
#define RVG_INPUT_PATH_F_DOWNGRADE_DEGENERATES_H

#include "rvg-util.h"
#include "rvg-i-sink.h"
#include "rvg-i-point-input-path-f-forwarder.h"
#include "rvg-floatint.h"
#include "rvg-xform-svd.h"

namespace rvg {

// Assumes segments have already been monotonized
template<typename SINK>
class input_path_f_downgrade_degenerates :
    public i_point_input_path_f_forwarder<input_path_f_downgrade_degenerates<SINK>>,
    public i_sink<input_path_f_downgrade_degenerates<SINK>> {

    rvgf m_tol, m_tol2;
    SINK m_sink;
public:

    input_path_f_downgrade_degenerates(rvgf tol, SINK &&s):
        m_tol{tol},
        m_tol2{tol*tol},
        m_sink(std::forward<SINK>(s)) {
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
    }

friend i_sink<input_path_f_downgrade_degenerates<SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

friend i_point_input_path<input_path_f_downgrade_degenerates<SINK>>;

    void do_quadratic_segment(const R2 &p0, const R2 &p1, const R2 &p2) {
        // translate so q0 is at origin
        R2 q1 = p1-p0;
        R2 q2 = p2-p0;
        rvgf n = det(q1, q2);
        rvgf d2 = len2(q2);
        // if endpoints are close or
        // the middle control point is close to the line connecting them
        if (d2 <= m_tol2 || n*n <= d2*m_tol2) {
            // degenerate to linear segment connecting endpoints
            m_sink.linear_segment(p0, p2);
        } else {
            // otherwise, full quadratic
            m_sink.quadratic_segment(p0, p1, p2);
        }
    }

    void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2) {
		using util::is_almost_one;
		// make sure there are no ideal points for t in [0,1]
		assert(w1 > -1);
		// check for downgrade to integral quadratic segment
		if (is_almost_one(w1)) {
			this->quadratic_segment(x0, y0, x1, y1, x2, y2);
		// split if w1 < 0 so we know the convex hull property holds
		} else if (w1 < 0) {
			rvgf mx = rvgf{0.5}*(3*x1+x2)/(1+w1);
			rvgf my = rvgf{0.5}*(3*y1+y2)/(1+w1);
			rvgf p = std::fabs(2*(1+w1));
			rvgf s = std::sqrt(p);
			rvgf w = rvgf{0.5}*s;
			this->do_rational_quadratic_segment(
				x1, y1,
				2*x1/s, 2*y1/s, w,
				mx, my
			);
			this->do_rational_quadratic_segment(
				mx, my,
				(x1+x2)/s, (y1+y2)/s, w,
				x2, y2
			);
		// now we know the convex hull property holds
		} else {
			// translate so q0 is at origin
			R2 q1{x1-x0*w1, y1-y0*w1};
			R2 q2{x2-x0, y2-y0};
			rvgf d2 = len2(q2);
			rvgf n = det(q1, q2);
			// if endpoints are close or
			// the middle control point is close to the line connecting them
			if (d2 <= m_tol2 || n*n <= d2*w1*w1*m_tol2) {
				// degenerate to linear segment connecting endpoints
				m_sink.linear_segment(x0, y0, x2, y2);
			} else {
				// otherwise, full rational quadratic
				m_sink.rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2);
			}
		}
	}

    void do_rational_quadratic_segment(const R3 &p0, const R3 &p1, const R3 &p2) {
        R2 q0, q2;
        R3 q1;
        std::tie(q0, q1, q2) = canonize_rational_quadratic_bezier(p0, p1, p2);
        do_rational_quadratic_segment(q0[0], q0[1], q1[0], q1[1], q1[2],
			q2[0], q2[1]);
    }

    void do_cubic_segment(const R2 &p0, const R2 &p1, const R2 &p2, const R2 &p3) {
		// If the segment is an elevated quadratic, we can compute its
		// middle control point using either the first two or the
		// last two cubic control points
		R2 q0 = 0.5*(3*p1-p0);
		R2 q1 = 0.5*(3*p2-p3);
		// The difference between these two ways of computing the middle
		// control point of the quadratic is the same as half the cubic
		// coefficient of the polynomials, but it has a better geometric
		// interpretation which we can relate to our tolerance in pixels
		R2 dq = q1-q0;
		// If the difference is bigger than the tolerance, we downrade to
		// quadratic
		if (len2(dq) <= m_tol2) {
			R2 q = 0.5*(q0+q1);
			this->quadratic_segment(p0, q, p3);
		// Now check if the cubic is degenerate line
		} else {
			// Find the best rank 1 affine space for the 4 control points
			// See if the any control point is further than the tolerance
			// away from the space
			// Center so mean is zero
			R2 m = 0.25*(p0+p1+p2+p3);
			R2 q0 = p0-m, q1 = p1-m, q2 = p2-m, q3 = p3-m;
			// Compute covariance matrix
			rvgf a = q0[0]*q0[0]+q1[0]*q1[0]+q2[0]*q2[0]+q3[0]*q3[0];
			rvgf b = q0[0]*q0[1]+q1[0]*q1[1]+q2[0]*q2[1]+q3[0]*q3[1];
			rvgf d = q0[1]*q0[1]+q1[1]*q1[1]+q2[1]*q2[1]+q3[1]*q3[1];
			// Compute svd
			rotation U;
			scaling S;
			asvd(linearity{a, b, b, d}, U, S);
			auto u = perp(R2{U.get_cos(), U.get_sin()});
			if (std::fabs(dot(u, q0)) > m_tol ||
			    std::fabs(dot(u, q1)) > m_tol ||
			    std::fabs(dot(u, q2)) > m_tol ||
			    std::fabs(dot(u, q3)) > m_tol) {
				// If so, output cubic
				m_sink.cubic_segment(p0, p1, p2, p3);
			} else {
				// otherwise, approximate with line connecting endpoints
				m_sink.linear_segment(p0, p3);
			}
		}
    }
};

template <typename SINK>
inline auto make_input_path_f_downgrade_degenerates(rvgf tol, SINK &&sink) {
    return input_path_f_downgrade_degenerates<SINK>{tol, std::forward<SINK>(sink)};
}

}
#endif
