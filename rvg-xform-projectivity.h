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
#ifndef RVG_XFORM_PROJECTIVITY_H
#define RVG_XFORM_PROJECTIVITY_H

#include "rvg-i-xform.h"

namespace rvg {

class projectivity final: public i_xform<projectivity> {

    std::array<std::array<rvgf,3>,3> m_m;

public:
    // constructors
    projectivity(const projectivity &p) = default;

    explicit projectivity(const std::array<std::array<rvgf,3>,3> &m): m_m(m) {
#ifdef XFORM_DEBUG
        std::cerr << "projectivity(const std::array<std::array<rvgf,3>,3> &)\n";
#endif
    }

    projectivity(const R3 &c0, const R3 &c1, const R3 &c2):
        projectivity{ {{
            {{c0[0],c1[0],c2[0]}},
            {{c0[1],c1[1],c2[1]}},
            {{c0[2],c1[2],c2[2]}} }} }
        {
#ifdef XFORM_DEBUG
        std::cerr << "affinity(const R3 &, const R3 &, const R3 &)\n";
#endif
    }

    projectivity(rvgf a, rvgf b, rvgf c, rvgf d, rvgf e, rvgf f,
        rvgf g, rvgf h, rvgf i):
        projectivity{ {{
            {{a, b, c}},
            {{d, e, f}},
            {{g, h, i}} }} } {
#ifdef XFORM_DEBUG
        std::cerr << "projectivity(rvgf, ..., rvgf)\n";
#endif
    }

    projectivity():
        projectivity{ {{
            {{1.f,0.f,0.f}},
            {{0.f,1.f,0.f}},
            {{0.f,0.f,1.f}} }} } {
#ifdef XFORM_DEBUG
        std::cerr << "projectivity()\n";
#endif
    }

    const std::array<rvgf, 3> &operator[](int i) const;

private:
    friend i_xform<projectivity>;

    RP2_tuple do_apply(rvgf x, rvgf y, rvgf w) const;

    RP2_tuple do_apply(rvgf x, rvgf y) const;

    RP2 do_apply(const RP2 &p) const;

    RP2 do_apply(const R2 &e) const;

    projectivity do_transformed(const projectivity &o) const;

    projectivity do_without_xf(void) const;

    projectivity do_rotated(rvgf cos, rvgf sin) const;

    projectivity do_scaled(rvgf sx, rvgf sy) const;

    projectivity do_translated(rvgf tx, rvgf ty) const;

    projectivity do_linear(rvgf a, rvgf b, rvgf c, rvgf d) const;

    projectivity do_affine(rvgf a, rvgf b, rvgf tx,
        rvgf c, rvgf d, rvgf ty) const;

    projectivity do_projected(rvgf a, rvgf b, rvgf c, rvgf d, rvgf e, rvgf f,
        rvgf g, rvgf h, rvgf i) const;

    projectivity do_added(const projectivity &o) const;

    projectivity do_subtracted(const projectivity &o) const;

    projectivity do_windowviewport(const window &w, const viewport& v,
        e_align align_x, e_align align_y, e_aspect aspect) const;

    std::ostream &do_print(std::ostream &out) const;

    bool do_is_equal(const projectivity &o) const;

    bool do_is_almost_equal(const projectivity &o) const;

    bool do_is_identity(void) const;

    projectivity do_transpose(void) const;

    projectivity do_adjugate(void) const;

    projectivity do_inverse(void) const;

    rvgf do_det(void) const;
};


} // namespace rvg

#endif
