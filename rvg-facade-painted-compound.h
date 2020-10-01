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
#ifndef RVG_FACADE_PAINTED_COMPOUND_H
#define RVG_FACADE_PAINTED_COMPOUND_H

#include <vector>
#include <type_traits>

#include "rvg-meta.h"
#include "rvg-shape.h"
#include "rvg-unorm.h"
#include "rvg-paint.h"
#include "rvg-xform.h"
#include "rvg-i-xformable.h"
#include "rvg-facade-stencil-compound.h"

namespace rvg_facade {

class painted_compound final: public rvg::i_xformable<painted_compound> {

    using xformable_base = rvg::i_xformable<painted_compound>;

public:
    enum class e_type {
        painted_shape,
        tensor_product_patch,
        coons_patch,
        gouraud_triangle,
        blurred,
        xformed,
        faded,
        clipped,
        empty
    };

    class blurred_data {

        float m_radius;
        std::vector<painted_compound> m_painted;

    public:
        template <typename P>
        blurred_data(float radius, P &&p,
            typename std::enable_if<
                rvg::meta::forward_same_or_convertible<P,
                    std::vector<painted_compound>>::value
            >::type * = nullptr):
            m_radius(radius),
            m_painted(std::forward<P>(p)) { ; }

        const std::vector<painted_compound> &get_painted(void) const {
            return m_painted;
        }

        const float &get_radius(void) const {
            return m_radius;
        }
    };

    class xformed_data {

        std::vector<painted_compound> m_painted;

    public:
        template <typename P>
        xformed_data(P &&p,
            typename std::enable_if<
                rvg::meta::forward_same_or_convertible<P,
                    std::vector<painted_compound>>::value
            >::type * = nullptr):
            m_painted(std::forward<P>(p)) { ; }

        const std::vector<painted_compound> &get_painted(void) const {
            return m_painted;
        }
    };

    class faded_data {

        rvg::unorm8 m_opacity;
        std::vector<painted_compound> m_painted;

    public:
        template <typename P>
        faded_data(int opacity, P &&p,
            typename std::enable_if<
                rvg::meta::forward_same_or_convertible<P,
                    std::vector<painted_compound>>::value
            >::type * = nullptr):
            m_opacity(rvg::unorm8::clamped(opacity)),
            m_painted(std::forward<P>(p)) { ; }

        const std::vector<painted_compound> &get_painted(void) const {
            return m_painted;
        }

        rvg::unorm8 get_opacity(void) const {
            return m_opacity;
        }
    };

    class clipped_data {

        std::vector<stencil_compound> m_clipper;
        std::vector<painted_compound> m_clippee;

    public:
        template <typename R, typename P>
        clipped_data(R &&clipper, P &&clippee,
            typename std::enable_if<
                rvg::meta::forward_same_or_convertible<R,
                    std::vector<stencil_compound>>::value &&
                rvg::meta::forward_same_or_convertible<P,
                    std::vector<painted_compound>>::value
            >::type * = nullptr):
            m_clipper(std::forward<R>(clipper)),
            m_clippee(std::forward<P>(clippee)) { ; }

        const std::vector<stencil_compound> &get_clipper(void) const {
            return m_clipper;
        }

        const std::vector<painted_compound> &get_clippee(void) const {
            return m_clippee;
        }

    };

private:

    e_type m_type;

    //??D Change to std::variant when we switch to C++17?
    union data {
        data() { ; }
        ~data() { ; }
        rvg::painted_shape_data painted_shape;
        rvg::patch<16,4> tensor_product_patch;
        rvg::patch<12,4> coons_patch;
        rvg::patch<3,3> gouraud_triangle;
        xformed_data xformed;
        faded_data faded;
        clipped_data clipped;
        blurred_data blurred;
    } m_union;


public:

    painted_compound(): m_type(e_type::empty) { ; }

    template <typename P>
    painted_compound(P &&ps, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<P>::type,
            rvg::painted_shape_data
        >::value>::type * = nullptr): m_type(e_type::painted_shape) {
        new (&m_union.painted_shape) rvg::painted_shape_data{std::forward<P>(ps)};
    }

    template <typename T>
    painted_compound(T &&tpp, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<T>::type,
            rvg::patch<16,4>
        >::value>::type * = nullptr): m_type(e_type::tensor_product_patch) {
        new (&m_union.tensor_product_patch) rvg::patch<16,4>{std::forward<T>(tpp)};
    }

