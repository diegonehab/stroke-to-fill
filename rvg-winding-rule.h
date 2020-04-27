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
#ifndef RVG_WINDING_RULE_H
#define RVG_WINDING_RULE_H

#include <cstdint>

namespace rvg {

enum class e_winding_rule: uint8_t {
    non_zero,  // non-zero winding number is in
    odd,       // odd winding number is in
    zero,      // zero winding number is in (non-standard)
    even,      // even winding number is in (non-standard)
};

} // namespace rvg

#endif
