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
#ifndef RVG_I_PARAMETERS_F_HOLD_H
#define RVG_I_PARAMETERS_F_HOLD_H

#include <algorithm>   // std::sort
#include <boost/container/small_vector.hpp>

#include "rvg-i-sink.h"
#include "rvg-path-instruction.h"
#include "rvg-floatint.h"
#include "rvg-monotonic-parameters-f-forward-if.h"
#include "rvg-cubic-parameters-f-forward-if.h"
#include "rvg-offsetting-parameters-f-forward-if.h"
#include "rvg-join-parameters-f-forward-if.h"
#include "rvg-dashing-parameters-f-forward-if.h"

namespace rvg {

template <typename DERIVED>
class i_parameters_f_hold:
    public i_monotonic_parameters<DERIVED>,
    public i_cubic_parameters<DERIVED>,
    public i_offsetting_parameters<DERIVED>,
    public i_join_parameters<DERIVED>,
    public i_dashing_parameters<DERIVED> {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    i_parameters_f_hold(void) {
        static_assert(meta::is_an_i_sink<DERIVED>::value,
            "derived class is not an i_sink");
    }

    using parameter = std::pair<path_instruction, floatint>;

protected:

    auto &parameters(void) {
        return m_parameters;
    }

    const auto &parameters(void) const {
        return m_parameters;
    }

    void forward_parameters(void) {
        auto &roots = make_monotonic_parameters_f_forward_if(derived().sink());
        auto &cubic = make_cubic_parameters_f_forward_if(derived().sink());
        auto &join = make_join_parameters_f_forward_if(derived().sink());
        auto &offsetting = make_offsetting_parameters_f_forward_if(
            derived().sink());
        auto &dashing = make_dashing_parameters_f_forward_if(derived().sink());
        for (const auto &p: m_parameters) {
            switch (p.first) {
                case path_instruction::inflection_parameter:
                    cubic.inflection_parameter(p.second.f);
                    break;
                case path_instruction::double_point_parameter:
                    cubic.double_point_parameter(p.second.f);
                    break;
                case path_instruction::root_dx_parameter:
                    roots.root_dx_parameter(p.second.f);
                    break;
                case path_instruction::root_dy_parameter:
                    roots.root_dy_parameter(p.second.f);
                    break;
                case path_instruction::root_dw_parameter:
                    roots.root_dw_parameter(p.second.f);
                    break;
                case path_instruction::offset_cusp_parameter:
                    offsetting.offset_cusp_parameter(p.second.f);
                    break;
                case path_instruction::evolute_cusp_parameter:
                    offsetting.evolute_cusp_parameter(p.second.f);
                    break;
                case path_instruction::join_tangent_parameter:
                    join.join_tangent_parameter(p.second.f);
                    break;
                case path_instruction::join_vertex_parameter:
                    join.join_vertex_parameter(p.second.f);
                    break;
                case path_instruction::begin_dash_parameter:
                    dashing.begin_dash_parameter(p.second.f);
                    break;
                case path_instruction::end_dash_parameter:
                    dashing.end_dash_parameter(p.second.f);
                    break;
                case path_instruction::backward_begin_dash_parameter:
                    dashing.backward_begin_dash_parameter(p.second.f);
                    break;
                case path_instruction::backward_end_dash_parameter:
                    dashing.backward_end_dash_parameter(p.second.f);
                    break;
                default:
                    // ignore
                    break;
            }
        }
    }

    void forward_parameters(rvgf t0, rvgf t1) {
        auto &roots = make_monotonic_parameters_f_forward_if(derived().sink());
        auto &cubic = make_cubic_parameters_f_forward_if(derived().sink());
        auto &join = make_join_parameters_f_forward_if(derived().sink());
        auto &offsetting = make_offsetting_parameters_f_forward_if(
            derived().sink());
        auto &dashing = make_dashing_parameters_f_forward_if(derived().sink());
        for (const auto &p: m_parameters) {
            rvgf t = p.second.f;
            if (t <= t0 || t >= t1) continue;
            switch (p.first) {
                case path_instruction::inflection_parameter:
                    cubic.inflection_parameter(t);
                    break;
                case path_instruction::double_point_parameter:
                    cubic.double_point_parameter(t);
                    break;
                case path_instruction::root_dx_parameter:
                    roots.root_dx_parameter(t);
                    break;
                case path_instruction::root_dy_parameter:
                    roots.root_dy_parameter(t);
                    break;
                case path_instruction::root_dw_parameter:
                    roots.root_dw_parameter(t);
                    break;
                case path_instruction::offset_cusp_parameter:
                    offsetting.offset_cusp_parameter(t);
                    break;
                case path_instruction::evolute_cusp_parameter:
                    offsetting.evolute_cusp_parameter(t);
                    break;
                case path_instruction::join_tangent_parameter:
                    join.join_tangent_parameter(t);
                    break;
                case path_instruction::join_vertex_parameter:
                    join.join_vertex_parameter(t);
                    break;
                case path_instruction::begin_dash_parameter:
                    dashing.begin_dash_parameter(t);
                    break;
                case path_instruction::end_dash_parameter:
                    dashing.end_dash_parameter(t);
                    break;
                case path_instruction::backward_begin_dash_parameter:
                    dashing.backward_begin_dash_parameter(t);
                    break;
                case path_instruction::backward_end_dash_parameter:
                    dashing.backward_end_dash_parameter(t);
                    break;
                default:
                    // ignore
                    break;
            }
        }
    }

    void sort_parameters(void) {
		std::sort(m_parameters.begin(), m_parameters.end(),
			[](const parameter &a, const parameter &b) {
				return a.second.f < b.second.f;
		});
	}

    void clear_parameters(void) {
        m_parameters.clear();
    }

// i_cubic_parameters interface
friend i_cubic_parameters<DERIVED>;

    void do_inflection_parameter(rvgf t) {
        m_parameters.emplace_back(path_instruction::inflection_parameter, t);
    }

    void do_double_point_parameter(rvgf t) {
        m_parameters.emplace_back(path_instruction::double_point_parameter, t);
    }

// i_offsetting_parameters interface
friend i_offsetting_parameters<DERIVED>;

    void do_offset_cusp_parameter(rvgf t) {
        m_parameters.emplace_back(path_instruction::offset_cusp_parameter, t);
    }

    void do_evolute_cusp_parameter(rvgf t) {
        m_parameters.emplace_back(path_instruction::evolute_cusp_parameter, t);
    }

// i_join_parameters interface
friend i_join_parameters<DERIVED>;

    void do_join_tangent_parameter(rvgf t) {
        m_parameters.emplace_back(path_instruction::join_tangent_parameter, t);
    }

    void do_join_vertex_parameter(rvgf t) {
        m_parameters.emplace_back(path_instruction::join_vertex_parameter, t);
    }

// i_decorated_path interface
friend i_dashing_parameters<DERIVED>;

    void do_begin_dash_parameter(rvgf t) {
        m_parameters.emplace_back(path_instruction::begin_dash_parameter, t);
    }

    void do_end_dash_parameter(rvgf t) {
        m_parameters.emplace_back(path_instruction::end_dash_parameter, t);
    }

    void do_backward_begin_dash_parameter(rvgf t) {
        m_parameters.emplace_back(path_instruction::backward_begin_dash_parameter, t);
    }

    void do_backward_end_dash_parameter(rvgf t) {
        m_parameters.emplace_back(path_instruction::backward_end_dash_parameter, t);
    }

// i_monotonic_parameters interface
friend i_monotonic_parameters<DERIVED>;

    void do_root_dx_parameter(rvgf t) {
        m_parameters.emplace_back(path_instruction::root_dx_parameter, t);
    }

    void do_root_dy_parameter(rvgf t) {
        m_parameters.emplace_back(path_instruction::root_dy_parameter, t);
    }

    void do_root_dw_parameter(rvgf t) {
        m_parameters.emplace_back(path_instruction::root_dw_parameter, t);
    }

private:

    boost::container::small_vector<parameter, 12> m_parameters;

};

} // namespace rvg

#endif