    template <typename C>
    painted_compound(C &&cp, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<C>::type,
            rvg::patch<12,4>
        >::value>::type * = nullptr): m_type(e_type::coons_patch) {
        new (&m_union.coons_patch) rvg::patch<12,4>{std::forward<C>(cp)};
    }

    template <typename G>
    painted_compound(G &&gt, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<G>::type,
            rvg::patch<3,3>
        >::value>::type * = nullptr): m_type(e_type::gouraud_triangle) {
        new (&m_union.gouraud_triangle) rvg::patch<3,3>{std::forward<G>(gt)};
    }

    template <typename B>
    painted_compound(B &&b, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<B>::type,
            blurred_data
        >::value>::type * = nullptr): m_type(e_type::blurred) {
        new (&m_union.blurred) blurred_data{std::forward<B>(b)};
    }

    template <typename C>
    painted_compound(C &&c, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<C>::type,
            clipped_data
        >::value>::type * = nullptr): m_type(e_type::clipped) {
        new (&m_union.clipped) clipped_data{std::forward<C>(c)};
    }

    template <typename X>
    painted_compound(X &&t, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<X>::type,
            xformed_data
        >::value>::type * = nullptr): m_type(e_type::xformed) {
        new (&m_union.xformed) xformed_data{std::forward<X>(t)};
    }

    template <typename T>
    painted_compound(T &&t, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<T>::type,
            faded_data
        >::value>::type * = nullptr): m_type(e_type::faded) {
        new (&m_union.faded) faded_data{std::forward<T>(t)};
    }

    painted_compound(painted_compound &&other):
        xformable_base(std::move(other)),
        m_type(std::move(other.m_type)) {
        switch (m_type) {
            case e_type::painted_shape:
                new (&m_union.painted_shape)
                    rvg::painted_shape_data{std::move(other.m_union.painted_shape)};
                break;
            case e_type::tensor_product_patch:
                new (&m_union.tensor_product_patch)
                    rvg::patch<16,4>{std::move(other.m_union.tensor_product_patch)};
                break;
            case e_type::coons_patch:
                new (&m_union.coons_patch)
                    rvg::patch<12,4>{std::move(other.m_union.coons_patch)};
                break;
            case e_type::gouraud_triangle:
                new (&m_union.gouraud_triangle)
                    rvg::patch<3,3>{std::move(other.m_union.gouraud_triangle)};
                break;
            case e_type::blurred:
                new (&m_union.blurred)
                    blurred_data{std::move(other.m_union.blurred)};
                break;
            case e_type::xformed:
                new (&m_union.xformed)
                    xformed_data{std::move(other.m_union.xformed)};
                break;
            case e_type::faded:
                new (&m_union.faded)
                    faded_data{std::move(other.m_union.faded)};
                break;
            case e_type::clipped:
                new (&m_union.clipped)
                    clipped_data{std::move(other.m_union.clipped)};
                break;
            case e_type::empty:
                break;
        }
    }

    painted_compound(const painted_compound &other):
        xformable_base(other),
        m_type(other.m_type) {
        switch (m_type) {
            case e_type::painted_shape:
                new (&m_union.painted_shape)
                    rvg::painted_shape_data{other.m_union.painted_shape};
                break;
            case e_type::tensor_product_patch:
                new (&m_union.tensor_product_patch)
                    rvg::patch<16,4>{other.m_union.tensor_product_patch};
                break;
            case e_type::coons_patch:
                new (&m_union.coons_patch)
                    rvg::patch<12,4>{other.m_union.coons_patch};
                break;
            case e_type::gouraud_triangle:
                new (&m_union.gouraud_triangle)
                    rvg::patch<3,3>{other.m_union.gouraud_triangle};
                break;
            case e_type::blurred:
                new (&m_union.blurred) blurred_data{other.m_union.blurred};
                break;
            case e_type::xformed:
                new (&m_union.xformed) xformed_data{other.m_union.xformed};
                break;
            case e_type::faded:
                new (&m_union.faded) faded_data{other.m_union.faded};
                break;
            case e_type::clipped:
                new (&m_union.clipped) clipped_data{other.m_union.clipped};
                break;
            case e_type::empty:
                break;
        }
    }

