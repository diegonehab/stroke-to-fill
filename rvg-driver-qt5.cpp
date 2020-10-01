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
#include <string>
#include <sstream>
#include <cmath>
#include <vector>

#include "rvg-lua.h"
#include "rvg-lua-facade.h"

#include "rvg-i-scene-data.h"

#include "rvg-input-path-f-xform.h"
#include "rvg-image.h"
#include "rvg-pngio.h"

#include "rvg-xform.h"

#include "rvg-tuple.h"
#include "rvg-bezier.h"

#include "rvg-scene.h"
#include "rvg-viewport.h"

#include "rvg-i-sink.h"

#include "rvg-driver-qt5.h"

#include "qpainter.h"
#include "qpainterpath.h"
#include "qbuffer.h"

namespace rvg { namespace driver { namespace qt5 {


class input_path_f_to_qpainter_path final :
	public i_input_path<input_path_f_to_qpainter_path> {

	QPainterPath *m_path;

public:
	input_path_f_to_qpainter_path(QPainterPath *path) : m_path(path) { }

private:

	friend i_input_path<input_path_f_to_qpainter_path>;

	void do_begin_contour(float x0, float y0) {
		m_path->moveTo(x0, y0);
	}

	void do_end_open_contour(float x0, float y0) {
		(void)x0;
		(void)y0;
	}

	void do_end_closed_contour(float x0, float y0) {
		(void)x0;
		(void)y0;
		m_path->closeSubpath();
	}

	void do_linear_segment(float x0, float y0, float x1, float y1) {
		(void)x0; (void)y0;
		m_path->lineTo(x1, y1);
	}

	void do_quadratic_segment(float x0, float y0, float x1, float y1,
		float x2, float y2) {
		(void)x0; (void)y0;
		m_path->quadTo(x1, y1, x2, y2);
	}

	void do_rational_quadratic_segment(float x0, float y0, float x1, float y1,
		float w1, float x2, float y2) {
		(void)x0; (void)y0;
		(void)x1; (void)y1; (void)w1;
		(void)x2; (void)y2;
		// Should never be called
	}

	void do_cubic_segment(float x0, float y0, float x1, float y1,
		float x2, float y2, float x3, float y3) {
		(void)x0; (void)y0;
		m_path->cubicTo(x1, y1, x2, y2, x3, y3);
	}

};



class qt_scene_iterator : public i_scene_data<qt_scene_iterator> {
public:
	std::vector<xform> xf_stack;
	QPainter *painter;

	qt_scene_iterator(QPainter *p, const xform &xf) : painter(p) { push_xf(xf);}

	void push_xf(const xform &xf) {
		xf_stack.push_back(top_xf() * xf);
	}

	void pop_xf() {
		xf_stack.pop_back();
	}

	xform top_xf() {
		static xform id;
		return xf_stack.empty() ? id : xf_stack.back();
	}


	void set_paint(const paint &p, QBrush &brush) {
		RGBA8 color = p.get_solid_color();
		float r = (float)color[0] / 255.f;
		float g = (float)color[1] / 255.f;
		float b = (float)color[2] / 255.f;
		float a = (float)color[3] / 255.f;
		brush = QBrush(QColor::fromRgbF(r, g, b, a));
	}



	void do_painted_shape(e_winding_rule wr, const shape &s, const paint &p) {
		auto pd = s.as_path_data_ptr();


		QPainterPath qt_input_path;
		QBrush brush;

		auto iter = input_path_f_to_qpainter_path(&qt_input_path);
		set_paint(p, brush);

		push_xf(s.get_xf());
		pd->iterate(make_input_path_f_xform(top_xf(), iter));
		pop_xf();

		if (wr == e_winding_rule::odd) {
            qt_input_path.setFillRule(Qt::OddEvenFill);
        } else {
            qt_input_path.setFillRule(Qt::WindingFill);
        }


		painter->fillPath(qt_input_path, brush);

	}

