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
#include <EGL/egl.h>
#include <GL/glew.h>

#include <string>
#include <sstream>
#include <cmath>

#include "rvg-lua.h"
#include "rvg-lua-facade.h"

#include "rvg-i-svg-path.h"
#include "rvg-i-scene-data.h"

#include "rvg-input-path-f-to-svg-path.h"
#include "rvg-input-path-f-xform.h"
#include "rvg-image.h"
#include "rvg-pngio.h"

#include "rvg-xform.h"
#include "rvg-driver-nvpr.h"

namespace rvg {
    namespace driver {
        namespace nvpr {

const scene &accelerate(const scene &c, const window &w,
    const viewport &v) {
    (void) w;
    (void) v;
    return c;
}

class svg_path_f_command_coords final:
    public i_svg_path<svg_path_f_command_coords> {
private:
    std::vector<GLubyte> &m_commands;
    std::vector<float> &m_coords;

    template <typename ...DATA>
    void push_cmd(int cmd, DATA ...coords) {
        m_commands.push_back(cmd);
        push_coords(coords...);
    }

    void push_coords(void) { }

    template <typename ...REST>
    void push_coords(rvgf f, REST... rest) {
        m_coords.push_back((float)f);
        push_coords(rest...);
    }

public:
    svg_path_f_command_coords( std::vector<uint8_t> &commands, std::vector<float> &coords):
        m_commands(commands), m_coords(coords) { ; }

private:
    friend i_svg_path<svg_path_f_command_coords>;

    void do_move_to_abs(rvgf x0, rvgf y0) {
        push_cmd(GL_MOVE_TO_NV, x0, y0);
    }

    void do_move_to_rel(rvgf x0, rvgf y0) {
        push_cmd(GL_RELATIVE_MOVE_TO_NV, x0, y0);
    }

    void do_close_path(void) {
        push_cmd(GL_CLOSE_PATH_NV);
    }

    void do_line_to_abs(rvgf x1, rvgf y1) {
        push_cmd(GL_LINE_TO_NV, x1, y1);
    }

    void do_line_to_rel(rvgf x1, rvgf y1) {
        push_cmd(GL_RELATIVE_LINE_TO_NV, x1, y1);
    }

    void do_hline_to_abs(rvgf x1) {
        push_cmd(GL_HORIZONTAL_LINE_TO_NV, x1);
    }

    void do_hline_to_rel(rvgf x1) {
        push_cmd(GL_RELATIVE_HORIZONTAL_LINE_TO_NV, x1);
    }

    void do_vline_to_abs(rvgf y1) {
        push_cmd(GL_VERTICAL_LINE_TO_NV, y1);
    }

    void do_vline_to_rel(rvgf y1) {
        push_cmd(GL_RELATIVE_VERTICAL_LINE_TO_NV, y1);
    }

    void do_quad_to_abs(rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        push_cmd(GL_QUADRATIC_CURVE_TO_NV, x1, y1, x2, y2);
    }

    void do_quad_to_rel(rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        push_cmd(GL_RELATIVE_QUADRATIC_CURVE_TO_NV, x1, y1, x2, y2);
    }

    void do_squad_to_abs(rvgf x2, rvgf y2) {
        push_cmd(GL_SMOOTH_QUADRATIC_CURVE_TO_NV, x2, y2);
    }

    void do_squad_to_rel(rvgf x2, rvgf y2) {
        push_cmd(GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_NV, x2, y2);
    }

    void do_rquad_to_abs(rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        (void) x1;
        (void) y1;
        (void) w1;
        (void) x2;
        (void) y2;
        //push_cmd('R', x1, y1, w1, x2, y2);
    }

    void do_rquad_to_rel(rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        (void) x1;
        (void) y1;
        (void) w1;
        (void) x2;
        (void) y2;
        //push_cmd('r', x1, y1, w1, x2, y2);
    }

    void do_svg_arc_to_abs(rvgf rx, rvgf ry, rvgf a, rvgf fa, rvgf fs,
        rvgf x2, rvgf y2) {
        if (fa) {
            if (fs) {
                push_cmd(GL_LARGE_CCW_ARC_TO_NV, rx, ry, a, x2, y2);
            } else {
                push_cmd(GL_LARGE_CW_ARC_TO_NV, rx, ry, a, x2, y2);
            }
        } else {
            if (fs) {
                push_cmd(GL_SMALL_CCW_ARC_TO_NV, rx, ry, a, x2, y2);
            } else {
                push_cmd(GL_SMALL_CW_ARC_TO_NV, rx, ry, a, x2, y2);
            }
        }
    }

    void do_svg_arc_to_rel(rvgf rx, rvgf ry, rvgf a, rvgf fa, rvgf fs,
        rvgf x2, rvgf y2) {
        if (fa) {
            if (fs) {
                push_cmd(GL_RELATIVE_LARGE_CCW_ARC_TO_NV, rx, ry, a, x2, y2);
            } else {
                push_cmd(GL_RELATIVE_LARGE_CW_ARC_TO_NV, rx, ry, a, x2, y2);
            }
        } else {
            if (fs) {
                push_cmd(GL_RELATIVE_SMALL_CCW_ARC_TO_NV, rx, ry, a, x2, y2);
            } else {
                push_cmd(GL_RELATIVE_SMALL_CW_ARC_TO_NV, rx, ry, a, x2, y2);
            }
        }
    }

    void do_cubic_to_abs(rvgf x1, rvgf y1, rvgf x2, rvgf y2,
    rvgf x3, rvgf y3) {
        push_cmd(GL_CUBIC_CURVE_TO_NV, x1, y1, x2, y2, x3, y3);
    }

    void do_cubic_to_rel(rvgf x1, rvgf y1, rvgf x2, rvgf y2,
    rvgf x3, rvgf y3) {
        push_cmd(GL_RELATIVE_CUBIC_CURVE_TO_NV, x1, y1, x2, y2, x3, y3);
    }

    void do_scubic_to_abs(rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        push_cmd(GL_SMOOTH_CUBIC_CURVE_TO_NV, x2, y2, x3, y3);
    }

    void do_scubic_to_rel(rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        push_cmd(GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO_NV, x2, y2, x3, y3);
    }
};

void xf_gl_mul(const xform &xf) {
    float gl_mat[16] = {
        static_cast<float>(xf[0][0]), static_cast<float>(xf[1][0]), 0.f, 0.f,
        static_cast<float>(xf[0][1]), static_cast<float>(xf[1][1]), 0.f, 0.f,
        0.f,                          0.f,                          1.f, 0.f,
        static_cast<float>(xf[0][2]), static_cast<float>(xf[1][2]), 0.f, 1.f,
    };
    glMultMatrixf(gl_mat);
}

inline svg_path_f_command_coords make_svg_path_f_commands_coords(
    std::vector<GLubyte> &commands, std::vector<float> &coords) {
    return svg_path_f_command_coords{commands, coords};
}

class scene_f_nvpr final: public i_scene_data<scene_f_nvpr> {

