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
#include "rvg-facade-scene-data.h"

namespace rvg_facade {

struct depths { uint16_t blur, fade, clip, xform; };

static void build_stencil(const std::vector<stencil_compound> &s,
    struct depths &d, rvg::scene_data &forward);

static void build_painted(const std::vector<painted_compound> &p,
    struct depths &d, rvg::scene_data &forward);

template <typename IT>
typename std::enable_if<std::is_same<
    typename std::iterator_traits<IT>::value_type, stencil_compound>::value>::type
build_stencil(IT first, IT last, struct depths &d,
    rvg::scene_data &forward) {
    using ST = stencil_compound::e_type;
    for ( ; first != last; ++first) {
        const auto &st = *first;
        if (st.get_type() == ST::stencil_shape) {
            const auto &ss = st.get_stencil_shape();
            forward.stencil_shape(
                ss.get_winding_rule(),
                ss.get_shape().transformed(st.get_xf()));
        } else {
            // all stencil objects have their own xf
            // if the xf is not the identity, we bracket
            // around it with a transform bracket
            bool xformed = !st.get_xf().is_identity();
            if (xformed) {
                forward.begin_transform(d.xform, st.get_xf());
                ++d.xform;
            }
            switch (st.get_type()) {
                case ST::xformed:
                    // already bracketed outside
                    build_stencil(st.get_xformed().get_stencil(), d, forward);
                    break;
                case ST::clipped: {
                    const auto &clipped = st.get_clipped();
                    forward.begin_clip(d.clip++);
                    build_stencil(clipped.get_clipper(), d, forward);
                    forward.activate_clip(d.clip-1);
                    build_stencil(clipped.get_clippee(), d, forward);
                    forward.end_clip(--d.clip);
                    break;
                }
                default:
                    // do nothing
                    break;
            }
            if (xformed) {
                --d.xform;
                forward.end_transform(d.xform, st.get_xf());
            }
        }
    }
}

static void build_stencil(const std::vector<stencil_compound> &stencil,
    struct depths &depths, rvg::scene_data &forward) {
    build_stencil(stencil.begin(), stencil.end(), depths, forward);
}

template <typename IT>
typename std::enable_if<
    std::is_same<
        typename std::iterator_traits<IT>::value_type,
        painted_compound
    >::value
>::type
build_painted(IT first, IT last, struct depths &d, rvg::scene_data &forward) {
    using PT = painted_compound::e_type;
    for ( ; first != last; ++first) {
        const painted_compound &p = *first;
        switch (p.get_type()) {
            case PT::painted_shape: {
                const auto &ps = p.get_painted_shape();
                forward.painted_shape(ps.get_winding_rule(),
                    ps.get_shape().transformed(p.get_xf()),
                    ps.get_paint().transformed(p.get_xf()));
                break;
            }
            case PT::tensor_product_patch:
                forward.tensor_product_patch(
                    p.get_tensor_product_patch().transformed(p.get_xf()));
                break;
            case PT::coons_patch:
                forward.coons_patch(
                    p.get_coons_patch().transformed(p.get_xf()));
                break;
            case PT::gouraud_triangle:
                forward.gouraud_triangle(
                    p.get_gouraud_triangle().transformed(p.get_xf()));
                break;
            default:
                // all painted_compound objects have their own xf
                // if the xf is not the identity, we bracket
                // around it with a transform bracket
                bool xformed = !p.get_xf().is_identity();
                if (xformed) {
                    forward.begin_transform(d.xform, p.get_xf());
                    ++d.xform;
                }
                switch (p.get_type()) {
                    case PT::faded: {
                        const auto &faded = p.get_faded();
                        forward.begin_fade(d.fade++, faded.get_opacity());
                        build_painted(faded.get_painted(), d, forward);
                        forward.end_fade(--d.fade, faded.get_opacity());
                        break;
                    }
                    case PT::xformed: {
                        // already bracketed outside
                        build_painted(p.get_xformed().get_painted(),
                            d, forward);
                        break;
                    }
                    case PT::blurred: {
                        const auto &blurred = p.get_blurred();
                        forward.begin_blur(d.blur++, blurred.get_radius());
                        build_painted(blurred.get_painted(), d, forward);
                        forward.end_blur(--d.blur, blurred.get_radius());
                        break;
                    }
                    case PT::clipped: {
                        const auto &clipped = p.get_clipped();
                        forward.begin_clip(d.clip++);
                        build_stencil(clipped.get_clipper(), d, forward);
                        forward.activate_clip(d.clip-1);
                        build_painted(clipped.get_clippee(), d, forward);
                        forward.end_clip(--d.clip);
                        break;
                    }
                    default:
                        // do nothing
                        break;
                }
                if (xformed) {
                    --d.xform;
                    forward.end_transform(d.xform, p.get_xf());
                }
                break;
        }
    }
}

static void build_painted(const std::vector<painted_compound> &painted_compound,
    struct depths &d, rvg::scene_data &forward) {
    build_painted(painted_compound.begin(), painted_compound.end(), d, forward);
}

template <typename IT>
typename std::enable_if<
    std::is_same<
        typename std::iterator_traits<IT>::value_type,
        painted_compound
    >::value, rvg::scene_data
>::type make_scene_data(IT first, IT last) {
    rvg::scene_data s{};
    struct depths d{0,0,0,0};
    build_painted(first, last, d, s);
    return s;
}

rvg::scene_data make_scene_data(const painted_compound &p) {
    return make_scene_data({ p });
}

rvg::scene_data make_scene_data(const std::initializer_list<painted_compound> &p) {
    return make_scene_data(p.begin(), p.end());
}

rvg::scene_data make_scene_data(const std::vector<painted_compound> &p) {
    return make_scene_data(p.begin(), p.end());
}

} // namespace rvg_facade
