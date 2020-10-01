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
#ifndef RVG_FACADE_STENCIL_COMPOUND_H
#define RVG_FACADE_STENCIL_COMPOUND_H

#include <vector>
#include "rvg-xform.h"
#include "rvg-i-xformable.h"
#include "rvg-shape.h"
#include "rvg-meta.h"
#include "rvg-winding-rule.h"
#include "rvg-stencil-shape-data.h"

namespace rvg_facade {

class stencil_compound final: public rvg::i_xformable<stencil_compound> {

    using xformable_base = rvg::i_xformable<stencil_compound>;

public:

    enum class e_type {
        stencil_shape,
        xformed,
        clipped,
        empty,
    };

    class xformed_data {

        std::vector<stencil_compound> m_stencil;

    public:
        template <typename S>
        xformed_data(S &&s,
            typename std::enable_if<
                rvg::meta::forward_same_or_convertible<S,
                    std::vector<stencil_compound>>::value
            >::type * = nullptr):
            m_stencil(std::forward<S>(s)) { ; }

        const std::vector<stencil_compound> &get_stencil(void) const {
            return m_stencil;
        }

    };

    class clipped_data {

        std::vector<stencil_compound> m_clipper;
        std::vector<stencil_compound> m_clippee;

    public:
        template <typename ER, typename EE>
        clipped_data(ER &&clipper, EE &&clippee,
            typename std::enable_if<
                rvg::meta::forward_same_or_convertible<ER,
                    std::vector<stencil_compound>>::value &&
                rvg::meta::forward_same_or_convertible<EE,
                    std::vector<stencil_compound>>::value
            >::type * = nullptr):
            m_clipper(std::forward<ER>(clipper)),
            m_clippee(std::forward<EE>(clippee)) { ; }

        const std::vector<stencil_compound> &get_clipper(void) const {
            return m_clipper;
        }

        const std::vector<stencil_compound> &get_clippee(void) const {
            return m_clippee;
        }

    };

private:

    e_type m_type;

    //??D Change to std::variant when we switch to C++17?
    union data {
        data() { ; }
        ~data() { ; }
        rvg::stencil_shape_data stencil_shape;
        xformed_data xformed;
        clipped_data clipped;
    } m_union;

public:

    stencil_compound(void): m_type(e_type::empty) { ; }

    template <typename S>
    stencil_compound(S &&sd, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<S>::type,
            rvg::stencil_shape_data
        >::value>::type * = nullptr):
        m_type(e_type::stencil_shape) {
        new (&m_union.stencil_shape) rvg::stencil_shape_data{std::forward<S>(sd)};
    }

    template <typename C>
    stencil_compound(C &&c, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<C>::type,
            clipped_data
        >::value>::type * = nullptr):
        m_type(e_type::clipped) {
        new (&m_union.clipped) clipped_data{std::forward<C>(c)};
    }

    template <typename X>
    stencil_compound(X &&t, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<X>::type,
            xformed_data
        >::value>::type * = nullptr):
        m_type(e_type::xformed) {
        new (&m_union.xformed) xformed_data{std::forward<X>(t)};
    }

    stencil_compound(stencil_compound &&other):
        xformable_base(std::move(other)),
        m_type(std::move(other.m_type)) {
        switch (m_type) {
            case e_type::stencil_shape:
                new (&m_union.stencil_shape)
                    rvg::stencil_shape_data{std::move(other.m_union.stencil_shape)};
                break;
            case e_type::xformed:
                new (&m_union.xformed)
                    xformed_data{std::move(other.m_union.xformed)};
                break;
            case e_type::clipped:
                new (&m_union.clipped)
                    clipped_data{std::move(other.m_union.clipped)};
                break;
            case e_type::empty:
                break;
        }
    }

    stencil_compound(const stencil_compound &other):
        xformable_base(other),
        m_type(other.m_type) {
        switch (m_type) {
            case e_type::stencil_shape:
                new (&m_union.stencil_shape)
                    rvg::stencil_shape_data{other.m_union.stencil_shape};
                break;
            case e_type::xformed:
                new (&m_union.xformed) xformed_data(other.m_union.xformed);
                break;
            case e_type::clipped:
                new (&m_union.clipped) clipped_data(other.m_union.clipped);
                break;
            case e_type::empty:
                break;
        }
    }