	std::vector<xform> m_xf_stack;

public:

    scene_f_nvpr(const xform &xf) {
        m_xf_stack.push_back(xf);
    }

private:
    using base = i_scene_data<scene_f_nvpr>;

    void push_xf(const xform &xf) {
        if (m_xf_stack.empty()) {
            m_xf_stack.push_back(xf);
        } else {
            m_xf_stack.push_back(m_xf_stack.back().transformed(xf));
        }
        glPushMatrix();
        xf_gl_mul(xf);
    }

    void pop_xf(void) {
        if (!m_xf_stack.empty()) {
            glPopMatrix();
            m_xf_stack.pop_back();
        }
    }

    xform top_xf(void) {
        if (!m_xf_stack.empty()) {
            return m_xf_stack.back();
        } else return xform{};
    }

    void set_paint(const paint &p) {
        if (p.get_type() == paint::e_type::solid_color) {
            RGBA<unorm<float>> c = p.get_solid_color();
            glColor4f(c[0], c[1], c[2], c[3] * unorm<float>{p.get_opacity()});
        }
	}

    void set_cap(e_stroke_cap cap, GLint what, GLuint nvpr_path) {
		GLint nvpr_cap = GL_FLAT;
		switch (cap) {
			case e_stroke_cap::butt:
				nvpr_cap = GL_FLAT;
				break;
			case e_stroke_cap::round:
				nvpr_cap = GL_ROUND_NV;
				break;
			case e_stroke_cap::square:
				nvpr_cap = GL_SQUARE_NV;
				break;
			case e_stroke_cap::triangle:
				nvpr_cap = GL_TRIANGULAR_NV;
				break;
			default:
				break;
		}
		glPathParameteriNV(nvpr_path, what, nvpr_cap);
    }

    void set_join(e_stroke_join join, GLuint nvpr_path) {
		switch (join) {
			case e_stroke_join::miter_or_bevel:
				glPathParameteriNV(nvpr_path, GL_PATH_JOIN_STYLE_NV,
					GL_MITER_REVERT_NV);
				break;
			case e_stroke_join::miter_clip:
				glPathParameteriNV(nvpr_path, GL_PATH_JOIN_STYLE_NV,
					GL_MITER_TRUNCATE_NV);
                break;
			case e_stroke_join::round:
				glPathParameteriNV(nvpr_path, GL_PATH_JOIN_STYLE_NV,
					GL_ROUND_NV);
				break;
			case e_stroke_join::bevel:
				glPathParameteriNV(nvpr_path, GL_PATH_JOIN_STYLE_NV,
					GL_BEVEL_NV);
				break;
			default:
				break;
		}
    }

