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
#ifndef RVG_FREETYPE_H
#define RVG_FREETYPE_H

#include <memory>
#include <type_traits>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "rvg-freetype-typeface.h"

namespace rvg {

class freetype final {

    using library_ptr =
        std::shared_ptr<typename std::remove_pointer<FT_Library>::type>;

    library_ptr m_library;

public:

    freetype(void);

    freetype(library_ptr library);

    int get_memory_num_faces(const uint8_t *base, size_t size);

    int get_num_faces(const char *filename);

    freetype_typeface::ptr new_face(const char *filename, int face_index = 0);

    freetype_typeface::ptr new_memory_face(const uint8_t *base, size_t size,
        int face_index = 0);
};

}

#endif
