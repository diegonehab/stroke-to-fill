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
inline xform make_identity(void) {
    return static_cast<projectivity>(identity{});
}

inline xform make_rotation(rvgf deg) {
    return static_cast<projectivity>(rotation{deg});
}

inline xform make_rotation(rvgf deg, rvgf cx, rvgf cy) {
    return static_cast<projectivity>(
        translation{-cx, -cy}.rotated(deg).translated(cx, cy));
}

inline xform make_translation(rvgf tx, rvgf ty) {
    return static_cast<projectivity>(translation{tx, ty});
}

inline xform make_scaling(rvgf sx, rvgf sy, rvgf cx, rvgf cy) {
    return static_cast<projectivity>(
        translation{-cx, -cy}.scaled(sx, sy).translated(cx, cy));
}

inline xform make_scaling(rvgf s, rvgf cx, rvgf cy) {
    return static_cast<projectivity>(
        translation{-cx, -cy}.scaled(s).translated(cx, cy));
}

inline xform make_scaling(rvgf sx, rvgf sy) {
    return static_cast<projectivity>(scaling{sx, sy});
}

inline xform make_scaling(rvgf s) {
    return static_cast<projectivity>(scaling{s, s});
}

inline xform make_linearity(rvgf a, rvgf b, rvgf c, rvgf d) {
    return static_cast<projectivity>(linearity{a, b, c, d});
}

inline xform make_affinity(rvgf a, rvgf b, rvgf tx, rvgf c, rvgf d, rvgf ty) {
    return static_cast<projectivity>(affinity{a, b, tx, c, d, ty});
}

inline xform make_projectivity(rvgf a, rvgf b, rvgf c,
    rvgf d, rvgf e, rvgf f, rvgf g, rvgf h, rvgf i) {
    return projectivity{a, b, c, d, e, f, g, h, i};
}