    void set_stroke_style(float width, const stroke_style &st,
        GLuint nvpr_path) {
		glPathParameterfNV(nvpr_path, GL_PATH_STROKE_WIDTH_NV, width);
        set_join(st.get_join(), nvpr_path);
        set_cap(st.get_initial_cap(), GL_PATH_INITIAL_END_CAP_NV, nvpr_path);
		set_cap(st.get_terminal_cap(), GL_PATH_TERMINAL_END_CAP_NV, nvpr_path);
		set_cap(st.get_dash_initial_cap(), GL_PATH_INITIAL_DASH_CAP_NV,
            nvpr_path);
		set_cap(st.get_dash_terminal_cap(), GL_PATH_TERMINAL_DASH_CAP_NV,
            nvpr_path);
		glPathParameterfNV(nvpr_path, GL_PATH_MITER_LIMIT_NV,
            st.get_miter_limit());
		glPathParameteriNV(nvpr_path, GL_PATH_DASH_OFFSET_RESET_NV,
			st.get_resets_on_move()? GL_MOVE_TO_RESETS_NV:
                GL_MOVE_TO_CONTINUES_NV);
		glPathParameterfNV(nvpr_path, GL_PATH_DASH_OFFSET_NV,
            st.get_dash_offset()*width);
        std::vector<float> dashes;
        for (auto d: st.get_dashes()) dashes.push_back(d*width);
		glPathDashArrayNV(nvpr_path, dashes.size(), &dashes[0]);
	}

	GLuint new_nvpr_path(const path_data &p, const xform &pre_xf) {
        std::vector<GLubyte> commands;
        std::vector<float> coords;
		if (pre_xf.is_identity()) {
			p.iterate(
				make_input_path_f_to_svg_path(
                	make_svg_path_f_commands_coords(commands, coords)));
		} else {
			p.iterate(
				make_input_path_f_xform(pre_xf,
                    make_input_path_f_to_svg_path(
                        make_svg_path_f_commands_coords(commands, coords))));
		}
        GLuint nvpr_path = glGenPathsNV(1);
        glPathCommandsNV(nvpr_path, commands.size(), &commands[0],
            coords.size(), GL_FLOAT, &coords[0]);
		return nvpr_path;
	}

    friend base;

    void do_painted_shape(e_winding_rule wr, const shape &s, const paint &p) {
        const stroke_style *st = nullptr;
        float w = 0.f;
        xform pre_xf;
        path_data::const_ptr path_shape;
        if (!s.get_xf().is_identity()) push_xf(s.get_xf());
        if (s.is_stroke()) {
			st = &s.get_stroke_data().get_style();
			w = s.get_stroke_data().get_width();
			pre_xf = s.get_stroke_data().get_shape().get_xf();
			// convert shape to be stroked into a path
			path_shape = s.get_stroke_data().get_shape().as_path_data_ptr(
				pre_xf.transformed(top_xf()));
        } else {
            path_shape = s.as_path_data_ptr(top_xf());
        }
		GLuint nvpr_path = new_nvpr_path(*path_shape, pre_xf);
        set_paint(p);
		if (st) {
			set_stroke_style(w, *st, nvpr_path);
			glStencilStrokePathNV(nvpr_path, 0x1, ~0);
			glCoverStrokePathNV(nvpr_path, GL_CONVEX_HULL_NV);
		} else {
			if (wr == e_winding_rule::non_zero)
				glStencilFillPathNV(nvpr_path, GL_COUNT_UP_NV, 0x1F);
			else
				glStencilFillPathNV(nvpr_path, GL_INVERT, 0x1F);
			glCoverFillPathNV(nvpr_path, GL_BOUNDING_BOX_NV);
		}
        glDeletePathsNV(nvpr_path, 1);
        if (!s.get_xf().is_identity()) pop_xf();
    }

    void do_tensor_product_patch(const patch<16,4> &) { ; }
    void do_coons_patch(const patch<12,4> &) { ; }
    void do_gouraud_triangle(const patch<3,3> &) { ; }

    void do_stencil_shape(e_winding_rule wr, const shape &s) {
        (void) wr;
        (void) s;
    }

