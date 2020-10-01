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
#ifndef RVG_STROKE_METHOD_H
#define RVG_STROKE_METHOD_H

#include <cstdint>

namespace rvg {

enum class e_stroke_method: uint8_t {
    native,
#ifdef STROKER_RVG
    rvg,
#endif
#ifdef STROKER_LIVAROT
    livarot_stroke,
    livarot_outline,
#endif
#ifdef STROKER_DIRECT2D
    direct2d,
#endif
#ifdef STROKER_QUARTZ
    quartz,
#endif
#ifdef STROKER_AGG
    agg,
#endif
#ifdef STROKER_SKIA
    skia,
#endif
#ifdef STROKER_QT5
    qt5,
#endif
#ifdef STROKER_MUPDF
    mupdf,
#endif
#ifdef STROKER_CAIRO
    cairo_traps,
    cairo_polygon,
    cairo_tristrip,
#endif
#ifdef STROKER_GS
    gs,
    gs_compat,
    gs_fast,
#endif
#ifdef STROKER_OPENVG_RI
    openvg_ri,
#endif
};

} // namespace rvg

#endif
