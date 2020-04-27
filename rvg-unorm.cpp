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
#include "rvg-unorm.h"

namespace rvg {

const uint8_t gamma_addition_uint8_t[] = {
    0, 13, 22, 28, 34, 38, 42, 46, 50, 53, 56, 59, 61, 64, 66,
    69, 71, 73, 75, 77, 79, 81, 83, 85, 86, 88, 90, 92, 93, 95,
    96, 98, 99, 101, 102, 104, 105, 106, 108, 109, 110, 112,
    113, 114, 115, 117, 118, 119, 120, 121, 122, 124, 125, 126,
    127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
    139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 148, 149,
    150, 151, 152, 153, 154, 155, 155, 156, 157, 158, 159, 159,
    160, 161, 162, 163, 163, 164, 165, 166, 167, 167, 168, 169,
    170, 170, 171, 172, 173, 173, 174, 175, 175, 176, 177, 178,
    178, 179, 180, 180, 181, 182, 182, 183, 184, 185, 185, 186,
    187, 187, 188, 189, 189, 190, 190, 191, 192, 192, 193, 194,
    194, 195, 196, 196, 197, 197, 198, 199, 199, 200, 200, 201,
    202, 202, 203, 203, 204, 205, 205, 206, 206, 207, 208, 208,
    209, 209, 210, 210, 211, 212, 212, 213, 213, 214, 214, 215,
    215, 216, 216, 217, 218, 218, 219, 219, 220, 220, 221, 221,
    222, 222, 223, 223, 224, 224, 225, 226, 226, 227, 227, 228,
    228, 229, 229, 230, 230, 231, 231, 232, 232, 233, 233, 234,
    234, 235, 235, 236, 236, 237, 237, 238, 238, 238, 239, 239,
    240, 240, 241, 241, 242, 242, 243, 243, 244, 244, 245, 245,
    246, 246, 246, 247, 247, 248, 248, 249, 249, 250, 250, 251,
    251, 251, 252, 252, 253, 253, 254, 254, 255, 255
};

const uint8_t gamma_removal_uint8_t[] = {
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2,
    2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
    5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11,
    11, 12, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17,
    17, 18, 18, 19, 19, 20, 20, 21, 22, 22, 23, 23, 24, 24, 25,
    25, 26, 27, 27, 28, 29, 29, 30, 30, 31, 32, 32, 33, 34, 35,
    35, 36, 37, 37, 38, 39, 40, 41, 41, 42, 43, 44, 45, 45, 46,
    47, 48, 49, 50, 51, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 76,
    77, 78, 79, 80, 81, 82, 84, 85, 86, 87, 88, 90, 91, 92, 93,
    95, 96, 97, 99, 100, 101, 103, 104, 105, 107, 108, 109, 111,
    112, 114, 115, 116, 118, 119, 121, 122, 124, 125, 127, 128,
    130, 131, 133, 134, 136, 138, 139, 141, 142, 144, 146, 147,
    149, 151, 152, 154, 156, 157, 159, 161, 163, 164, 166, 168,
    170, 171, 173, 175, 177, 179, 181, 183, 184, 186, 188, 190,
    192, 194, 196, 198, 200, 202, 204, 206, 208, 210, 212, 214,
    216, 218, 220, 222, 224, 226, 229, 231, 233, 235, 237, 239,
    242, 244, 246, 248, 250, 253, 255,
};

} // namespace rvg
