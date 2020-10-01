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
#ifndef RVG_XFORM_SVD_H
#define RVG_XFORM_SVD_H

#include "rvg-xform.h"

namespace rvg {

// Computes something that is almost the SVD decomposition A
// The only difference is that we require U to be a rotation matrix.
// I.e., it must have determinant 1.
// In cases where the SVD would return U with determinant of -1, we simply
// negate the second column of U and the second line of S.
void asvd(const linearity &A, rotation &U, scaling &S, linearity &Vt);
void asvd(const linearity &A, rotation &U, scaling &S);

} // namespace rvg::xform

#endif
