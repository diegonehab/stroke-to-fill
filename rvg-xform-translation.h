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
#ifndef RVG_XFORM_TRANSLATION_H
#define RVG_XFORM_TRANSLATION_H

#include "rvg-i-xform.h"

namespace rvg {

class translation final: public i_xform<translation> {

    rvgf m_tx, m_ty;

public:
    // constructors
    translation(const translation &t) = default;

    translation(rvgf tx, rvgf ty): m_tx(tx), m_ty(ty) {
#ifdef XFORM_DEBUG
        std::cerr << "translation(rvgf, rvgf)\n";
#endif
    }

    translation(): translation(0.f, 0.f) {
#ifdef XFORM_DEBUG
        std::cerr << "translation()\n";
#endif
    }

    // promotions
    operator affinity() const {
#ifdef XFORM_DEBUG
        std::cerr << "translation.operator affinity()\n";
#endif
        return affinity(1.f, 0.f, m_tx, 0.f, 1.f, m_ty);
    }

    operator projectivity() const {
#ifdef XFORM_DEBUG
        std::cerr << "translation.operator projectivity()\n";
#endif
        return projectivity(1.f, 0.f, m_tx, 0.f, 1.f, m_ty, 0.f, 0.f, 1.f);
    }

    rvgf get_tx(void) const;

    rvgf get_ty(void) const;

private:
    friend i_xform<translation>;

    RP2_tuple do_apply(rvgf x, rvgf y, rvgf w) const;

    R2_tuple do_apply(rvgf x, rvgf y) const;

    RP2 do_apply(const RP2 &p) const;

    R2 do_apply(const R2 &e) const;

    translation do_transformed(const translation &o) const;

    translation do_without_xf(void) const;

    affinity do_rotated(rvgf cos, rvgf sin) const;

    affinity do_scaled(rvgf sx, rvgf sy) const;

    translation do_translated(rvgf tx, rvgf ty) const;

    affinity do_linear(rvgf a, rvgf b, rvgf c, rvgf d) const;

    affinity do_affine(rvgf a, rvgf b, rvgf tx, rvgf c, rvgf d, rvgf ty) const;

    projectivity do_projected(rvgf a, rvgf b, rvgf c, rvgf d, rvgf e, rvgf f,
        rvgf g, rvgf h, rvgf i) const;

    translation do_added(const translation &o) const;

    translation do_subtracted(const translation &o) const;

    affinity do_windowviewport(const window &w, const viewport& v,
        e_align align_x, e_align align_y, e_aspect aspect) const;

    std::ostream &do_print(std::ostream &out) const;

    bool do_is_equal(const translation &o) const;

    bool do_is_almost_equal(const translation &o) const;

    bool do_is_identity(void) const;

    projectivity do_transpose(void) const;

    translation do_adjugate(void) const;

    translation do_inverse(void) const;

    rvgf do_det(void) const;
};

} // namespace rvg

#endif
