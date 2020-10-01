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
#ifndef RVG_I_XFORM_H
#define RVG_I_XFORM_H

#include <utility>
#include <iosfwd>

#include "rvg-floatint.h"
#include "rvg-meta.h"
#include "rvg-point.h"
#include "rvg-util.h"
#include "rvg-window.h"
#include "rvg-viewport.h"
#include "rvg-align.h"
#include "rvg-aspect.h"

namespace rvg {

template <typename DERIVED>
class i_xform {
    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }
public:
    // apply transformation to vector components
    auto apply(rvgf x, rvgf y, rvgf w) const {
        return derived().do_apply(x, y, w);
    }

    auto apply(rvgf x, rvgf y) const {
        return derived().do_apply(x, y);
    }

    // apply transformation to vector
    auto apply(const RP2 &p) const {
        return derived().do_apply(p);
    }

    auto apply(const R2 &e) const {
        return derived().do_apply(e);
    }

    // apply rotation
    auto rotated(rvgf deg) const {
        rvgf rad = util::rad(deg);
        return derived().do_rotated(std::cos(rad), std::sin(rad));
    }

    auto rotated(rvgf cos, rvgf sin) const {
        return derived().do_rotated(cos, sin);
    }

    auto rotated(rvgf deg, rvgf cx, rvgf cy) const {
        rvgf rad = util::rad(deg);
        return derived().do_translated(-cx, -cy).rotated(std::cos(rad), std::sin(rad)).
            translated(cx, cy);
    }

    // apply scaling
    auto scaled(rvgf sx, rvgf sy) const {
        return derived().do_scaled(sx, sy);
    }

    auto scaled(rvgf s) const {
        return derived().do_scaled(s, s);
    }

    // apply scaling
    auto scaled(rvgf sx, rvgf sy, rvgf cx, rvgf cy) const {
        return derived().do_translated(-cx, -cy).scaled(sx, sy).translated(cx, cy);
    }

    // apply translation
    auto translated(rvgf tx, rvgf ty) const {
        return derived().do_translated(tx, ty);
    }

    // apply linear transformation
    auto linear(rvgf a, rvgf b, rvgf c, rvgf d) const {
        return derived().do_linear(a, b, c, d);
    }

    // apply affine transformation
    auto affine(rvgf a, rvgf b, rvgf tx, rvgf c, rvgf d, rvgf ty) const {
        return derived().do_affine(a, b, tx, c, d, ty);
    }

    // apply projective transformation
    auto projected(rvgf a, rvgf b, rvgf c, rvgf d, rvgf e, rvgf f,
        rvgf g, rvgf h, rvgf i) const {
        return derived().do_projected(a, b, c, d, e, f, g, h, i);
    }

    // apply window-viewport
    auto windowviewport(const window &w, const viewport &v,
        e_align ax = e_align::mid, e_align ay = e_align::mid,
        e_aspect a = e_aspect::none) const {
        return derived().do_windowviewport(w, v, ax, ay, a);
    }

    // apply derived transformation to derived transformation
    DERIVED transformed(const DERIVED &o) const {
        return derived().do_transformed(o);
    }

    // return identity of derived type
    DERIVED without_xf(void) const {
        return derived().do_without_xf();
    }

    DERIVED operator*(const DERIVED &o) const {
        return o.transformed(derived());
    }

    auto operator*(rvgf s) const {
        return derived().do_scaled(s);
    }

    auto operator*(const R2 &r2) const {
        return derived().do_apply(r2);
    }

    auto operator*(const RP2 &rp2) const {
        return derived().do_apply(rp2);
    }

    // add and subtract derived
    auto added(const DERIVED &o) const {
        return derived().do_added(o);
    }

    auto subtracted(const DERIVED &o) const {
        return derived().do_subtracted(o);
    }

    auto operator+(const DERIVED &o) const {
        return derived().do_added(o);
    }

    auto operator-(const DERIVED &o) const {
        return derived().do_subtracted(o);
    }

    bool operator==(const DERIVED &o) const {
        return derived().do_is_equal(o);
    }

    bool operator!=(const DERIVED &o) const {
        return !derived().do_is_equal(o);
    }

    // print to ostream
    std::ostream &print(std::ostream &out) const {
        return derived().do_print(out);
    }

    bool is_almost_equal(const DERIVED &o) const {
        return derived().do_is_almost_equal(o);
    }

    bool is_equal(const DERIVED &o) const {
        return derived().do_is_equal(o);
    }

    rvgf det(void) const {
        return derived().do_det();
    }

    // return matrix adjugate where A * A.ajugate() == A.det() * Identity()
    auto adjugate(void) const {
        return derived().do_adjugate();
    }

    // return matrix transform
    auto transpose(void) const {
        return derived().do_transpose();
    }

    // compute inverse transformation
    DERIVED inverse(void) const {
        return derived().do_inverse();
    }

    bool is_identity(void) const {
        return derived().do_is_identity();
    }

    DERIVED get_xf(void) const {
        return derived();
    }
};

template <typename DERIVED>
std::ostream &operator<<(std::ostream &out, const i_xform<DERIVED> &xf) {
    return xf.print(out);
}

} // namespace rvg

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_xform = std::integral_constant<
    bool,
    is_crtp_of<
        rvg::i_xform,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

} } // rvg::meta

#endif
