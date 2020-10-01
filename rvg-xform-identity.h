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
#ifndef RVG_XFORM_IDENTITY_H
#define RVG_XFORM_IDENTITY_H

#include "rvg-i-xform.h"

namespace rvg {

class identity final: public i_xform<identity> {
public:
    // promotions
    operator scaling() const {
#ifdef XFORM_DEBUG
        std::cerr << "identity.operator scaling()\n";
#endif
        return scaling(1.0f, 1.0f);
    }

    operator rotation() const {
#ifdef XFORM_DEBUG
        std::cerr << "identity.operator rotation()\n";
#endif
        return rotation(1.f, 0.f);
    }

    operator translation() const {
#ifdef XFORM_DEBUG
        std::cerr << "identity.operator translation()\n";
#endif
        return translation(0.f, 0.f);
    }

    operator linearity() const {
#ifdef XFORM_DEBUG
        std::cerr << "identity.operator linearity()\n";
#endif
        return linearity(1.f, 0.f, 0.f, 1.f);
    }

    operator affinity() const {
#ifdef XFORM_DEBUG
        std::cerr << "identity.operator affinity()\n";
#endif
        return affinity(1.f, 0.f, 0.f, 0.f, 1.f, 0.f);
    }

    operator projectivity() const {
#ifdef XFORM_DEBUG
        std::cerr << "identity.operator projectivity()\n";
#endif
        return projectivity(1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f);
    }

    // constructors
    identity() = default;

private:
    friend i_xform<identity>;

    RP2_tuple do_apply(rvgf x, rvgf y, rvgf w) const;

    R2_tuple do_apply(rvgf x, rvgf y) const;

    RP2 do_apply(const RP2 &p) const;

    R2 do_apply(const R2 &e) const;

    identity do_transformed(const identity &) const;

    identity do_without_xf(void) const;

    rotation do_rotated(rvgf cos, rvgf sin) const;

    translation do_translated(rvgf tx, rvgf ty) const;

    scaling do_scaled(rvgf sx, rvgf sy) const;

    scaling do_added(const identity &) const;

    scaling do_subtracted(const identity &) const;

    linearity do_linear(rvgf a, rvgf b, rvgf c, rvgf d) const;

    affinity do_affine(rvgf a, rvgf b, rvgf tx, rvgf c, rvgf d, rvgf ty) const;

    projectivity do_projected(rvgf a, rvgf b, rvgf c, rvgf d, rvgf e, rvgf f,
        rvgf g, rvgf h, rvgf i) const;

    affinity do_windowviewport(const window &w, const viewport& v,
        e_align ax, e_align ay, e_aspect a) const;

    std::ostream &do_print(std::ostream &out) const;

    bool do_is_equal(const identity &) const;

    bool do_is_almost_equal(const identity &) const;

    bool do_is_identity(void) const;

    identity do_transpose(void) const;

    identity do_adjugate(void) const;

    identity do_inverse(void) const;

    rvgf do_det(void) const;
};

} // namespace rvg

#endif
