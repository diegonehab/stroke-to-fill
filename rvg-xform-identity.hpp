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
inline RP2_tuple identity::do_apply(rvgf x, rvgf y, rvgf w) const {
    return RP2_tuple{x, y, w};
}

inline R2_tuple identity::do_apply(rvgf x, rvgf y) const {
    return R2_tuple{x, y};
}

inline RP2 identity::do_apply(const RP2 &p) const {
    return p;
}

inline R2 identity::do_apply(const R2 &e) const {
    return e;
}

inline identity identity::do_transformed(const identity &) const {
#ifdef XFORM_DEBUG
    std::cerr << "identity.operator*(const identity &)\n";
#endif
    return identity{};
}

inline identity identity::do_without_xf(void) const {
    return identity{};
}

inline rotation identity::do_rotated(rvgf cos, rvgf sin) const {
    return rotation{cos, sin};
}

inline translation identity::do_translated(rvgf tx, rvgf ty) const {
    return translation{tx, ty};
}

inline scaling identity::do_added(const identity &) const {
    return scaling{2, 2};
}

inline scaling identity::do_subtracted(const identity &) const {
    return scaling{0, 0};
}

inline linearity identity::do_linear(rvgf a, rvgf b, rvgf c, rvgf d) const {
    return linearity{a, b, c, d};
}

inline affinity identity::do_affine(rvgf a, rvgf b, rvgf tx,
    rvgf c, rvgf d, rvgf ty) const {
    return affinity{a, b, tx, c, d, ty};
}

inline projectivity identity::do_projected(rvgf a, rvgf b, rvgf c,
    rvgf d, rvgf e, rvgf f, rvgf g, rvgf h, rvgf i) const {
    return projectivity{a, b, c, d, e, f, g, h, i};
}

inline affinity identity::do_windowviewport(const window &w, const viewport &v,
    e_align ax, e_align ay, e_aspect a) const {
    return make_windowviewport(w, v, ax, ay, a);
}

inline scaling identity::do_scaled(rvgf sx, rvgf sy) const {
    return scaling{sx, sy};
}

inline bool identity::do_is_equal(const identity &) const {
    return true;
}

inline bool identity::do_is_almost_equal(const identity &) const {
    return true;
}

inline identity identity::do_adjugate(void) const {
    return identity{};
}

inline bool identity::do_is_identity(void) const {
    return true;
}

inline identity identity::do_inverse(void) const {
    return identity{};
}

inline identity identity::do_transpose(void) const {
    return identity{};
}

inline rvgf identity::do_det(void) const {
    return 1.f;
}
