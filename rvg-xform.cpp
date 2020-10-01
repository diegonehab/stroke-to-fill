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
#include <iostream>

#include "rvg-xform.h"

namespace rvg {

std::ostream &projectivity::do_print(std::ostream &out) const {
    out << "projectivity{" <<
        m_m[0][0] << ',' << m_m[0][1] << ',' << m_m[0][2] << ',' <<
        m_m[1][0] << ',' << m_m[1][1] << ',' << m_m[1][2] << ',' <<
        m_m[2][0] << ',' << m_m[2][1] << ',' << m_m[2][2] << '}';
    return out;
}

std::ostream &affinity::do_print(std::ostream &out) const {
    out << "affinity{" <<
        m_m[0][0] << ',' << m_m[0][1] << ',' << m_m[0][2] << ',' <<
        m_m[1][0] << ',' << m_m[1][1] << ',' << m_m[1][2] << '}';
    return out;
}

std::ostream &linearity::do_print(std::ostream &out) const {
    out << "linearity{" <<
        m_m[0][0] << ',' << m_m[0][1] << ',' <<
        m_m[1][0] << ',' << m_m[1][1] << '}';
    return out;
}

std::ostream &rotation::do_print(std::ostream &out) const {
    out << "rotation{" << m_cos << ',' << m_sin << '}';
    return out;
}

std::ostream &translation::do_print(std::ostream &out) const {
    out << "translation{" << m_tx << ',' << m_ty << '}';
    return out;
}

std::ostream &scaling::do_print(std::ostream &out) const {
    out << "scaling{" << m_sx << ',' << m_sy << '}';
    return out;
}

std::ostream &identity::do_print(std::ostream &out) const {
    out << "identity{}";
    return out;
}

affinity make_windowviewport(const rvg::window &wnd, const rvg::viewport &vp,
    e_align align_x, e_align align_y, e_aspect aspect) {
    rvgf wxl, wyb, wxr, wyt;
    std::tie(wxl, wyb) = wnd.bl();
    std::tie(wxr, wyt) = wnd.tr();
    rvgf wdx = wxr - wxl;
    rvgf wdy = wyt - wyb;
    rvgi vxl, vyb, vxr, vyt;
    std::tie(vxl, vyb) = vp.bl();
    std::tie(vxr, vyt) = vp.tr();
    rvgf vdx = static_cast<rvgf>(vxr - vxl);
    rvgf vdy = static_cast<rvgf>(vyt - vyb);
    rvgf xi = 0.f, xo = 0.f;
    switch (align_x) {
        case e_align::min:
            xi = wxl;
            xo = static_cast<rvgf>(vxl);
            break;
        case e_align::mid:
            xi = .5f*(wxl+wxr);
            xo = .5f*(static_cast<rvgf>(vxl+vxr));
            break;
        case e_align::max:
            xi = wxr;
            xo = static_cast<rvgf>(vxr);
            break;
    }
    rvgf yi = 0.f, yo = 0.f;
    switch (align_y) {
        case e_align::min:
            yi = wyb;
            yo = static_cast<rvgf>(vyb);
            break;
        case e_align::mid:
            yi = .5f*(wyb+wyt);
            yo = .5f*(static_cast<rvgf>(vyb+vyt));
            break;
        case e_align::max:
            yi = wyt;
            yo = static_cast<rvgf>(vyt);
            break;
    }
    rvgf sx = 0.f, sy = 0.f;
    switch (aspect) {
        case e_aspect::none:
            sx = static_cast<rvgf>(vdx)/wdx;
            sy = static_cast<rvgf>(vdy)/wdy;
            break;
        case e_aspect::extend:
            // if (wdy/wdx < vdy/vdx) {
            if (std::fabs(wdy*vdx) < std::fabs(vdy*wdx)) {
                sy = static_cast<rvgf>(vdy)/wdy;
                sx = sy*wdx/wdy;
            } else {
                sx = static_cast<rvgf>(vdx)/wdx;
                sy = sx*wdy/wdx;
            }
            break;
        case e_aspect::trim:
            if (std::fabs(wdy*vdx) < std::fabs(vdy*wdx)) {
                sx = static_cast<rvgf>(vdx)/wdx;
                sy = sx*wdy/wdx;
            } else {
                sy = static_cast<rvgf>(vdy)/wdy;
                sx = sy*wdx/wdy;
            }
            break;
    }
    return translation(-xi, -yi).scaled(sx, sy).translated(xo, yo);
}

}  // namespace rvg
