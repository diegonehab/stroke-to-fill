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
#ifndef RVG_PAINT_H
#define RVG_PAINT_H

#include "rvg-ptr.h"
#include "rvg-util.h"
#include "rvg-unorm.h"
#include "rvg-rgba.h"
#include "rvg-i-xformable.h"

#include "rvg-linear-gradient-data.h"
#include "rvg-radial-gradient-data.h"
#include "rvg-texture-data.h"

namespace rvg {

class paint final:
    public i_xformable<paint>,
    public boost::intrusive_ref_counter<paint> {

public:

    using ptr = boost::intrusive_ptr<class paint>;
    using const_ptr = boost::intrusive_ptr<const class paint>;

    enum class e_type {
        solid_color,
        linear_gradient,
        radial_gradient,
        texture,
        empty
    };

private:
    using xformable_base = i_xformable<paint>;
    using ptr_base = boost::intrusive_ref_counter<paint>;

    e_type m_type;

    unorm8 m_opacity;

    //??D Change to std::variant when we switch to C++17?
    union data {
        data() { ; }
        ~data() { ; }
        RGBA8 solid_color;
        linear_gradient_data::const_ptr linear_gradient_ptr;
        radial_gradient_data::const_ptr radial_gradient_ptr;
        texture_data::const_ptr texture_ptr;
    } m_union;

public:

    ~paint() {
        using linear_gradient_const_ptr = linear_gradient_data::const_ptr;
        using texture_const_ptr = texture_data::const_ptr;
        using radial_gradient_const_ptr = radial_gradient_data::const_ptr;

        switch (m_type) {
            case e_type::solid_color:
                m_union.solid_color.~RGBA8();
                break;
            case e_type::linear_gradient:
                m_union.linear_gradient_ptr.~linear_gradient_const_ptr();
                break;
            case e_type::radial_gradient:
                m_union.radial_gradient_ptr.~radial_gradient_const_ptr();
                break;
            case e_type::texture:
                m_union.texture_ptr.~texture_const_ptr();
                break;
            case e_type::empty:
                break;
        }
    }

    paint(): m_type(e_type::empty) { ; }

    paint(const paint &other):
        xformable_base(other),
        ptr_base(other),
        m_type(other.m_type),
        m_opacity(other.m_opacity) {
        switch (m_type) {
            case e_type::solid_color:
                new (&m_union.solid_color) RGBA8(other.
                    m_union.solid_color);
                break;
            case e_type::linear_gradient:
                new (&m_union.linear_gradient_ptr)
                    linear_gradient_data::const_ptr(other.m_union.
                        linear_gradient_ptr);
                break;
            case e_type::radial_gradient:
                new (&m_union.radial_gradient_ptr)
                    radial_gradient_data::const_ptr(other.m_union.
                        radial_gradient_ptr);
                break;
            case e_type::texture:
                new (&m_union.texture_ptr)
                    texture_data::const_ptr(other.m_union.texture_ptr);
                break;
            case e_type::empty:
                break;
        }
    }

    paint(paint &&other):
        xformable_base(std::move(other)),
        ptr_base(std::move(other)),
        m_type(std::move(other.m_type)),
        m_opacity(std::move(other.m_opacity)) {
        switch (m_type) {
            case e_type::solid_color:
                new (&m_union.solid_color)
                    RGBA8(std::move(other.m_union.solid_color));
                break;
            case e_type::linear_gradient:
                new (&m_union.linear_gradient_ptr)
                    linear_gradient_data::const_ptr(
                        std::move(other.m_union.linear_gradient_ptr));
                break;
            case e_type::radial_gradient:
                new (&m_union.radial_gradient_ptr)
                    radial_gradient_data::const_ptr(
                        std::move(other.m_union.radial_gradient_ptr));
                break;
            case e_type::texture:
                new (&m_union.texture_ptr)
                    texture_data::const_ptr(
                        std::move(other.m_union.texture_ptr));
                break;
            case e_type::empty:
                break;
        }
    }

    paint(const RGBA8 &solid_color, unorm8 opacity):
        m_type(e_type::solid_color),
        m_opacity(opacity) {
        new (&m_union.solid_color) RGBA8(solid_color);
    }

    paint(linear_gradient_data::const_ptr linear_gradient_ptr,
        unorm8 opacity):
        m_type(e_type::linear_gradient),
        m_opacity(opacity) {
        new (&m_union.linear_gradient_ptr)
            linear_gradient_data::const_ptr(linear_gradient_ptr);
    }

    paint(radial_gradient_data::const_ptr radial_gradient_ptr,
        unorm8 opacity):
        m_type(e_type::radial_gradient),
        m_opacity(opacity) {
        new (&m_union.radial_gradient_ptr)
            radial_gradient_data::const_ptr(radial_gradient_ptr);
    }

    paint(texture_data::const_ptr texture_ptr, unorm8 opacity):
        m_type(e_type::texture),
        m_opacity(opacity) {
        new (&m_union.texture_ptr) texture_data::const_ptr(texture_ptr);
    }

    paint &operator=(paint &&other) {
        if (this != &other) {
            if (m_type == other.m_type) {
                xformable_base::operator=(std::move(other));
                ptr_base::operator=(std::move(other));
                switch (m_type) {
                    case e_type::solid_color:
                        m_union.solid_color =
                            std::move(other.m_union.solid_color);
                        break;
                    case e_type::linear_gradient:
                        m_union.linear_gradient_ptr =
                            std::move(other.m_union.linear_gradient_ptr);
                        break;
                    case e_type::radial_gradient:
                        m_union.radial_gradient_ptr =
                            std::move(other.m_union.radial_gradient_ptr);
                        break;
                    case e_type::texture:
                        m_union.texture_ptr =
                            std::move(other.m_union.texture_ptr);
                        break;
                    case e_type::empty:
                        break;
                }
            } else {
                this->~paint();
                new (this) paint(std::move(other));
            }
        }
        return *this;
    }

    paint &operator=(const paint &other) {
        if (m_type == other.m_type) {
            xformable_base::operator=(other);
            ptr_base::operator=(other);
            switch (m_type) {
                case e_type::solid_color:
                    m_union.solid_color = other.m_union.solid_color;
                    break;
                case e_type::linear_gradient:
                    m_union.linear_gradient_ptr =
                        other.m_union.linear_gradient_ptr;
                    break;
                case e_type::radial_gradient:
                    m_union.radial_gradient_ptr =
                        other.m_union.radial_gradient_ptr;
                    break;
                case e_type::texture:
                    m_union.texture_ptr = other.m_union.texture_ptr;
                    break;
                case e_type::empty:
                    break;
            }
        } else {
            this->~paint();
            new (this) paint(other);
        }
        return *this;
    }

    const e_type &get_type(void) const {
        return m_type;
    }

    const char *type_name(void) const {
        switch (m_type) {
            case e_type::solid_color: return "solid_color";
            case e_type::linear_gradient: return "linear_gradient";
            case e_type::radial_gradient: return "radial_gradient";
            case e_type::texture: return "texture";
            case e_type::empty: return "empty";
            default: return "uninitialized?"; // shouldn't happen
        }
    }

    bool is_solid_color(void) const {
        return m_type == e_type::solid_color;
    }

    bool is_linear_gradient(void) const {
        return m_type == e_type::linear_gradient;
    }

    bool is_radial_gradient(void) const {
        return m_type == e_type::radial_gradient;
    }

    bool is_texture(void) const {
        return m_type == e_type::texture;
    }

    bool is_empty(void) const {
        return m_type == e_type::empty;
    }

    unorm8 get_opacity(void) const {
        return m_opacity;
    }

    RGBA8 get_solid_color(void) const {
        assert(is_solid_color());
        return m_union.solid_color;
    }

    linear_gradient_data::const_ptr get_linear_gradient_data_ptr(void) const {
        assert(is_linear_gradient());
        return m_union.linear_gradient_ptr;
    }

    const linear_gradient_data &get_linear_gradient_data(void) const {
        assert(is_linear_gradient());
        return *m_union.linear_gradient_ptr;
    }

    radial_gradient_data::const_ptr get_radial_gradient_data_ptr(void) const {
        assert(is_radial_gradient());
        return m_union.radial_gradient_ptr;
    }

    const radial_gradient_data &get_radial_gradient_data(void) const {
        assert(is_radial_gradient());
        return *m_union.radial_gradient_ptr;
    }

    texture_data::const_ptr get_texture_data_ptr(void) const {
        assert(is_texture());
        return m_union.texture_ptr;
    }

    const texture_data &get_texture_data(void) const {
        assert(is_texture());
        return *m_union.texture_ptr;
    }

};

} // namespace rvg

#endif