    void do_tensor_product_patch(const patch<16,4> &tpp) {
        (void) tpp;
    }

    void do_coons_patch(const patch<12,4> &cp) {
        (void) cp;
    }

    void do_gouraud_triangle(const patch<3,3> &gt) {
        (void) gt;
    }

	void do_stencil_shape(e_winding_rule wr, const shape &s) {
		(void)wr;
		(void)s;
	}

	void do_begin_clip(uint16_t depth) {
		(void)depth;
	}
	void do_activate_clip(uint16_t depth) {
		(void)depth;
	}
	void do_end_clip(uint16_t depth) {
		(void)depth;
	}
	void do_begin_fade(uint16_t depth, uint8_t opacity) {
		(void)depth;
		(void)opacity;
	}
	void do_end_fade(uint16_t depth, uint8_t opacity) {
		(void)depth;
		(void)opacity;
	}
	void do_begin_blur(uint16_t depth, float radius) {
		(void)depth;
		(void)radius;
	}
	void do_end_blur(uint16_t depth, float radius) {
		(void)depth;
		(void)radius;
	}
	void do_begin_transform(uint16_t depth, const xform &xf) {
		(void)depth;
		push_xf(xf);
	}
	void do_end_transform(uint16_t depth, const xform &xf) {
		(void)depth;
		(void)xf;
		pop_xf();
	}
};


const scene &accelerate(const scene &c, const window &w,
	const viewport &v) {
		(void)w;
		(void)v;
	return c;
}

void render(const scene &c, const window &w, const viewport &v,
	FILE *out, const std::vector<std::string> &args) {
	(void)out; (void)args;
	int xl, yb, xr, yt;
	std::tie(xl, yb) = v.bl();
	std::tie(xr, yt) = v.tr();
	int width = std::abs(xl - xr);
	int height = std::abs(yt - yb);


	xform flip = make_translation(0.f, -static_cast<float>(yb)).
		scaled(1.f, -1.f).translated(0.f, static_cast<float>(yt));
	xform screen_xf = flip * c.get_xf().windowviewport(w, v).translated(-xl, yb);


	QImage dev(QSize(width, height), QImage::Format_ARGB32);
	dev.fill(0);

	QPainter painter(&dev);
	painter.setRenderHint(QPainter::Antialiasing);

	qt_scene_iterator iter(&painter, screen_xf);

	c.get_scene_data().iterate(iter);

	QByteArray ba;
	QBuffer buffer(&ba);
	buffer.open(QIODevice::WriteOnly);
	dev.save(&buffer, "PNG");

	fwrite(ba.data(), sizeof(char), ba.length(), out);
}


}}}

// Lua version of the accelerate function.
// Since there is no acceleration, we simply
// and return the input scene unmodified.
static int luaaccelerate(lua_State *L) {
	rvg_lua_push<rvg::scene>(L, rvg::driver::qt5::accelerate(
		rvg_lua_check<rvg::scene>(L, 1),
		rvg_lua_check<rvg::window>(L, 2),
		rvg_lua_check<rvg::viewport>(L, 3)));
	return 1;
}

// Lua version of render function
static int luarender(lua_State *L) {
	auto c = rvg_lua_check<rvg::scene>(L, 1);
	auto w = rvg_lua_check<rvg::window>(L, 2);
	auto v = rvg_lua_check<rvg::viewport>(L, 3);
	rvg::driver::qt5::render(c, w, v, rvg_lua_check_file(L, 4));
	return 0;
}

// List of Lua functions exported into driver table
static const luaL_Reg modrg[] = {
	{"render", luarender },
	{"accelerate", luaaccelerate },
	{NULL, NULL}
};

// Lua function invoked to be invoked by require"driver.cairo"
extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_driver_qt5(lua_State *L) {
	rvg_lua_facade_new_driver(L, modrg);
	return 1;
}
