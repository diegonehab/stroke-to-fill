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
#ifndef RVG_XFORM_SCALING_H
#define RVG_XFORM_SCALING_H

#include "rvg-i-xform.h"

namespace rvg {

class scaling final: public i_xform<scaling> {

    rvgf m_sx, m_sy;

public:
    // constructors
    scaling(const scaling &s) = default;

    scaling(rvgf sx, rvgf sy): m_sx(sx), m_sy(sy) {
#ifdef XFORM_DEBUG
        std::cerr << "scaling(rvgf, rvgf)\n";
#endif
    }

    explicit scaling(rvgf s): scaling(s, s) {
#ifdef XFORM_DEBUG
        std::cerr << "explicit scaling(rvgf)\n";
#endif
    }

    scaling(): scaling(1.f, 1.f) {
#ifdef XFORM_DEBUG
        std::cerr << "scaling()\n";
#endif
    }

    // promotions
    operator linearity() const {
#ifdef XFORM_DEBUG
        std::cerr << "scaling.operator linearity()\n";
#endif
        return linearity(m_sx, 0.f, 0.f, m_sy);
    }

    operator affinity() const {
#ifdef XFORM_DEBUG
        std::cerr << "scaling.operator affinity()\n";
#endif
        return affinity(m_sx, 0.f, 0.f, 0.f, m_sy, 0.f);
    }

    operator projectivity() const {
#ifdef XFORM_DEBUG
        std::cerr << "scaling.operator projectivity()\n";
#endif
        return projectivity(m_sx, 0.f, 0.f, 0.f, m_sy, 0.f, 0.f, 0.f, 1.f);
    }

    rvgf get_sx(void) const;

    rvgf get_sy(void) const;

private:
    friend i_xform<scaling>;

    RP2_tuple do_apply(rvgf x, rvgf y, rvgf w) const;

    R2_tuple do_apply(rvgf x, rvgf y) const;

    RP2 do_apply(const RP2 &p) const;

    R2 do_apply(const R2 &e) const;

    scaling do_transformed(const scaling &o) const;

    scaling do_without_xf(void) const;

    linearity do_rotated(rvgf cos, rvgf sin) const;

    scaling do_scaled(rvgf sx, rvgf sy) const;

    affinity do_translated(rvgf tx, rvgf ty) const;

    linearity do_linear(rvgf a, rvgf b, rvgf c, rvgf d) const;

    affinity do_affine(rvgf a, rvgf b, rvgf tx, rvgf c, rvgf d, rvgf ty) const;

    projectivity do_projected(rvgf a, rvgf b, rvgf c, rvgf d, rvgf e, rvgf f,
        rvgf g, rvgf h, rvgf i) const;

    affinity do_windowviewport(const window &w, const viewport& v,
        e_align align_x, e_align align_y, e_aspect aspect) const;

    scaling do_added(const scaling &o) const;

    scaling do_subtracted(const scaling &o) const;

    std::ostream &do_print(std::ostream &out) const;

    bool do_is_equal(const scaling &o) const;

    bool do_is_almost_equal(const scaling &o) const;

    projectivity do_adjugate(void) const;

    bool do_is_identity(void) const;

    scaling do_transpose(void) const;

    scaling do_inverse(void) const;

    rvgf do_det(void) const;

};

} // namespace rvg

#endif
