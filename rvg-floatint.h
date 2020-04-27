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
#ifndef RVG_FLOATINT_H
#define RVG_FLOATINT_H

#include <cstdint>

namespace rvg {

#ifdef RVG_FLOATINT64
using rvgf = double;
using rvgi = int64_t;
#else
using rvgf = float;
using rvgi = int32_t;
#endif

// This represents a float or integer
union floatint {
    floatint(rvgf ff ) {  f = ff; }
    floatint(rvgi ii) {  i = ii; }
    operator rvgf() const { return f; }
    operator rvgi() const { return i; }
    rvgf f;
    rvgi i;
};

} // namespace rvg

#endif
