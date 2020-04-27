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
#ifndef RVG_SCENE_F_SPY_H
#define RVG_SCENE_F_SPY_H

#include <string>
#include <iostream>
#include <iomanip>

#include "rvg-lua.h"
#include "rvg-i-scene-data.h"
#include "rvg-scene-f-null.h"

namespace rvg {

template <typename SINK>
class scene_f_spy: public i_scene_data<scene_f_spy<SINK>> {

    std::string m_name;
    std::ostream &m_out;
    SINK m_sink;

public:
	scene_f_spy(const char *name, std::ostream &out, SINK &&sink):
        m_name(name),
        m_out(out),
        m_sink(std::forward<SINK>(sink)) {
        ;
    }

private:

friend i_scene_data<scene_f_spy<SINK>>;

    const char *winding_rule_name(e_winding_rule rule) {
        switch (rule) {
            case e_winding_rule::odd: return "odd";
            case e_winding_rule::non_zero: return "non_zero";
            case e_winding_rule::even: return "even";
            case e_winding_rule::zero: return "zero";
            default: return "unkonwn";
        }
    }

	void do_painted_shape(e_winding_rule rule, const shape &s, const paint &p) {
        m_out << m_name << "painted_shape(" << winding_rule_name(rule) <<
            ", " << s.type_name() << ", " <<
            p.type_name() << ")\n";
        m_sink.painted_shape(rule, s, p);
	}

	void do_tensor_product_patch(const patch<16,4> &tpp) {
        m_out << m_name << "tensor_product_patch\n";
        m_sink.tensor_product_patch(tpp);
	}

	void do_coons_patch(const patch<12,4> &cp) {
        m_out << m_name << "coons_patch\n";
        m_sink.coons_patch(cp);
	}

	void do_gouraud_triangle(const patch<3,3> &gt) {
        m_out << m_name << "gouraud_triangle\n";
        m_sink.gouraud_triangle(gt);
	}

	void do_stencil_shape(e_winding_rule rule, const shape &s) {
        m_out << m_name << "stencil_shape(" << winding_rule_name(rule) <<
            ", " << s.type_name() << ")\n";
        m_sink.stencil_shape(rule, s);
	}

	void do_begin_clip(uint16_t depth) {
        m_out << m_name << "begin_clip(" << depth << ")\n";
        m_sink.begin_clip(depth);
	}

	void do_end_clip(uint16_t depth) {
        m_out << m_name << "end_clip(" << depth << ")\n";
        m_sink.end_clip(depth);
	}

	void do_activate_clip(uint16_t depth) {
        m_out << m_name << "activate_clip(" << depth << ")\n";
        m_sink.activate_clip(depth);
	}

    void do_begin_fade(uint16_t depth, unorm8 opacity) {
        m_out << m_name << "begin_fade(" << depth << ", " <<
            static_cast<float>(unorm<float>(opacity)) << ")\n";
        m_sink.begin_fade(depth, opacity);
    }

    void do_end_fade(uint16_t depth, unorm8 opacity) {
        m_out << m_name << "end_fade(" << depth << ", " <<
            static_cast<float>(unorm<float>(opacity)) << ")\n";
        m_sink.end_fade(depth, opacity);
    }

    void do_begin_blur(uint16_t depth, float radius) {
        m_out << m_name << "begin_blur(" << depth << ", " <<
            radius << ")\n";
        m_sink.begin_blur(depth, radius);
    }

    void do_end_blur(uint16_t depth, float radius) {
        m_out << m_name << "end_blur(" << depth << ", " <<
            radius << ")\n";
        m_sink.end_blur(depth, radius);
    }

    void do_begin_transform(uint16_t depth, const xform &xf) {
        m_out << m_name << "begin_transform(" << depth << ", " <<
              xf << ")\n";
        m_sink.begin_transform(depth, xf);
    }

    void do_end_transform(uint16_t depth, const xform &xf) {
        m_out << m_name << "end_transform(" << depth << ", " <<
              xf << ")\n";
        m_sink.end_transform(depth, xf);
    }
};

template <typename SINK>
static inline auto
make_scene_f_spy(const char *name, std::ostream &out, SINK &&sink) {
    return scene_f_spy<SINK>{name, out, std::forward<SINK>(sink)};
}

template <typename SINK>
static inline auto
make_scene_f_spy(const char *name, SINK &&sink) {
    return scene_f_spy<SINK>{name, std::cerr, std::forward<SINK>(sink)};
}

static inline auto 
make_scene_f_spy(const char *name, std::ostream &out = std::cerr) {
    return scene_f_spy<scene_f_null>(name, out, make_scene_f_null());
}

} // namespace rvg

#endif
