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
#ifndef RVG_SCENE_ELEMENT_H
#define RVG_SCENE_ELEMENT_H

#include "rvg-painted-shape-data.h"
#include "rvg-stencil-shape-data.h"

namespace rvg {

class scene_element {

public:

    enum class e_type: uint8_t {
        painted_shape,
        stencil_shape,
        tensor_product_patch,
        coons_patch,
        gouraud_triangle,
		empty
    };

private:

    using TPP = patch<16,4>; // tensor product patch
    using CP = patch<12,4>;  // coons patch
    using GT = patch<3,3>;   // gouraud triangle

    e_type m_type;

    //??D Change to std::variant when we switch to C++17?
    union data {
        data() { ; }
        ~data() { ; }
        painted_shape_data painted_shape;
        stencil_shape_data stencil_shape;
        TPP::const_ptr tensor_product_patch_ptr;
        CP::const_ptr coons_patch_ptr;
        GT::const_ptr gouraud_triangle_ptr;
    } m_union;

public:

    scene_element(): m_type{e_type::empty} { ; }

    scene_element(const scene_element &other):
        m_type{other.m_type} {
        switch (m_type) {
            case e_type::painted_shape:
                new (&m_union.painted_shape)
                    painted_shape_data{other.m_union.painted_shape};
                break;
            case e_type::stencil_shape:
                new (&m_union.stencil_shape)
                    stencil_shape_data{other.m_union.stencil_shape};
                break;
            case e_type::tensor_product_patch:
                new (&m_union.tensor_product_patch_ptr)
                    TPP::const_ptr{other.m_union.tensor_product_patch_ptr};
                break;
            case e_type::coons_patch:
                new (&m_union.coons_patch_ptr)
                    CP::const_ptr{other.m_union.coons_patch_ptr};
                break;
            case e_type::gouraud_triangle:
                new (&m_union.gouraud_triangle_ptr)
                    GT::const_ptr{other.m_union.gouraud_triangle_ptr};
                break;
            case e_type::empty:
                break;
        }
    }

    scene_element(scene_element &&other):
        m_type{other.m_type} {
        switch (m_type) {
            case e_type::painted_shape:
                new (&m_union.painted_shape) painted_shape_data{
                    std::move(other.m_union.painted_shape)};
                break;
            case e_type::stencil_shape:
                new (&m_union.stencil_shape)
                    stencil_shape_data{std::move(other.m_union.stencil_shape)};
                break;
            case e_type::tensor_product_patch:
                new (&m_union.tensor_product_patch_ptr)
                    TPP::const_ptr{std::move(other.
                        m_union.tensor_product_patch_ptr)};
                break;
            case e_type::coons_patch:
                new (&m_union.coons_patch_ptr)
                    CP::const_ptr{std::move(other.m_union.coons_patch_ptr)};
                break;
            case e_type::gouraud_triangle:
                new (&m_union.gouraud_triangle_ptr)
                    GT::const_ptr{std::move(other.
                        m_union.gouraud_triangle_ptr)};
                break;
            case e_type::empty:
                break;
        }
    }

    explicit scene_element(const painted_shape_data &data):
        m_type{e_type::painted_shape} {
        new (&m_union.painted_shape) painted_shape_data{data};
    }

    explicit scene_element(const stencil_shape_data &data):
        m_type{e_type::stencil_shape} {
        new (&m_union.stencil_shape) stencil_shape_data{data};
    }

    explicit scene_element(TPP::const_ptr ptr):
        m_type{e_type::tensor_product_patch} {
        new (&m_union.tensor_product_patch_ptr) TPP::const_ptr{ptr};
    }

    explicit scene_element(CP::const_ptr ptr):
        m_type{e_type::coons_patch} {
        new (&m_union.coons_patch_ptr) CP::const_ptr{ptr};
    }

    explicit scene_element(GT::const_ptr ptr):
        m_type{e_type::gouraud_triangle} {
        new (&m_union.gouraud_triangle_ptr) GT::const_ptr{ptr};
    }

    const char *type_name(void) const {
        switch (m_type) {
            case e_type::painted_shape: return "painted shape";
            case e_type::stencil_shape: return "stencil shape";
            case e_type::tensor_product_patch: return "tensor product patch";
            case e_type::coons_patch: return "coons patch";
            case e_type::gouraud_triangle: return "gouraud triangle";
            case e_type::empty: return "empty";
            default: return "uninitialized?"; // shouldn't happen
        }
    }