    void do_begin_clip(uint16_t depth) {
        (void) depth;
    }
    void do_activate_clip(uint16_t depth) {
        (void) depth;
    }
    void do_end_clip(uint16_t depth) {
        (void) depth;
    }
    void do_begin_fade(uint16_t depth, uint8_t opacity) {
        (void) depth;
        (void) opacity;
    }
    void do_end_fade(uint16_t depth, uint8_t opacity) {
        (void) depth;
        (void) opacity;
    }
    void do_begin_blur(uint16_t depth, float radius) {
        (void) depth;
        (void) radius;
    }
    void do_end_blur(uint16_t depth, float radius) {
        (void) depth;
        (void) radius;
    }
    void do_begin_transform(uint16_t depth, const xform &xf) {
        (void) depth;
        push_xf(xf);
    }
    void do_end_transform(uint16_t depth, const xform &xf) {
        (void) depth;
        (void) xf;
        pop_xf();
    }
};

void render(const scene &c, const window &w, const viewport &v,
    FILE *out, const std::vector<std::string> &args) {
    (void) out; (void) args;
    int xl, yb, xr, yt;
    std::tie(xl, yb) = v.bl();
    std::tie(xr, yt) = v.tr();
    int width = std::abs(xl-xr);
    int height = std::abs(yt-yb);
    const EGLint pbuffer_attribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE,
    };
    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_SAMPLES, 8,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
	// 1. Initialize EGL
	EGLDisplay egl_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (egl_dpy == EGL_NO_DISPLAY) {
        fprintf(stderr, "eglGetDisplay failed\n");
        return;
    }
	EGLint major, minor;
	if (eglInitialize(egl_dpy, &major, &minor) == EGL_FALSE) {
        fprintf(stderr, "eglInitialize failed\n");
        eglTerminate(egl_dpy);
        return;
    }
	// 2. Select an appropriate configuration
	EGLint num_configs = 0;
	eglChooseConfig(egl_dpy, config_attribs, nullptr, 0, &num_configs);
    if (num_configs <= 0) {
        fprintf(stderr, "eglChooseConfig failed\n");
        eglTerminate(egl_dpy);
        return;
    }
    std::vector<EGLConfig> egl_cfgs(num_configs);
	eglChooseConfig(egl_dpy, config_attribs, &egl_cfgs[0], num_configs,
        &num_configs);
    auto egl_cfg = egl_cfgs[0];
	// 3. Create a surface
	EGLSurface egl_surf = eglCreatePbufferSurface(egl_dpy, egl_cfg,
        pbuffer_attribs);
    if (egl_surf == EGL_NO_SURFACE) {
        fprintf(stderr, "eglCreatePbufferSurface failed\n");
        eglTerminate(egl_dpy);
        return;
    }

	// 4. Bind the API
	if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
        fprintf(stderr, "eglBindAPI failed\n");
        eglTerminate(egl_dpy);
        return;
    }

	// 5. Create a context and make it current
	EGLContext egl_ctx = eglCreateContext(egl_dpy, egl_cfg, EGL_NO_CONTEXT, NULL);
    if (egl_ctx == EGL_NO_CONTEXT) {
        fprintf(stderr, "eglCreateContext failed\n");
        eglTerminate(egl_dpy);
        return;
    }
	if (eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx) == EGL_FALSE) {
        fprintf(stderr, "eglMakeCurrent failed\n");
        eglTerminate(egl_dpy);
        return;
    }
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "glew failed\n");
	}

	if (!GL_NV_path_rendering) {
		fprintf(stderr, "no nvpr\n");
	}
    // 6. Go on drawing
    glClearColor(.0f, .0f, .0f, .0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,width,0,height,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0,0,width,height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 0, 0x1F);
    glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
    glClearStencil(0);
    glClearColor(.0f, .0f, .0f, .0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    xf_gl_mul(make_windowviewport(w, v) * c.get_xf());
    scene_f_nvpr nvpr_scene(c.get_xf());
    c.get_scene_data().iterate(nvpr_scene);
    glPopMatrix();
    glFlush();
    std::vector<uint8_t> data(width*height*4);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
    image<uint8_t, 4> buf;
    buf.load_from(width, width, height, 4*width, 4, &data[0], &data[1],
        &data[2], &data[3]);
    store_png<uint8_t>(out, buf);
	// 7. Terminate EGL when finished
	eglTerminate(egl_dpy);
}

} } } // namespace rvg::driver::nvpr

// Lua version of the accelerate function.
// Since there is no acceleration, we simply
// and return the input scene unmodified.
// ??D maybe we should create a "display list" and return it?
static int luaaccelerate(lua_State *L) {
    lua_settop(L, 1);
    return 1;
}

// Lua version of the render function
static int luarender(lua_State *L) {
    auto c = rvg_lua_check<rvg::scene>(L, 1);
    auto w = rvg_lua_check<rvg::window>(L, 2);
    auto v = rvg_lua_check<rvg::viewport>(L, 3);
    rvg::driver::nvpr::render(c, w, v, rvg_lua_check_file(L, 4));
    return 0;
}

// List of Lua functions exported into driver table
static const luaL_Reg modnvpregl[] = {
    {"render", luarender },
    {"accelerate", luaaccelerate },
    {NULL, NULL}
};

// Lua function invoked to be invoked by require"driver.nvpr"
extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_driver_nvpr(lua_State *L) {
	rvg_lua_facade_new_driver(L, modnvpregl);
    return 1;
}
