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
#ifndef RVG_STROKER_MUPDF_H
#define RVG_STROKER_MUPDF_H

#include <vector>
#include "rvg-stroke-style.h"
#include "rvg-shape.h"

namespace rvg {
    namespace stroker {

shape mupdf(const shape &input_shape, const xform &screen_xf, float width,
    stroke_style::const_ptr style);

} }

#endif
