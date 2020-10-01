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
#ifndef RVG_WINDOW_H
#define RVG_WINDOW_H

#include "rvg-bbox.h"
#include "rvg-floatint.h"

namespace rvg {

using window = bbox<rvgf>;

static inline window make_window(rvgf xl, rvgf yb, rvgf xr, rvgf yt) {
    return window(xl, yb, xr, yt);
}

} // namespace rvg

#endif
