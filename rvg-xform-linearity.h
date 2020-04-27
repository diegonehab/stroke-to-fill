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
#ifndef RVG_XFORM_LINEARITY_H
#define RVG_XFORM_LINEARITY_H

#include <array>

#include "rvg-i-xform.h"

namespace rvg {

class linearity final: public i_xform<linearity> {

    std::array<std::array<rvgf,2>,2> m_m;

public:
    // constructors
    linearity(const linearity &l) = default;

    explicit linearity(const std::array<std::array<rvgf,2>,2> &m): m_m(m) {
#ifdef XFORM_DEBUG
        std::cerr << "linearity(const std::array<std::array<rvgf,2>,2> &)\n";
#endif
    }

    linearity(const R2 &c0, const R2 &c1):
        linearity{ {{ {{c0[0],c1[0]}}, {{c0[1],c1[1]}} }} } {
#ifdef XFORM_DEBUG
        std::cerr << "linearity(const R2 &, const R2 &)\n";
#endif
    }

    linearity(rvgf a, rvgf b, rvgf c, rvgf d):
        linearity{ {{ {{a, b}} ,{{c, d}} }}} {
#ifdef XFORM_DEBUG
        std::cerr << "linearity(rvgf, ..., rvgf)\n";
#endif
    }

    linearity(): linearity{ {{ {{1.f, 0.f}}, {{0.f, 1.f}} }} } {
#ifdef XFORM_DEBUG
        std::cerr << "linearity()\n";
#endif
    }

    // promotions
    operator affinity() const {
#ifdef XFORM_DEBUG
        std::cerr << "linearity.operator affinity()\n";
#endif
        return affinity{m_m[0][0], m_m[0][1], 0.f, m_m[1][0], m_m[1][1], 0.f};
    }

    operator projectivity() const {
#ifdef XFORM_DEBUG
        std::cerr << "linearity.operator projectivity()\n";
#endif
        return projectivity{m_m[0][0], m_m[0][1], 0.f, m_m[1][0], m_m[1][1], 0.f, 0.f, 0.f, 1.f};
    }

    const std::array<rvgf,2> &operator[](int i) const;

private:
    friend i_xform<linearity>;

    RP2_tuple do_apply(rvgf x, rvgf y, rvgf w) const;

    R2_tuple do_apply(rvgf x, rvgf y) const;

    RP2 do_apply(const RP2 &p) const;

    R2 do_apply(const R2 &e) const;

    linearity do_transformed(const linearity &o) const;

    linearity do_without_xf(void) const;

    linearity do_rotated(rvgf cos, rvgf sin) const;

    linearity do_scaled(rvgf sx, rvgf sy) const;

    linearity do_added(const linearity &o) const;

    linearity do_subtracted(const linearity &o) const;

    affinity do_translated(rvgf tx, rvgf ty) const;

    linearity do_linear(rvgf a, rvgf b, rvgf c, rvgf d) const;

    affinity do_affine(rvgf a, rvgf b, rvgf tx, rvgf c, rvgf d, rvgf ty) const;

    projectivity do_projected(rvgf a, rvgf b, rvgf c, rvgf d, rvgf e, rvgf f,
        rvgf g, rvgf h, rvgf i) const;

    affinity do_windowviewport(const window &w, const viewport& v,
        e_align align_x, e_align align_y, e_aspect aspect) const;

    std::ostream &do_print(std::ostream &out) const;

    bool do_is_equal(const linearity &o) const;

    bool do_is_almost_equal(const linearity &o) const;

    bool do_is_identity(void) const;

    linearity do_adjugate(void) const;

    linearity do_transpose(void) const;

    linearity do_inverse(void) const;

    rvgf do_det(void) const;
};


} // namespace rvg

#endif