    ~scene_element() {
		using tensor_product_patch_const_ptr = TPP::const_ptr;
		using coons_patch_const_ptr = CP::const_ptr;
		using gouraud_triangle_const_ptr = GT::const_ptr;
        switch (m_type) {
            case e_type::painted_shape:
                m_union.painted_shape.~painted_shape_data();
                break;
            case e_type::stencil_shape:
                m_union.stencil_shape.~stencil_shape_data();
                break;
            case e_type::tensor_product_patch:
                m_union.tensor_product_patch_ptr.
					~tensor_product_patch_const_ptr();
                break;
            case e_type::coons_patch:
                m_union.coons_patch_ptr.~coons_patch_const_ptr();
                break;
            case e_type::gouraud_triangle:
                m_union.gouraud_triangle_ptr.~gouraud_triangle_const_ptr();
                break;
            case e_type::empty:
                break;
        }
    }

    scene_element &operator=(const scene_element &other) {
        if (this != &other) {
            if (m_type == other.m_type) {
                switch (m_type) {
                    case e_type::painted_shape:
                        m_union.painted_shape = other.m_union.painted_shape;
                        break;
                    case e_type::stencil_shape:
                        m_union.stencil_shape = other.m_union.stencil_shape;
                        break;
                    case e_type::tensor_product_patch:
                        m_union.tensor_product_patch_ptr =
							other.m_union.tensor_product_patch_ptr;
                        break;
                    case e_type::coons_patch:
                        m_union.coons_patch_ptr = other.m_union.coons_patch_ptr;
                        break;
                    case e_type::gouraud_triangle:
                        m_union.gouraud_triangle_ptr =
							other.m_union.gouraud_triangle_ptr;
                        break;
                    case e_type::empty:
                        break;
                }
            } else {
                this->~scene_element();
                new (this) scene_element(other);
            }
        }
        return *this;
    }

    scene_element &operator=(scene_element &&other) {
        if (this != &other) {
            if (m_type == other.m_type) {
                switch (m_type) {
                    case e_type::painted_shape:
                        m_union.painted_shape =
							std::move(other.m_union.painted_shape);
                        break;
                    case e_type::stencil_shape:
                        m_union.stencil_shape =
							std::move(other.m_union.stencil_shape);
                        break;
                    case e_type::tensor_product_patch:
                        m_union.tensor_product_patch_ptr =
							std::move(other.m_union.tensor_product_patch_ptr);
                        break;
                    case e_type::coons_patch:
                        m_union.coons_patch_ptr =
                            std::move(other.m_union.coons_patch_ptr);
                        break;
                    case e_type::gouraud_triangle:
                        m_union.gouraud_triangle_ptr =
							std::move(other.m_union.gouraud_triangle_ptr);
                        break;
                    case e_type::empty:
                        break;
                }
            } else {
                this->~scene_element();
                new (this) scene_element(std::move(other));
            }
        }
        return *this;
    }

    e_type get_type(void) const {
        return m_type;
    }

    bool is_empty(void) const {
        return m_type == e_type::empty;
    }

    bool is_painted_shape(void) const {
        return m_type == e_type::painted_shape;
    }

    bool is_stencil_shape(void) const {
        return m_type == e_type::stencil_shape;
    }

    bool is_tensor_product_patch(void) const {
        return m_type == e_type::tensor_product_patch;
    }

    bool is_coons_patch(void) const {
        return m_type == e_type::coons_patch;
    }

    bool is_gouraud_triangle(void) const {
        return m_type == e_type::gouraud_triangle;
    }

    const painted_shape_data &get_painted_shape_data(void) const {
        assert(is_painted_shape());
        return m_union.painted_shape;
    }

    const stencil_shape_data &get_stencil_shape_data(void) const {
        assert(is_stencil_shape());
        return m_union.stencil_shape;
    }

    TPP::const_ptr get_tensor_product_patch_ptr(void) const {
        assert(is_tensor_product_patch());
        return m_union.tensor_product_patch_ptr;
    }

    const TPP &get_tensor_product_patch(void) const {
        assert(is_tensor_product_patch());
        return *m_union.tensor_product_patch_ptr;
    }

    CP::const_ptr get_coons_patch_ptr(void) const {
        assert(is_coons_patch());
        return m_union.coons_patch_ptr;
    }

    const CP &get_coons_patch(void) const {
        assert(is_coons_patch());
        return *m_union.coons_patch_ptr;
    }

    GT::const_ptr get_gouraud_triangle_ptr(void) const {
        assert(is_gouraud_triangle());
        return m_union.gouraud_triangle_ptr;
    }

    const GT &get_gouraud_triangle(void) const {
        assert(is_gouraud_triangle());
        return *m_union.gouraud_triangle_ptr;
    }

};

} // namespace rvg


#endif
