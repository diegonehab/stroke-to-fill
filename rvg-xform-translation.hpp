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
inline RP2_tuple translation::do_apply(rvgf x, rvgf y, rvgf w) const {
    return RP2_tuple{x + m_tx*w, y+m_ty*w, w};
}

inline RP2 translation::do_apply(const RP2 &p) const {
    return RP2{p[0] + m_tx*p[2], p[1]+m_ty*p[2], p[2]};
}

inline R2_tuple translation::do_apply(rvgf x, rvgf y) const {
    return R2_tuple{x + m_tx, y + m_ty};
}

inline R2 translation::do_apply(const R2 &e) const {
    return R2{e[0] + m_tx, e[1] + m_ty};
}

inline translation translation::do_transformed(const translation &o) const {
#ifdef XFORM_DEBUG
    std::cerr << "translation.operator*(const translation &)\n";
#endif
    return translation{m_tx+o.m_tx, m_ty+o.m_ty};
}

inline translation translation::do_without_xf(void) const {
    return translation{};
}

inline affinity translation::do_scaled(rvgf sx, rvgf sy) const {
    return scaling{sx, sy} * (*this);
}

inline affinity translation::do_rotated(rvgf cos, rvgf sin) const {
    return rotation{cos, sin} * (*this);
}

inline translation translation::do_translated(rvgf tx, rvgf ty) const {
    return do_transformed(translation(tx, ty));
}

inline affinity translation::do_linear(rvgf a, rvgf b, rvgf c, rvgf d) const {
    return linearity{a, b, c, d} * (*this);
}

inline affinity translation::do_affine(rvgf a, rvgf b, rvgf tx,
    rvgf c, rvgf d, rvgf ty) const {
    return affinity{a, b, tx, c, d, ty} * (*this);
}

inline projectivity translation::do_projected(rvgf a, rvgf b, rvgf c,
    rvgf d, rvgf e, rvgf f, rvgf g, rvgf h, rvgf i) const {
    return projectivity{a, b, c, d, e, f, g, h, i} * (*this);
}

inline translation translation::do_added(const translation &o) const {
    return translation{m_tx+o.m_tx, m_ty+o.m_ty};
}

inline translation translation::do_subtracted(const translation &o) const {
    return translation{m_tx-o.m_tx, m_ty-o.m_ty};
}

inline affinity translation::do_windowviewport(const window &w,
    const viewport &v, e_align ax, e_align ay, e_aspect a) const {
    return make_windowviewport(w, v, ax, ay, a) * (*this);
}

inline bool translation::do_is_almost_equal(const translation &o) const {
    return util::is_almost_equal(m_tx, o.m_tx) &&
           util::is_almost_equal(m_ty, o.m_ty);
}

inline bool translation::do_is_equal(const translation &o) const {
    return m_tx == o.m_tx && m_ty == o.m_ty;
}

inline translation translation::do_adjugate(void) const {
    return translation{-m_tx, -m_ty};
}

inline translation translation::do_inverse(void) const {
    return translation{-m_tx, -m_ty};
}

inline bool translation::do_is_identity(void) const {
    return m_tx == 0.f && m_ty == 0.f;
}

inline projectivity translation::do_transpose(void) const {
    return projectivity{1, 0, 0, 0, 1, 0, m_tx, m_ty, 1};
}

inline rvgf translation::do_det(void) const {
    return 1.f;
}

inline rvgf translation::get_tx(void) const {
    return m_tx;
}

inline rvgf translation::get_ty(void) const {
    return m_ty;
}
