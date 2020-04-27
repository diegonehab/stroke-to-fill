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
#ifndef RVG_XFORM_WINDOWVIEWPORT_H
#define RVG_XFORM_WINDOWVIEWPORT_H

#include "rvg-window.h"
#include "rvg-viewport.h"
#include "rvg-align.h"
#include "rvg-aspect.h"
#include "rvg-xform-affinity.h"

namespace rvg {

affinity make_windowviewport( const window &wid, const viewport &vp,
    e_align align_x = e_align::mid, e_align align_y = e_align::mid,
    e_aspect aspect = e_aspect::none);

} // namespace rvg

#endif