    ~stencil_compound() {
        using rvg::stencil_shape_data;
        switch (m_type) {
            case e_type::stencil_shape:
                m_union.stencil_shape.~stencil_shape_data();
                break;
            case e_type::xformed:
                m_union.xformed.~xformed_data();
                break;
            case e_type::clipped:
                m_union.clipped.~clipped_data();
                break;
            case e_type::empty:
                break;
        }
    }

    stencil_compound &operator=(stencil_compound &&other) {
        if (this != &other) {
            if (m_type == other.m_type) {
                xformable_base::operator=(std::move(other));
                switch (m_type) {
                    case e_type::stencil_shape:
                        m_union.stencil_shape =
                            std::move(other.m_union.stencil_shape);
                        break;
                    case e_type::xformed:
                        m_union.xformed = std::move(other.m_union.xformed);
                        break;
                    case e_type::clipped:
                        m_union.clipped = std::move(other.m_union.clipped);
                        break;
                    case e_type::empty:
                        break;
                }
            } else {
                this->~stencil_compound();
                new (this) stencil_compound(std::move(other));
            }
        }
        return *this;
    }

    stencil_compound &operator=(const stencil_compound &other) {
        if (m_type == other.m_type) {
            xformable_base::operator=(other);
            switch (m_type) {
                case e_type::stencil_shape:
                    m_union.stencil_shape = other.m_union.stencil_shape;
                    break;
                case e_type::xformed:
                    m_union.xformed = other.m_union.xformed;
                    break;
                case e_type::clipped:
                    m_union.clipped = other.m_union.clipped;
                    break;
                case e_type::empty:
                    break;
            }
        } else {
            this->~stencil_compound();
            new (this) stencil_compound(other);
        }
        return *this;
    }

    const e_type &get_type(void) const {
        return m_type;
    }

    bool is_stencil_shape(void) const {
        return m_type == e_type::stencil_shape;
    }

    bool is_xformed(void) const {
        return m_type == e_type::xformed;
    }

    bool is_clipped(void) const {
        return m_type == e_type::clipped;
    }

    bool is_empty(void) const {
        return m_type == e_type::empty;
    }

    const rvg::stencil_shape_data &get_stencil_shape(void) const {
        assert(is_stencil_shape());
        return m_union.stencil_shape;
    }

    const xformed_data &get_xformed(void) const {
        assert(is_xformed());
        return m_union.xformed;
    }

    const clipped_data &get_clipped(void) const {
        assert(is_clipped());
        return m_union.clipped;
    }

};

template <typename S>
stencil_compound make_stencil_compound_stencil_shape(rvg::e_winding_rule r, S &&s,
    typename std::enable_if<
        rvg::meta::forward_same_or_convertible<S, rvg::shape>::value
    >::type * = nullptr) {
	return stencil_compound{
        rvg::stencil_shape_data{r,
            rvg::make_intrusive<rvg::shape>(std::forward<S>(s)) } };
}

template <typename R, typename P>
stencil_compound make_stencil_compound_clipped(R &&clipper, P &&clippee,
    typename std::enable_if<
        rvg::meta::forward_same_or_convertible<
            R,
            std::vector<stencil_compound>
        >::value &&
        rvg::meta::forward_same_or_convertible<
            P,
            std::vector<stencil_compound>
        >::value
    >::type * = nullptr) {
	return stencil_compound{ stencil_compound::clipped_data{
        std::forward<R>(clipper), std::forward<P>(clippee)} };
}

template <typename X, typename S>
stencil_compound make_stencil_compound_xformed(X &xf, S &&s,
    typename std::enable_if<
        rvg::meta::forward_same_or_convertible<
            X,
            rvg::xform
        >::value &&
        rvg::meta::forward_same_or_convertible<
            S,
            std::vector<stencil_compound>
        >::value
    >::type * = nullptr) {
	return stencil_compound{ stencil_compound::xformed_data{
        std::forward<S>(s)} }.transformed(xf);
}

} // namespace rvg_facade

#endif