    const char *get_type_name(void) const {
        switch (m_type) {
            case e_type::painted_shape: return "painted_shape";
            case e_type::tensor_product_patch: return "tensor_product_patch";
            case e_type::coons_patch: return "coons_patch";
            case e_type::gouraud_triangle: return "gouraud_triangle";
            case e_type::blurred: return "blurred";
            case e_type::xformed: return "xformed";
            case e_type::faded: return "faded";
            case e_type::clipped: return "clipped";
            case e_type::empty: return "empty";
            default: return "(unitialized?)";
        }
    }

    ~painted_compound() {
        using rvg::painted_shape_data;
        using rvg::patch;
        switch (m_type) {
            case e_type::painted_shape:
                m_union.painted_shape.~painted_shape_data();
                break;
            case e_type::tensor_product_patch:
                m_union.tensor_product_patch.~patch<16,4>();
                break;
            case e_type::coons_patch:
                m_union.coons_patch.~patch<12,4>();
                break;
            case e_type::gouraud_triangle:
                m_union.gouraud_triangle.~patch<3,3>();
                break;
            case e_type::xformed:
                m_union.xformed.~xformed_data();
                break;
            case e_type::faded:
                m_union.faded.~faded_data();
                break;
            case e_type::clipped:
                m_union.clipped.~clipped_data();
                break;
            case e_type::blurred:
                m_union.blurred.~blurred_data();
                break;
            case e_type::empty:
                break;
        }
    }

    painted_compound &operator=(painted_compound &&other) {
        if (m_type == other.m_type) {
            xformable_base::operator=(std::move(other));
            switch (m_type) {
                case e_type::painted_shape:
                    m_union.painted_shape =
                        std::move(other.m_union.painted_shape);
                    break;
                case e_type::tensor_product_patch:
                    m_union.tensor_product_patch =
                        std::move(other.m_union.tensor_product_patch);
                    break;
                case e_type::coons_patch:
                    m_union.coons_patch = std::move(other.m_union.coons_patch);
                    break;
                case e_type::gouraud_triangle:
                    m_union.gouraud_triangle =
                        std::move(other.m_union.gouraud_triangle);
                    break;
                case e_type::blurred:
                    m_union.blurred = std::move(other.m_union.blurred);
                    break;
                case e_type::xformed:
                    m_union.xformed = std::move(other.m_union.xformed);
                    break;
                case e_type::faded:
                    m_union.faded = std::move(other.m_union.faded);
                    break;
                case e_type::clipped:
                    m_union.clipped = std::move(other.m_union.clipped);
                    break;
                case e_type::empty:
                    break;
            }
        } else {
            this->~painted_compound();
            new (this) painted_compound(std::move(other));
        }
        return *this;
    }

    painted_compound &operator=(const painted_compound &other) {
        if (this != &other) {
            if (m_type == other.m_type) {
                xformable_base::operator=(other);
                switch (m_type) {
                    case e_type::painted_shape:
                        m_union.painted_shape = other.m_union.painted_shape;
                        break;
                    case e_type::tensor_product_patch:
                        m_union.tensor_product_patch =
                            other.m_union.tensor_product_patch;
                        break;
                    case e_type::coons_patch:
                        m_union.coons_patch = other.m_union.coons_patch;
                        break;
                    case e_type::gouraud_triangle:
                        m_union.gouraud_triangle =
                            other.m_union.gouraud_triangle;
                        break;
                    case e_type::blurred:
                        m_union.blurred = other.m_union.blurred;
                        break;
                    case e_type::xformed:
                        m_union.xformed = other.m_union.xformed;
                        break;
                    case e_type::faded:
                        m_union.faded = other.m_union.faded;
                        break;
                    case e_type::clipped:
                        m_union.clipped = other.m_union.clipped;
                        break;
                    case e_type::empty:
                        break;
                }
            } else {
                this->~painted_compound();
                new (this) painted_compound(other);
            }
        }
        return *this;
    }

    const e_type &get_type(void) const {
        return m_type;
    }

