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
#ifndef RVG_XFORM_H
#define RVG_XFORM_H

#ifdef XFORM_DEBUG
#include <iostream>
#endif

// Our xforms are transformations, and should not be confused
// with the matrices that represent them.
// These transformations operate on Euclidean points in R^2, on
// projective points in RP^2, and on each other.
// This means that if you multiply a transformation by a
// scalar, you are not multiplying the matrix by that
// scalar. You are creating a transformation that, when
// applied to a point, results in a scaled point.
// We define the scaling of points in the Euclydean plane.
// So the projective representation will not have all its
// coordinates multiplied by the scalar. Only the first two.
// By the same philosophy, our projective points can be
// added and multiplied by scalars.
#include "rvg-i-xform.h"
#include "rvg-xform-projectivity.h"
#include "rvg-xform-affinity.h"
#include "rvg-xform-linearity.h"
#include "rvg-xform-translation.h"
#include "rvg-xform-rotation.h"
#include "rvg-xform-scaling.h"
#include "rvg-xform-identity.h"
#include "rvg-xform-windowviewport.h"

namespace rvg {

// This is the default transformation type used
using xform = projectivity;

// Convenience functions
xform make_identity(void);
xform make_rotation(rvgf ang);
xform make_rotation(rvgf ang, rvgf cx, rvgf cy);
xform make_translation(rvgf tx, rvgf ty);
xform make_scaling(rvgf sx, rvgf sy, rvgf cx, rvgf cy);
xform make_scaling(rvgf s, rvgf cx, rvgf cy);
xform make_scaling(rvgf sx, rvgf sy);
xform make_scaling(rvgf s);
xform make_linearity(rvgf a, rvgf b, rvgf c, rvgf d);
xform make_affinity(rvgf a, rvgf b, rvgf tx, rvgf c, rvgf d, rvgf ty);
xform make_projectivity(rvgf a, rvgf b, rvgf c, rvgf d, rvgf e, rvgf f,
    rvgf g, rvgf h, rvgf i);

template <typename M,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_xform<M>::value>::type>
rvgf det(const M &m) {
    return m.det();
}

template <typename M,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_xform<M>::value>::type>
decltype(std::declval<const M&>().transpose())
transpose(const M &m) {
    return m.transpose();
}

template <typename M,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_xform<M>::value>::type>
decltype(std::declval<const M&>().adjugate())
adjugate(const M &m) {
    return m.adjugate();
}

#include "rvg-xform-product.hpp"
#include "rvg-xform-projectivity.hpp"
#include "rvg-xform-affinity.hpp"
#include "rvg-xform-linearity.hpp"
#include "rvg-xform-translation.hpp"
#include "rvg-xform-rotation.hpp"
#include "rvg-xform-scaling.hpp"
#include "rvg-xform-identity.hpp"
#include "rvg-xform.hpp"

} // namespace rvg

#endif
