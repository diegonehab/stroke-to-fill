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
#ifndef RVG_SVG_PATH_COMMANDS_H
#define RVG_SVG_PATH_COMMANDS_H

#include "rvg-svg-path-token.h"

namespace rvg {
    namespace svg_path_commands {

// declares one svg_token for each SVG command
extern svg_path_token T, t, R, r, A, a, C, c, H, h, L, l, M, m, Q, q, S, s, V, v, Z;

} } // namespace rvg::svg_path_commands

#endif
