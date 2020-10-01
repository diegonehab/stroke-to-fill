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
#ifndef RVG_SHAPE_H
#define RVG_SHAPE_H

#include "rvg-ptr.h"
#include "rvg-i-xformable.h"
#include "rvg-stroke-style.h"
#include "rvg-i-strokable.h"
#include "rvg-path-data.h"
#include "rvg-circle-data.h"
#include "rvg-triangle-data.h"
#include "rvg-rect-data.h"
#include "rvg-polygon-data.h"

namespace rvg {

// ??D Add ref qualifiers to all methods so we
//     can modify in-place in case the *this is an r-value ref
class shape final:
    public i_xformable<shape>,
    public i_strokable<shape>,
    public boost::intrusive_ref_counter<shape> {
public:
    using ptr = boost::intrusive_ptr<shape>;
    using const_ptr = boost::intrusive_ptr<const shape>;

private:
    using xformable_base = i_xformable<shape>;
    using strokable_base = i_strokable<shape>;
    using ptr_base = boost::intrusive_ref_counter<shape>;

public:

    enum class e_type {
        path,
        circle,
        triangle,
        rect,
        polygon,
        blend,
        stroke,
        empty,
    };

    class blend_data {
        path_data::const_ptr m_from_ptr, m_to_ptr;
        rvgf m_t;
    public:
        blend_data(path_data::const_ptr from_ptr,
            path_data::const_ptr to_ptr, rvgf t):
            m_from_ptr(from_ptr),
            m_to_ptr(to_ptr),
            m_t(t) { ; }
        const path_data &get_from(void) const { return *m_from_ptr; }
        const path_data &get_to(void) const { return *m_to_ptr; }
        path_data::const_ptr get_from_ptr(void) const {
            return m_from_ptr;
        }
        path_data::const_ptr get_to_ptr(void) const { return m_to_ptr; }
        rvgf t(void) const { return m_t; }
    };

    class stroke_data {
        const_ptr m_shape_ptr;
        float m_width;
        stroke_style::const_ptr m_style_ptr;

        static const stroke_style::const_ptr default_stroke_style_ptr;

    public:
        stroke_data(const_ptr shape_ptr, float width,
            stroke_style::const_ptr style_ptr):
            m_shape_ptr(shape_ptr),
            m_width(width),
            m_style_ptr(style_ptr) {
            if (!style_ptr) {
                m_style_ptr = default_stroke_style_ptr;
                // There is no way to know that the static
                // default_stroke_style_ptr
                // member has been initialized in rvg-shape.cpp
                // before some other module tries to construct a
                // shape object while initializing one of its
                // own static members. So we need to guard for
                // the possibility that default_stroke_style_ptr
                // is still zero-initialized here
                // ??D Maybe solve with "magic static"?
                if (!m_style_ptr) {
                    m_style_ptr = make_intrusive<stroke_style>();
                }
            }
        }
        const shape &get_shape(void) const {
            return *m_shape_ptr;
        }
        const_ptr get_shape_ptr(void) const {
            return m_shape_ptr;
        }
        const stroke_style &get_style(void) const {
            return *m_style_ptr;
        }
        stroke_style::const_ptr get_style_ptr(void) const {
            return m_style_ptr;
        }
        float get_width(void) const { return m_width; }
    };

private:

    e_type m_type;

    //??D Change to std::variant when we switch to C++17?
    union data {
        data() { ; }
        ~data() { ; }
        path_data::const_ptr path_ptr;
        circle_data::const_ptr circle_ptr;
        triangle_data::const_ptr triangle_ptr;
        rect_data::const_ptr rect_ptr;
        polygon_data::const_ptr polygon_ptr;
        blend_data blend;
        stroke_data stroke;
    } m_union;

public:

    shape(): m_type(e_type::empty) { ; }

    shape(const shape &other):
        xformable_base(other),
        strokable_base(other),
        ptr_base(other),
        m_type(other.m_type) {
        switch (m_type) {
            case e_type::path:
                new (&m_union.path_ptr)
                    path_data::const_ptr(other.m_union.path_ptr);
                break;
            case e_type::circle:
                new (&m_union.circle_ptr) circle_data::const_ptr(other.
                    m_union.circle_ptr);
                break;
            case e_type::triangle:
                new (&m_union.triangle_ptr) triangle_data::const_ptr(other.
                    m_union.triangle_ptr);
                break;
            case e_type::rect:
                new (&m_union.rect_ptr) rect_data::const_ptr(other.
                    m_union.rect_ptr);
                break;
            case e_type::polygon:
                new (&m_union.polygon_ptr) polygon_data::const_ptr(other.
                    m_union.polygon_ptr);
                break;
            case e_type::blend:
                new (&m_union.blend)
                    shape::blend_data(other.m_union.blend);
                break;
            case e_type::stroke:
                new (&m_union.stroke)
                    shape::stroke_data(other.m_union.stroke);
                break;
            case e_type::empty:
                break;
        }
    }

    shape(shape &&other):
        xformable_base(std::move(other)),
        strokable_base(std::move(other)),
        ptr_base(std::move(other)),
		m_type(std::move(other.m_type)) {
        switch (m_type) {
            case e_type::path:
                new (&m_union.path_ptr) path_data::const_ptr(std::move(other.
                    m_union.path_ptr));
                break;
            case e_type::circle:
                new (&m_union.circle_ptr) circle_data::const_ptr(
                    std::move(other.m_union.circle_ptr));
                break;
            case e_type::triangle:
                new (&m_union.triangle_ptr) triangle_data::const_ptr(
                    std::move(other.m_union.triangle_ptr));
                break;
            case e_type::rect:
                new (&m_union.rect_ptr) rect_data::const_ptr(
                    std::move(other.m_union.rect_ptr));
                break;
            case e_type::polygon:
                new (&m_union.polygon_ptr) polygon_data::const_ptr(
                    std::move(other.m_union.polygon_ptr));
                break;
            case e_type::blend:
                new (&m_union.blend)
                    shape::blend_data(std::move(other.m_union.blend));
                break;
            case e_type::stroke:
                new (&m_union.stroke)
                    shape::stroke_data(std::move(other.m_union.stroke));
                break;
            case e_type::empty:
                break;
        }
    }

    explicit shape(path_data::const_ptr path_ptr):
        m_type(e_type::path) {
        new (&m_union.path_ptr) path_data::const_ptr{path_ptr};
    }

    explicit shape(circle_data::const_ptr circle_ptr):
        m_type(e_type::circle) {
        new (&m_union.circle_ptr) circle_data::const_ptr{circle_ptr};
    }

    explicit shape(triangle_data::const_ptr triangle_ptr):
        m_type(e_type::triangle) {
        new (&m_union.triangle_ptr) triangle_data::const_ptr{triangle_ptr};
    }

    explicit shape(rect_data::const_ptr rect_ptr):
        m_type(e_type::rect) {
        new (&m_union.rect_ptr) rect_data::const_ptr{rect_ptr};
    }

    explicit shape(polygon_data::const_ptr polygon_ptr):
        m_type(e_type::polygon) {
        new (&m_union.polygon_ptr) polygon_data::const_ptr{polygon_ptr};
    }

    explicit shape(const typename shape::blend_data &blend):
        m_type(e_type::blend) {
        new (&m_union.blend) typename shape::blend_data{blend};
    }

    explicit shape(typename shape::blend_data &&blend):
        m_type(e_type::blend) {
        new (&m_union.blend) typename shape::blend_data{std::move(blend)};
    }

    explicit shape(const typename shape::stroke_data &stroke):
        m_type(e_type::stroke) {
        new (&m_union.stroke) typename shape::stroke_data{stroke};
    }

    explicit shape(typename shape::stroke_data &&stroke):
        m_type(e_type::stroke) {
        new (&m_union.stroke) typename shape::stroke_data{std::move(stroke)};
    }

    const char *type_name(void) const {
        switch (m_type) {
            case e_type::path: return "path";
            case e_type::circle: return "circle";
            case e_type::triangle: return "triangle";
            case e_type::rect: return "rect";
            case e_type::polygon: return "polygon";
            case e_type::blend: return "blend";
            case e_type::stroke: return "stroke";
            case e_type::empty: return "empty";
            default: return "uninitialized?"; // shouldn't happen
        }
    }

    ~shape() {
        using path_data_const_ptr = path_data::const_ptr;
        using circle_data_const_ptr = circle_data::const_ptr;
        using polygon_data_const_ptr = polygon_data::const_ptr;
        using rect_data_const_ptr = rect_data::const_ptr;
        using triangle_data_const_ptr = triangle_data::const_ptr;
        switch (m_type) {
            case e_type::path:
                m_union.path_ptr.~path_data_const_ptr();
                break;
            case e_type::circle:
                m_union.circle_ptr.~circle_data_const_ptr();
                break;
            case e_type::triangle:
                m_union.triangle_ptr.~triangle_data_const_ptr();
                break;
            case e_type::rect:
                m_union.rect_ptr.~rect_data_const_ptr();
                break;
            case e_type::polygon:
                m_union.polygon_ptr.~polygon_data_const_ptr();
                break;
            case e_type::blend:
                m_union.blend.~blend_data();
                break;
            case e_type::stroke:
                m_union.stroke.~stroke_data();
                break;
            case e_type::empty:
                break;
        }
    }

    shape &operator=(const shape &other) {
        if (this != &other) {
            if (m_type == other.m_type) {
                xformable_base::operator=(other);
                strokable_base::operator=(other);
                ptr_base::operator=(other);
                switch (m_type) {
                    case e_type::path:
                        m_union.path_ptr = other.m_union.path_ptr;
                        break;
                    case e_type::circle:
                        m_union.circle_ptr = other.m_union.circle_ptr;
                        break;
                    case e_type::triangle:
                        m_union.triangle_ptr = other.m_union.triangle_ptr;
                        break;
                    case e_type::rect:
                        m_union.rect_ptr = other.m_union.rect_ptr;
                        break;
                    case e_type::polygon:
                        m_union.polygon_ptr = other.m_union.polygon_ptr;
                        break;
                    case e_type::blend:
                        m_union.blend = other.m_union.blend;
                        break;
                    case e_type::stroke:
                        m_union.stroke = other.m_union.stroke;
                        break;
                    case e_type::empty:
                        break;
                }
            } else {
                this->~shape();
                new (this) shape(other);
            }
        }
        return *this;
    }

    shape &operator=(shape &&other) {
        if (m_type == other.m_type) {
            xformable_base::operator=(std::move(other));
            strokable_base::operator=(std::move(other));
            ptr_base::operator=(std::move(other));
            switch (m_type) {
                case e_type::path:
                    m_union.path_ptr = std::move(other.m_union.path_ptr);
                    break;
                case e_type::circle:
                    m_union.circle_ptr = std::move(other.m_union.circle_ptr);
                    break;
                case e_type::triangle:
                    m_union.triangle_ptr = std::move(other.
                        m_union.triangle_ptr);
                    break;
                case e_type::rect:
                    m_union.rect_ptr = std::move(other.m_union.rect_ptr);
                    break;
                case e_type::polygon:
                    m_union.polygon_ptr = std::move(other.m_union.polygon_ptr);
                    break;
                case e_type::blend:
                    m_union.blend = std::move(other.m_union.blend);
                    break;
                case e_type::stroke:
                    m_union.stroke = std::move(other.m_union.stroke);
                    break;
                case e_type::empty:
                    break;
            }
        } else {
            this->~shape();
            new (this) shape(std::move(other));
        }
        return *this;
    }

    path_data::const_ptr as_path_data_ptr(const xform &post_xf = identity{})
        const;

    //??D change e_type to type?
    e_type get_type(void) const {
        return m_type;
    }

    bool is_stroke(void) const {
        return m_type == e_type::stroke;
    }

    bool is_path(void) const {
        return m_type == e_type::path;
    }

    bool is_triangle(void) const {
        return m_type == e_type::triangle;
    }

    bool is_rect(void) const {
        return m_type == e_type::rect;
    }

    bool is_polygon(void) const {
        return m_type == e_type::polygon;
    }

    bool is_circle(void) const {
        return m_type == e_type::circle;
    }

    bool is_blend(void) const {
        return m_type == e_type::blend;
    }

    bool is_empty(void) const {
        return m_type == e_type::empty;
    }

    const blend_data &get_blend_data(void) const {
        assert(is_blend());
        return m_union.blend;
    }

    const stroke_data &get_stroke_data(void) const {
        assert(is_stroke());
        return m_union.stroke;
    }

    path_data::const_ptr get_path_data_ptr(void) const {
        assert(is_path());
        return m_union.path_ptr;
    }

    const path_data &get_path_data(void) const {
        assert(is_path());
        return *m_union.path_ptr;
    }

    circle_data::const_ptr get_circle_data_ptr(void) const {
        assert(is_circle());
        return m_union.circle_ptr;
    }

    const circle_data &get_circle_data(void) const {
        assert(is_circle());
        return *m_union.circle_ptr;
    }

    triangle_data::const_ptr get_triangle_data_ptr(void) const {
        assert(is_triangle());
        return m_union.triangle_ptr;
    }

    const triangle_data &get_triangle_data(void) const {
        assert(is_triangle());
        return *m_union.triangle_ptr;
    }

    rect_data::const_ptr get_rect_data_ptr(void) const {
        assert(is_rect());
        return m_union.rect_ptr;
    }

    const rect_data &get_rect_data(void) const {
        assert(is_rect());
        return *m_union.rect_ptr;
    }

    polygon_data::const_ptr get_polygon_data_ptr(void) const {
        assert(is_polygon());
        return m_union.polygon_ptr;
    }

    const polygon_data &get_polygon_data(void) const {
        assert(is_polygon());
        return *m_union.polygon_ptr;
    }

private:

    friend strokable_base;

    shape do_stroked(float width) const {
        // allocate a brand new style in heap
        // return a stroke version of this shape
        return shape{
            stroke_data{
                make_intrusive<shape>(*this),
                width,
                nullptr
            }
        };
    }

    shape do_stroked(float width, stroke_style::const_ptr style) const {
        // allocate a brand new style in heap
        // return a stroke version of this shape
        return shape{
            stroke_data{
                make_intrusive<shape>(*this),
                width,
                style
            }
        };
    }

    shape do_dashed(const stroke_dashes &dashes) const {
		// if current shape is not stroke, ignore
		if (is_stroke()) {
			return shape{
                stroke_data{
                    get_stroke_data().get_shape_ptr(),
                    get_stroke_data().get_width(),
                    make_intrusive<stroke_style>(get_stroke_data().
                        get_style().dashed(dashes))
                }
			};
		} else {
            return *this;
        }
    }

    shape do_capped(e_stroke_cap cap) const {
		// if current shape is not stroke, ignore
		if (is_stroke()) {
			return shape{
                stroke_data{
                    get_stroke_data().get_shape_ptr(),
                    get_stroke_data().get_width(),
                    make_intrusive<stroke_style>(get_stroke_data().
                        get_style().capped(cap))
                }
			};
		} else {
            return *this;
        }
    }

    shape do_initial_capped(e_stroke_cap cap) const {
		// if current shape is not stroke, ignore
		if (is_stroke()) {
			return shape{
                stroke_data{
                    get_stroke_data().get_shape_ptr(),
                    get_stroke_data().get_width(),
                    make_intrusive<stroke_style>(get_stroke_data().
                        get_style().initial_capped(cap))
                }
			};
		} else {
            return *this;
        }
    }

    shape do_terminal_capped(e_stroke_cap cap) const {
		// if current shape is not stroke, ignore
		if (is_stroke()) {
			return shape{
                stroke_data{
                    get_stroke_data().get_shape_ptr(),
                    get_stroke_data().get_width(),
                    make_intrusive<stroke_style>(get_stroke_data().
                        get_style().terminal_capped(cap))
                }
			};
		} else {
            return *this;
        }
    }

    shape do_dash_initial_capped(e_stroke_cap cap) const {
		// if current shape is not stroke, ignore
		if (is_stroke()) {
			return shape{
                stroke_data{
                    get_stroke_data().get_shape_ptr(),
                    get_stroke_data().get_width(),
                    make_intrusive<stroke_style>(get_stroke_data().
                        get_style().dash_initial_capped(cap))
                }
			};
		} else {
            return *this;
        }
    }

    shape do_dash_terminal_capped(e_stroke_cap cap) const {
		// if current shape is not stroke, ignore
		if (is_stroke()) {
			return shape{
                stroke_data{
                    get_stroke_data().get_shape_ptr(),
                    get_stroke_data().get_width(),
                    make_intrusive<stroke_style>(get_stroke_data().
                        get_style().dash_terminal_capped(cap))
                }
			};
		} else {
            return *this;
        }
    }

    shape do_joined(e_stroke_join join) const {
		// if current shape is not stroke, ignore
		if (is_stroke()) {
			return shape{
                stroke_data{
                    get_stroke_data().get_shape_ptr(),
                    get_stroke_data().get_width(),
                    make_intrusive<stroke_style>(get_stroke_data().
                        get_style().joined(join))
                }
			};
		} else {
            return *this;
        }
    }

    shape do_inner_joined(e_stroke_join inner_join) const {
		// if current shape is not stroke, ignore
		if (is_stroke()) {
			return shape{
                stroke_data{
                    get_stroke_data().get_shape_ptr(),
                    get_stroke_data().get_width(),
                    make_intrusive<stroke_style>(get_stroke_data().
                        get_style().inner_joined(inner_join))
                }
			};
		} else {
            return *this;
        }
    }

    shape do_miter_limited(float miter_limit) const {
		// if current shape is not stroke, ignore
		if (is_stroke()) {
			return shape{
                stroke_data{
                    get_stroke_data().get_shape_ptr(),
                    get_stroke_data().get_width(),
                    make_intrusive<stroke_style>(
                        get_stroke_data().get_style().
                            miter_limited(miter_limit))
                }
			};
		} else {
            return *this;
        }
    }

    shape do_reset_on_move(bool value) const {
		// if current shape is not stroke, ignore
		if (is_stroke()) {
			return shape{
                stroke_data{
                    get_stroke_data().get_shape_ptr(),
                    get_stroke_data().get_width(),
                    make_intrusive<stroke_style>(get_stroke_data().
                        get_style().reset_on_move(value))
                }
			};
		} else {
            return *this;
        }
    }

    shape do_dash_offset(float value) const {
		// if current shape is not stroke, ignore
		if (is_stroke()) {
			return shape{
                stroke_data{
                    get_stroke_data().get_shape_ptr(),
                    get_stroke_data().get_width(),
                    make_intrusive<stroke_style>(get_stroke_data().
                        get_style().dash_offset(value))
                }
			};
		} else {
            return *this;
        }
    }
};

} // namespace rvg

#endif
