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
#ifndef RVG_CANONIZE_RATIONAL_QUADRATIC_BEZIER_H
#define RVG_CANONIZE_RATIONAL_QUADRATIC_BEZIER_H

#include <cassert>

#include "rvg-util.h"
#include "rvg-point.h"

namespace rvg {

static inline
std::tuple<R2, R3, R2>
canonize_rational_quadratic_bezier(const R3 &p0, const R3 &p1, const R3 &p2) {
	if (!util::is_almost_one(p0[2]) || !util::is_almost_one(p2[2])) {
		// convert rational quadratic segment to its canonic form, where the
		// first and last control points are in the affine plane
		// make sure curve is bounded
		rvgf w0w2 = p0[2]*p2[2];
		// ??D there should be two kinds of rational
		// quadratic segments, one that can be expressed
		// with w0=1 and w2=1, and another that can be
		// expressed as w0=1 but w2=-1. This second kind
		// can never arise from affine transformations
		// of ellipses, but could appear when we allow
		// projective transformations. We will probably
		// have to deal with them in the future.
		assert(w0w2 > 0 && !util::is_almost_zero(w0w2));
		// convert segment
		rvgf iw0 = 1/p0[2];
		rvgf iw1 = 1/std::sqrt(w0w2);
		rvgf iw2 = 1/p2[2];
		return std::make_tuple(R2{p0[0]*iw0, p0[1]*iw0},
			R3{p1[0]*iw1, p1[1]*iw1, p1[2]*iw1}, R2{p2[0]*iw2, p2[1]*iw2});
	} else {
		return std::make_tuple(R2{p0[0],p0[1]}, p1, R2{p2[0], p2[1]});
	}
}

} // namespace rvg

#endif
