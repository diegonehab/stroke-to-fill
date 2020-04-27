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
// apply transformation to vector
inline RP2_tuple rotation::do_apply(rvgf x, rvgf y, rvgf w) const {
    return RP2_tuple{m_cos * x - m_sin * y, m_sin * x + m_cos * y, w};
}

inline R2_tuple rotation::do_apply(rvgf x, rvgf y) const {
    return R2_tuple{m_cos * x - m_sin * y, m_sin * x + m_cos * y};
}

inline RP2 rotation::do_apply(const RP2 &p) const {
    return RP2{m_cos * p[0] - m_sin * p[1],
        m_sin * p[0] + m_cos * p[1], p[2]};
}

inline R2 rotation::do_apply(const R2 &e) const {
    return R2{m_cos * e[0] - m_sin * e[1],
        m_sin * e[0] + m_cos * e[1]};
}

inline rotation rotation::do_transformed(const rotation &o) const {
#ifdef XFORM_DEBUG
    std::cerr << "rotation.operator*(const rotation &)\n";
#endif
    return rotation(m_cos*o.m_cos - m_sin*o.m_sin, m_sin*o.m_cos + m_cos*o.m_sin);
}

inline rotation rotation::do_without_xf(void) const {
    return rotation{};
}

inline rotation rotation::do_rotated(rvgf cos, rvgf sin) const {
    return do_transformed(rotation{cos, sin});
}

inline linearity rotation::do_scaled(rvgf sx, rvgf sy) const {
    return scaling{sx, sy} * (*this);
}

inline linearity rotation::do_linear(rvgf a, rvgf b, rvgf c, rvgf d) const {
    return linearity{a, b, c, d} * (*this);
}

inline affinity rotation::do_affine(rvgf a, rvgf b, rvgf tx,
    rvgf c, rvgf d, rvgf ty) const {
    return affinity{a, b, tx, c, d, ty} * (*this);
}

inline projectivity rotation::do_projected(rvgf a, rvgf b, rvgf c,
    rvgf d, rvgf e, rvgf f, rvgf g, rvgf h, rvgf i) const {
    return projectivity{a, b, c, d, e, f, g, h, i} * (*this);
}

inline linearity rotation::do_added(const rotation &o) const {
    return linearity{
        m_cos+o.m_cos, m_sin+o.m_sin, 
        -m_sin-o.m_sin, m_cos+o.m_cos
    };
}

inline linearity rotation::do_subtracted(const rotation &o) const {
    return linearity{
        m_cos-o.m_cos, m_sin-o.m_sin, 
        -m_sin+o.m_sin, m_cos-o.m_cos
    };
}

inline affinity rotation::do_translated(rvgf tx, rvgf ty) const {
    return translation{tx, ty} * (*this);
}

inline affinity rotation::do_windowviewport(const window &w, const viewport &v,
    e_align ax, e_align ay, e_aspect a) const {
    return make_windowviewport(w, v, ax, ay, a) * (*this);
}

inline bool rotation::do_is_equal(const rotation &o) const {
    return m_cos == o.m_cos && m_sin == o.m_sin;
}

inline bool rotation::do_is_almost_equal(const rotation &o) const {
    return util::is_almost_equal(m_cos, o.m_cos) &&
           util::is_almost_equal(m_sin, o.m_sin);
}

inline projectivity rotation::do_adjugate(void) const {
    return projectivity{
        m_cos, m_sin, 0,
       -m_sin, m_cos, 0,
            0,     0, do_det()
    };
}

inline bool rotation::do_is_identity(void) const {
    return m_cos == 1.f && m_sin == 0.f;
}

inline rotation rotation::do_inverse(void) const {
    rvgf s = 1.f/do_det();
    return rotation{m_cos*s, -m_sin*s};
}

inline rotation rotation::do_transpose(void) const {
    return rotation{m_cos, -m_sin};
}

inline rvgf rotation::do_det(void) const {
    return m_cos*m_cos + m_sin*m_sin;
}

inline rvgf rotation::get_cos(void) const {
    return m_cos;
}

inline rvgf rotation::get_sin(void) const {
    return m_sin;
}
