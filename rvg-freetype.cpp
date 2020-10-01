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
#include "rvg-freetype.h"

namespace rvg {

freetype::freetype(void) {
    FT_Library library;
    if (!FT_Init_FreeType(&library)) {
        m_library = library_ptr(library,
            [](FT_Library lib) {
                 FT_Done_FreeType(lib);
            }
        );
    }
}

freetype::freetype(library_ptr library):
    m_library(library) {
    ;
}

int freetype::get_num_faces(const char *filename) {
    FT_Face face;
    if (FT_New_Face(m_library.get(), filename, -1, &face)) {
        return 0;
    }
    int num_faces = face->num_faces;
    FT_Done_Face(face);
    return num_faces;
}

int freetype::get_memory_num_faces(const uint8_t *base, size_t size) {
    FT_Face face;
    if (FT_New_Memory_Face(m_library.get(), base, (FT_Long)size, -1, &face)) {
        return 0;
    }
    int num_faces = face->num_faces;
    FT_Done_Face(face);
    return num_faces;
}

freetype_typeface::ptr
freetype::new_face(const char *filename, int face_index) {
    FT_Face face;
    if (FT_New_Face(m_library.get(), filename, face_index, &face)) {
        return nullptr;
    }
    if (!FT_IS_SCALABLE(face) || FT_IS_TRICKY(face)) {
        return nullptr;
    }
    FT_Set_Char_Size(face, 0, 0, 0, 0); // dummy call
    return make_intrusive<freetype_typeface>(face);
}

freetype_typeface::ptr
freetype::new_memory_face(const uint8_t *base, size_t size, int face_index) {
    FT_Face face;
    if (FT_New_Memory_Face(m_library.get(), base, (FT_Long) size, face_index, &face)) {
        return nullptr;
    }
    if (!FT_IS_SCALABLE(face) || FT_IS_TRICKY(face)) {
        return nullptr;
    }
    FT_Set_Char_Size(face, 0, 0, 0, 0); // dummy call
    return make_intrusive<freetype_typeface>(face);
}

}