    bool is_painted_shape(void) const {
        return m_type == e_type::painted_shape;
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

    bool is_xformed(void) const {
        return m_type == e_type::xformed;
    }

    bool is_blurred(void) const {
        return m_type == e_type::blurred;
    }

    bool is_faded(void) const {
        return m_type == e_type::faded;
    }

    bool is_clipped(void) const {
        return m_type == e_type::clipped;
    }

    bool is_empty(void) const {
        return m_type == e_type::empty;
    }

    const rvg::painted_shape_data &get_painted_shape(void) const {
        assert(is_painted_shape());
        return m_union.painted_shape;
    }

    const rvg::patch<16,4> &get_tensor_product_patch(void) const {
        assert(is_tensor_product_patch());
        return m_union.tensor_product_patch;
    }

    const rvg::patch<12,4> &get_coons_patch(void) const {
        assert(is_coons_patch());
        return m_union.coons_patch;
    }

    const rvg::patch<3,3> &get_gouraud_triangle(void) const {
        assert(is_gouraud_triangle());
        return m_union.gouraud_triangle;
    }

    const xformed_data &get_xformed(void) const {
        assert(is_xformed());
        return m_union.xformed;
    }

    const blurred_data &get_blurred(void) const {
        assert(is_blurred());
        return m_union.blurred;
    }

    const faded_data &get_faded(void) const {
        assert(is_faded());
        return m_union.faded;
    }

    const clipped_data &get_clipped(void) const {
        assert(is_clipped());
        return m_union.clipped;
    }
};

template <typename P>
painted_compound make_painted_compound_blurred(float r, P &&p,
    typename std::enable_if<
        std::is_same<
            typename std::remove_reference<P>::type::value_type,
            painted_compound
        >::value
    >::type * = nullptr) {
	return painted_compound{
        painted_compound::blurred_data{r, std::forward<P>(p)} };
}

template <typename P>
painted_compound make_painted_compound_xformed(const rvg::xform &xf, P &&p,
    typename std::enable_if<
        rvg::meta::forward_same_or_convertible<
            P,
            std::vector<painted_compound>
        >::value
    >::type * = nullptr) {
	return painted_compound{
        painted_compound::xformed_data{std::forward<P>(p)} }.transformed(xf);
}

template <typename P>
painted_compound make_painted_compound_faded(int opacity, P &&p,
    typename std::enable_if<
        rvg::meta::forward_same_or_convertible<
            P,
            std::vector<painted_compound>
        >::value
    >::type * = nullptr) {
	return painted_compound{
        painted_compound::faded_data{opacity, std::forward<P>(p)} };
}

template <typename S, typename P>
painted_compound make_painted_compound_painted_shape(rvg::e_winding_rule w, S &&s,
    P &&p, typename std::enable_if<
        rvg::meta::forward_same_or_convertible<S, rvg::shape>::value &&
        rvg::meta::forward_same_or_convertible<P, rvg::paint>::value
    >::type * = nullptr) {
	return painted_compound{
        rvg::painted_shape_data{w,
            rvg::make_intrusive<rvg::shape>(std::forward<S>(s)),
            rvg::make_intrusive<rvg::paint>(std::forward<P>(p)) } };
}

template <typename T>
painted_compound make_painted_compound_tensor_product_patch(T && tpp,
    typename std::enable_if<
        rvg::meta::forward_same_or_convertible<T, rvg::patch<16,4>>::value
    >::type * = nullptr) {
	return painted_compound{ std::forward<T>(tpp) };
}

template <typename C>
painted_compound make_painted_compound_coons_patch(C && cp,
    typename std::enable_if<
        rvg::meta::forward_same_or_convertible<C, rvg::patch<12,4>>::value
    >::type * = nullptr) {
	return painted_compound{ std::forward<C>(cp) };
}

template <typename G>
painted_compound make_painted_gouraud_triangle(G && gt,
    typename std::enable_if<
        rvg::meta::forward_same_or_convertible<G, rvg::patch<3,3>>::value
    >::type * = nullptr) {
	return painted_compound{ std::forward<G>(gt) };
}

template <typename R, typename P>
painted_compound make_painted_compound_clipped(R &&clipper, P &&clippee,
    typename std::enable_if<
        rvg::meta::forward_same_or_convertible<
            R,
            std::vector<stencil_compound>
        >::value &&
        rvg::meta::forward_same_or_convertible<
            P,
            std::vector<painted_compound>
        >::value
    >::type * = nullptr) {
	return painted_compound{
        painted_compound::clipped_data{ std::forward<R>(clipper),
		std::forward<P>(clippee)} };
}

} // namespace rvg_facade

#endif
