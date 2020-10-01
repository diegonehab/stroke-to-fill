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
#ifndef RVG_SCENE_DATA_H
#define RVG_SCENE_DATA_H

#include <cstdint>

#include "rvg-ptr.h"
#include "rvg-i-scene-data.h"
#include "rvg-xform.h"
#include "rvg-patch.h"
#include "rvg-scene-element.h"
#include "rvg-scene-bracket.h"

namespace rvg {

class scene_data final:
    public i_scene_data<scene_data>,
    public boost::intrusive_ref_counter<scene_data> {

    std::vector<scene_bracket> m_brackets;
    std::vector<scene_bracket::element_id> m_matching_clip_bracket;
    std::vector<scene_element> m_elements;

public:

    using ptr = boost::intrusive_ptr<scene_data>;
    using const_ptr = boost::intrusive_ptr<const scene_data>;

    template <typename SF>
    void iterate_brackets(SF &forward) const {
        for (const auto &b: m_brackets) {
            using BT = typename scene_bracket::e_type;
            switch (b.get_type()) {
                case BT::begin_clip:
                    forward.begin_clip(b.get_depth());
                    break;
                case BT::activate_clip:
                    forward.activate_clip(b.get_depth());
                    break;
                case BT::end_clip:
                    forward.end_clip(b.get_depth());
                    break;
                case BT::begin_fade:
                    forward.begin_fade(b.get_depth(),
                        b.get_begin_fade_data().opacity);
                    break;
                case BT::end_fade:
                    forward.end_fade(b.get_depth(),
                        b.get_end_fade_data().opacity);
                    break;
                case BT::begin_blur:
                    forward.begin_blur(b.get_depth(),
                        b.get_begin_blur_data().radius);
                    break;
                case BT::end_blur:
                    forward.end_blur(b.get_depth(),
                        b.get_end_blur_data().radius);
                    break;
                case BT::begin_transform:
                    forward.begin_transform(b.get_depth(),
                        b.get_begin_transform_data().xf);
                    break;
                case BT::end_transform:
                    forward.end_transform(b.get_depth(),
                        b.get_end_transform_data().xf);
                    break;
                case BT::empty:
                    break;
            }
        }
    }

    template <typename SF>
    void iterate_brackets(SF &&forward) const {
        this->iterate_brackets(forward);
    }

    template <typename SF>
    void riterate_brackets(SF &forward, size_t bracket_index,
        size_t min_bracket_index) const {
        while (bracket_index != min_bracket_index) {
            const auto &b = m_brackets[bracket_index-1];
            using BT = typename scene_bracket::e_type;
            if (b.get_type() == BT::end_clip) {
                auto end_index = bracket_index-1;
                auto activate_index = b.get_end_clip_data().matching_bracket;
                auto begin_index = m_brackets[activate_index].
                    get_activate_clip_data().matching_bracket;
                forward.begin_clip(b.get_depth());
                this->riterate_brackets(forward, activate_index, begin_index+1);
                forward.activate_clip(b.get_depth());
                this->riterate_brackets(forward, end_index, activate_index+1); 
                forward.end_clip(b.get_depth());
                bracket_index = begin_index;
            } else {
                switch (b.get_type()) {
                    case BT::begin_clip:
                    case BT::activate_clip:
                    case BT::end_clip:
                        assert(0); // can't happen
                        break;
                    case BT::begin_fade:
                        forward.end_fade(b.get_depth(),
                            b.get_begin_fade_data().opacity);
                        break;
                    case BT::end_fade:
                        forward.begin_fade(b.get_depth(),
                            b.get_end_fade_data().opacity);
                        break;
                    case BT::begin_blur:
                        forward.end_blur(b.get_depth(),
                        b.get_begin_blur_data().radius);
                        break;
                    case BT::end_blur:
                        forward.begin_blur(b.get_depth(),
                            b.get_end_blur_data().radius);
                        break;
                    case BT::begin_transform:
                        forward.end_transform(b.get_depth(),
                            b.get_begin_transform_data().xf);
                        break;
                    case BT::end_transform:
                        forward.begin_transform(b.get_depth(),
                            b.get_end_transform_data().xf);
                        break;
                    case BT::empty:
                        break;
                }
                --bracket_index;
            }
        }
    }

    template <typename SF>
    void riterate_brackets(SF &forward) const {
        this->riterate_brackets(forward, m_brackets.size(), 0);
    }

    template <typename SF>
    void riterate_brackets(SF &&forward) const {
        this->riterate_brackets(forward);
    }

    template <typename SF>
    void iterate(SF &forward) const {
        std::size_t bracket_index = 0, element_index = 0;
        while (element_index < m_elements.size() ||
               bracket_index < m_brackets.size()) {
            if (bracket_index < m_brackets.size() &&
                m_brackets[bracket_index].get_element_id() <= element_index) {
                const auto &b = m_brackets[bracket_index];
                using BT = typename scene_bracket::e_type;
                switch (b.get_type()) {
                    case BT::begin_clip:
                        forward.begin_clip(b.get_depth());
                        break;
                    case BT::activate_clip:
                        forward.activate_clip(b.get_depth());
                        break;
                    case BT::end_clip:
                        forward.end_clip(b.get_depth());
                        break;
                    case BT::begin_fade:
                        forward.begin_fade(b.get_depth(),
                            b.get_begin_fade_data().opacity);
                        break;
                    case BT::end_fade:
                        forward.end_fade(b.get_depth(),
                            b.get_end_fade_data().opacity);
                        break;
                    case BT::begin_blur:
                        forward.begin_blur(b.get_depth(),
                            b.get_begin_blur_data().radius);
                        break;
                    case BT::end_blur:
                        forward.end_blur(b.get_depth(),
                            b.get_end_blur_data().radius);
                        break;
                    case BT::begin_transform:
                        forward.begin_transform(b.get_depth(),
                            b.get_begin_transform_data().xf);
                        break;
                    case BT::end_transform:
                        forward.end_transform(b.get_depth(),
                            b.get_end_transform_data().xf);
                        break;
                    case BT::empty:
                        break;
                }
                ++bracket_index;
            } else if (element_index < m_elements.size()) {
                const auto &e = m_elements[element_index];
                using ET = typename scene_element::e_type;
                switch (e.get_type()) {
                    case ET::painted_shape: {
                        const auto &p = e.get_painted_shape_data();
                        forward.painted_shape(p.get_winding_rule(),
                            p.get_shape(), p.get_paint());
                        break;
                    }
                    case ET::stencil_shape: {
                        const auto &p = e.get_stencil_shape_data();
                        forward.stencil_shape(p.get_winding_rule(),
                            p.get_shape());
                        break;
                    }
                    case ET::tensor_product_patch:
                        forward.tensor_product_patch(
                            e.get_tensor_product_patch());
                        break;
                    case ET::coons_patch:
                        forward.coons_patch(e.get_coons_patch());
                        break;
                    case ET::gouraud_triangle:
                        forward.gouraud_triangle(e.get_gouraud_triangle());
                        break;
                    case ET::empty:
                        break;
                }
                ++element_index;
            } else {
                break;
            }
        }
    }

    template <typename SF>
    void iterate(SF &&forward) const {
        this->iterate(forward);
    }

    template <typename SF>
    void riterate(SF &forward, size_t bracket_index, size_t min_bracket_index,
        size_t element_index, size_t min_element_index) const {
        while (element_index != min_element_index ||
               bracket_index != min_bracket_index) {
            if (bracket_index != min_bracket_index &&
                m_brackets[bracket_index-1].get_element_id() >= element_index) {
                const auto &b = m_brackets[bracket_index-1];
                using BT = typename scene_bracket::e_type;
                if (b.get_type() == BT::end_clip) {
                    auto end_index = bracket_index-1;
                    auto end_element_index = b.get_element_id();
                    auto activate_index = b.get_end_clip_data().
                        matching_bracket;
                    auto activate_element_index = m_brackets[activate_index].
                        get_element_id();
                    auto begin_index = m_brackets[activate_index].
                        get_activate_clip_data().matching_bracket;
                    auto begin_element_index = m_brackets[begin_index].
                        get_element_id();
                    forward.begin_clip(b.get_depth());
                    this->riterate(forward, activate_index, begin_index+1,
                        activate_element_index, begin_element_index);
                    forward.activate_clip(b.get_depth());
                    this->riterate(forward, end_index, activate_index+1,
                        end_element_index, activate_element_index);
                    forward.end_clip(b.get_depth());
                    bracket_index = begin_index;
                    element_index = begin_element_index;
                } else {
                    switch (b.get_type()) {
                        case BT::begin_clip:
                        case BT::activate_clip:
                        case BT::end_clip:
                            assert(0); // can't happen
                            break;
                        case BT::begin_fade:
                            forward.end_fade(b.get_depth(),
                                b.get_begin_fade_data().opacity);
                            break;
                        case BT::end_fade:
                            forward.begin_fade(b.get_depth(),
                                b.get_end_fade_data().opacity);
                            break;
                        case BT::begin_blur:
                            forward.end_blur(b.get_depth(),
                            b.get_begin_blur_data().radius);
                            break;
                        case BT::end_blur:
                            forward.begin_blur(b.get_depth(),
                                b.get_end_blur_data().radius);
                            break;
                        case BT::begin_transform:
                            forward.end_transform(b.get_depth(),
                                b.get_begin_transform_data().xf);
                            break;
                        case BT::end_transform:
                            forward.begin_transform(b.get_depth(),
                                b.get_end_transform_data().xf);
                            break;
                        case BT::empty:
                            break;
                    }
                    --bracket_index;
                }
            } else if (element_index != min_element_index) {
                const auto &e = m_elements[element_index-1];
                using ET = typename scene_element::e_type;
                switch (e.get_type()) {
                    case ET::painted_shape: {
                        const auto &p = e.get_painted_shape_data();
                        forward.painted_shape(p.get_winding_rule(),
                            p.get_shape(), p.get_paint());
                        break;
                    }
                    case ET::stencil_shape: {
                        const auto &p = e.get_stencil_shape_data();
                        forward.stencil_shape(p.get_winding_rule(),
                            p.get_shape());
                        break;
                    }
                    case ET::tensor_product_patch:
                        forward.tensor_product_patch(
                            e.get_tensor_product_patch());
                        break;
                    case ET::coons_patch:
                        forward.coons_patch(e.get_coons_patch());
                        break;
                    case ET::gouraud_triangle:
                        forward.gouraud_triangle(e.get_gouraud_triangle());
                        break;
                    case ET::empty:
                        break;
                }
                --element_index;
            } else {
                break;
            }
        }
    }

    template <typename SF>
    void riterate(SF &forward) const {
        this->riterate(forward, m_brackets.size(), 0, m_elements.size(), 0);
    }

    template <typename SF>
    void riterate(SF &&forward) const {
        this->riterate(forward);
    }


private:

    friend i_scene_data<scene_data>;

    void do_painted_shape(e_winding_rule winding_rule, const shape &s,
        const paint &p) {
        m_elements.emplace_back(painted_shape_data{winding_rule,
            make_intrusive<shape>(s),
            make_intrusive<paint>(p)});
    }

    void do_tensor_product_patch(const patch<16,4> &tpp) {
        m_elements.emplace_back(make_intrusive<const patch<16,4>>(tpp));
    }

    void do_coons_patch(const patch<12,4> &cp) {
        m_elements.emplace_back(make_intrusive<const patch<12,4>>(cp));
    }

    void do_gouraud_triangle(const patch<3,3> &gt) {
        m_elements.emplace_back(make_intrusive<const patch<3,3>>(gt));
    }

    void do_stencil_shape(e_winding_rule winding_rule, const shape &s) {
        m_elements.emplace_back(stencil_shape_data{winding_rule,
            make_intrusive<shape>(s)});
    }

    void do_begin_clip(uint16_t depth) {
        if (m_matching_clip_bracket.size() <= depth) {
            m_matching_clip_bracket.resize(depth+1);
        }
        m_matching_clip_bracket[depth] = static_cast<scene_bracket::element_id>(
            m_brackets.size());
        m_brackets.emplace_back(
            // we don't know the matching end_clip yet
            scene_bracket::begin_clip_data{0},
            static_cast<scene_bracket::element_id>(m_elements.size()),
            depth
        );
    }

    void do_activate_clip(uint16_t depth) {
        auto activate_index = static_cast<scene_bracket::element_id>(
            m_brackets.size());
        auto begin_index = m_matching_clip_bracket[depth];
        m_brackets.emplace_back(
            scene_bracket::activate_clip_data{begin_index},
            static_cast<scene_bracket::element_id>(m_elements.size()),
            depth
        );
        m_matching_clip_bracket[depth] = activate_index;
    }

    void do_end_clip(uint16_t depth) {
        auto end_index = static_cast<scene_bracket::element_id>(
            m_brackets.size());
        auto activate_index = m_matching_clip_bracket[depth];
        m_brackets.emplace_back(
            scene_bracket::end_clip_data{activate_index},
            static_cast<scene_bracket::element_id>(m_elements.size()),
            depth
        );
        auto begin_index = m_brackets[activate_index].get_activate_clip_data().
            matching_bracket;
        m_brackets[begin_index].get_begin_clip_data().matching_bracket =
            end_index;
    }

    void do_begin_fade(uint16_t depth, unorm8 opacity) {
        m_brackets.emplace_back(
            scene_bracket::begin_fade_data{opacity},
            static_cast<scene_bracket::element_id>(m_elements.size()),
            depth
        );
    }

    void do_end_fade(uint16_t depth, unorm8 opacity) {
        m_brackets.emplace_back(
            scene_bracket::end_fade_data{opacity},
            static_cast<scene_bracket::element_id>(m_elements.size()),
            depth
        );
    }

    void do_begin_blur(uint16_t depth, float radius) {
        m_brackets.emplace_back(
            scene_bracket::begin_blur_data{radius},
            static_cast<scene_bracket::element_id>(m_elements.size()),
            depth
        );
    }

    void do_end_blur(uint16_t depth, float radius) {
        m_brackets.emplace_back(
            scene_bracket::end_blur_data{radius},
            static_cast<scene_bracket::element_id>(m_elements.size()),
            depth
        );
    }

    void do_begin_transform(uint16_t depth, const xform &xf) {
        m_brackets.emplace_back(
            scene_bracket::begin_transform_data{xf},
            static_cast<scene_bracket::element_id>(m_elements.size()),
            depth
        );

    }

    void do_end_transform(uint16_t depth, const xform &xf) {
        m_brackets.emplace_back(
            scene_bracket::end_transform_data{xf},
            static_cast<scene_bracket::element_id>(m_elements.size()),
            depth
        );
    }
};

} // namespace rvg

#endif
