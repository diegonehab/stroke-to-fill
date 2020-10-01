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
inline linearity linearity::do_scaled(rvgf sx, rvgf sy) const {
    return scaling{sx, sy} * (*this);
}

inline linearity linearity::do_rotated(rvgf cos, rvgf sin) const {
    return rotation{cos, sin} * (*this);
}

inline affinity linearity::do_translated(rvgf tx, rvgf ty) const {
    return translation{tx, ty} * (*this);
}

inline linearity linearity::do_linear(rvgf a, rvgf b, rvgf c, rvgf d) const {
    return linearity{a, b, c, d} * (*this);
}

inline affinity linearity::do_affine(rvgf a, rvgf b, rvgf tx,
    rvgf c, rvgf d, rvgf ty) const {
    return affinity{a, b, tx, c, d, ty} * (*this);
}

inline projectivity linearity::do_projected(rvgf a, rvgf b, rvgf c,
    rvgf d, rvgf e, rvgf f, rvgf g, rvgf h, rvgf i) const {
    return projectivity{a, b, c, d, e, f, g, h, i} * (*this);
}

inline linearity linearity::do_added(const linearity &o) const {
    return linearity{
        m_m[0][0]+o.m_m[0][0], m_m[0][1]+o.m_m[0][1],
        m_m[1][0]+o.m_m[1][0], m_m[1][1]+o.m_m[1][1]
    };
}

inline linearity linearity::do_subtracted(const linearity &o) const {
    return linearity{
        m_m[0][0]-o.m_m[0][0], m_m[0][1]-o.m_m[0][1],
        m_m[1][0]-o.m_m[1][0], m_m[1][1]-o.m_m[1][1]
    };
}

inline affinity linearity::do_windowviewport(const window &w, const viewport &v,
    e_align ax, e_align ay, e_aspect a) const {
    return make_windowviewport(w, v, ax, ay, a) * (*this);
}

inline RP2_tuple linearity::do_apply(rvgf x, rvgf y, rvgf w) const {
    rvgf rx = m_m[0][0] * x + m_m[0][1] * y;
    rvgf ry = m_m[1][0] * x + m_m[1][1] * y;
    return RP2_tuple{rx, ry, w};
}

inline RP2 linearity::do_apply(const RP2 &p) const {
    rvgf rx = m_m[0][0] * p[0] + m_m[0][1] * p[1];
    rvgf ry = m_m[1][0] * p[0] + m_m[1][1] * p[1];
    return RP2{rx, ry, p[2]};
}

inline R2_tuple linearity::do_apply(rvgf x, rvgf y) const {
    rvgf rx = m_m[0][0] * x + m_m[0][1] * y;
    rvgf ry = m_m[1][0] * x + m_m[1][1] * y;
    return R2_tuple{rx, ry};
}

inline R2 linearity::do_apply(const R2 &e) const {
    rvgf rx = m_m[0][0] * e[0] + m_m[0][1] * e[1];
    rvgf ry = m_m[1][0] * e[0] + m_m[1][1] * e[1];
    return R2{rx, ry};
}

inline linearity linearity::do_transformed(const linearity &o) const {
#ifdef XFORM_DEBUG
std::cerr << "linearity.operator*(linearity)\n";
#endif
    return linearity{m_m[0][0]*o.m_m[0][0] + m_m[1][0]*o.m_m[0][1],
        m_m[0][1]*o.m_m[0][0] + m_m[1][1]*o.m_m[0][1],
        m_m[0][0]*o.m_m[1][0] + m_m[1][0]*o.m_m[1][1],
        m_m[0][1]*o.m_m[1][0] + m_m[1][1]*o.m_m[1][1]};
}

inline linearity linearity::do_without_xf(void) const {
    return linearity{};
}

inline bool linearity::do_is_almost_equal(const linearity &o) const {
    return util::is_almost_equal(m_m[0][0], o.m_m[0][0]) &&
           util::is_almost_equal(m_m[0][1], o.m_m[0][1]) &&
           util::is_almost_equal(m_m[1][0], o.m_m[1][0]) &&
           util::is_almost_equal(m_m[1][1], o.m_m[1][1]);
}

inline bool linearity::do_is_equal(const linearity &o) const {
    return m_m == o.m_m;
}

inline bool linearity::do_is_identity(void) const {
    return m_m[0][0] == 1.f && m_m[0][1] == 0.f &&
           m_m[1][0] == 0.f && m_m[1][1] == 1.f;
}

inline linearity linearity::do_adjugate(void) const {
    return linearity{
        m_m[1][1], -m_m[0][1],
       -m_m[1][0],  m_m[0][0],
    };
}

inline linearity linearity::do_inverse(void) const {
    rvgf s = 1.f/do_det();
    return linearity{
        m_m[1][1]*s, -m_m[0][1]*s,
       -m_m[1][0]*s,  m_m[0][0]*s
    };
}

inline linearity linearity::do_transpose(void) const {
    return linearity{m_m[0][0], m_m[1][0], m_m[0][1], m_m[1][1]};
}

inline rvgf linearity::do_det(void) const {
    return m_m[0][0]*m_m[1][1] - m_m[0][1]*m_m[1][0];
}

inline const std::array<rvgf,2> &linearity::operator[](int i) const {
    return m_m[i];
}

static inline
RP2 intersection(const R2 &p0, const R2 &d0, const R2 &p1, const R2 &d1) {
    auto d01 = p1-p0;
    auto M = linearity{d0, -d1}.adjugate();
    auto u = M.det();
    auto v = M.apply(d01);
    return RP2{p0*u + v[0]*d0, u};
}

static inline
bool intersectq(const R2 &p0, const R2 &q0, const R2 &p1, const R2 &q1) {
    auto d01 = p1-p0;
    auto M = linearity{q0-p0, p1-q1}.adjugate();
    auto u = M.det();
    auto su = rvgf(util::sgn(u));
    auto au = std::fabs(u);
    auto v = su*M.apply(d01);
    return 0 < v[0] && v[0] < au && 0 < v[1] && v[1] < au;
}
