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
#ifndef RVG_DRIVER_QT5_H
#define RVG_DRIVER_QT5_H

#include <cstdio>
#include <string>
#include <vector>

#include "rvg-window.h"
#include "rvg-viewport.h"
#include "rvg-scene.h"

namespace rvg {
    namespace driver {
        namespace qt5 {

const scene &accelerate(const scene &c, const window &w,
    const viewport &v);

void render(const scene &c, const window &w, const viewport &v,
    FILE *out, const std::vector<std::string> &args =
        std::vector<std::string>());

} } } // namespace rvg::driver::qt5

#endif
