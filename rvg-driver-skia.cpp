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
#include "rvg-canonize-rational-quadratic-bezier.h"

#include "rvg-scene.h"
#include "rvg-viewport.h"

#include "rvg-i-sink.h"

#include "rvg-driver-skia.h"

#include "SkPath.h"
#include "SkPaint.h"
#include "SkCanvas.h"
#include "SkTileMode.h"
#include "SkBitmap.h"
#include "SkImage.h"
#include "SkEncodedImageFormat.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkGradientShader.h"
#include "SkMaskFilter.h"
#include "SkDashPathEffect.h"

namespace rvg { namespace driver { namespace skia {

class accelerated {
    sk_sp<SkPicture> m_picture;
public:
    accelerated(sk_sp<SkPicture> picture): m_picture(picture) { ; }
    sk_sp<const SkPicture> get_picture(void) const { return m_picture; }
    sk_sp<SkPicture> get_picture(void) { return m_picture; }
};

class input_path_f_to_skpath : public i_input_path<input_path_f_to_skpath> {
    SkPath &m_skpath;
public:
    explicit input_path_f_to_skpath(SkPath &skpath) : m_skpath(skpath) { ; }

private:

    friend i_input_path<input_path_f_to_skpath>;

    void do_begin_contour(float x0, float y0) {
        m_skpath.moveTo(x0, y0);
    }

    void do_end_open_contour(float x0, float y0) {
        (void)x0; (void)y0;
        // do nothing
    }

    void do_end_closed_contour(float x0, float y0) {
        (void)x0; (void)y0;
        m_skpath.close();
    }

    void do_linear_segment(float x0, float y0, float x1, float y1) {
        (void)x0; (void)y0;
        m_skpath.lineTo(x1, y1);
    }

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
        (void)x0; (void)y0;
        m_skpath.quadTo(x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment(float x0, float y0, float x1, float y1,
        float w1, float x2, float y2) {
        if (util::is_almost_zero(w1)) {
            R3 p0{x0,y0,1}, p1{x1,y1,w1}, p2{x2,y2,1};
            R3 q1,q2,q3;
            // split in two
            std::tie(q1,q2,q3) = bezier_split<rvgf>(
                std::make_tuple(p0,p1,p2), rvgf{0.5}
            );
            // canonize
            R2 a0, a2, b0, b2;
            R3 a1, b1;
            std::tie(a0, a1, a2) = canonize_rational_quadratic_bezier(p0,q1,q2);
            std::tie(b0, b1, b2) = canonize_rational_quadratic_bezier(q2,q3,p2);
            // send two pieces
            m_skpath.conicTo(a1[0]/a1[2], a1[1]/a1[2], a2[0], a2[1], a1[2]);
            m_skpath.conicTo(b1[0]/b1[2], b1[1]/b1[2], b2[0], b2[1], b1[2]);
        } else {
            m_skpath.conicTo(x1 / w1, y1 / w1, x2, y2, w1);
        }
    }

    void do_cubic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3) {
        (void)x0; (void)y0;
        m_skpath.cubicTo(x1, y1, x2, y2, x3, y3);
    }

};


static void skia_ramp_data(float opacity, const color_ramp& ramp,
    std::vector<SkScalar> &stops, std::vector<SkColor> &colors) {

    int size = ramp.get_color_stops().size();
    stops.reserve(size);
    colors.reserve(size);

    for (auto stop : ramp.get_color_stops()) {
        SkScalar t = stop.get_offset();
        auto color = stop.get_color();
        SkColor c = SkColorSetARGB((uint8_t)(opacity * color[3]), (uint8_t)color[0], (uint8_t)color[1], (uint8_t)color[2]);

        stops.push_back(t);
        colors.push_back(c);
    }
}

static SkTileMode spread_to_skia(e_spread s) {
    switch (s) {
        case e_spread::clamp: return SkTileMode::kClamp; break;
        case e_spread::wrap: return SkTileMode::kRepeat; break;
        case e_spread::mirror: return SkTileMode::kMirror; break;
        case e_spread::transparent:
        default: return SkTileMode::kDecal;break;
    }
}

static SkMatrix xf_to_skia(const xform &xf) {
    return SkMatrix::MakeAll(
        xf[0][0], xf[0][1], xf[0][2],
        xf[1][0], xf[1][1], xf[1][2],
        xf[2][0], xf[2][1], xf[2][2]);
}


class skia_scene_iterator : public i_scene_data<skia_scene_iterator> {
public:
    std::vector<xform> xf_stack;
    SkCanvas &canvas;

    skia_scene_iterator(SkCanvas &c, const xform &xf) : canvas(c) { push_xf(xf);}

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

    void set_paint(const paint &p, SkPaint &skp, const xform &xf) {
        skp.setAntiAlias(true);
        float opacity = float(p.get_opacity()) / 255.f;

        switch (p.get_type()) {
            case paint::e_type::solid_color: {
                auto color = p.get_solid_color();
                SkColor c = SkColorSetARGB((uint8_t)(opacity * color[3]), (uint8_t)color[0], (uint8_t)color[1], (uint8_t)color[2]);
                skp.setColor(c);
                break;
            }
            case paint::e_type::linear_gradient: {
                std::vector<SkScalar> stops;
                std::vector<SkColor> colors;
                auto &lg = p.get_linear_gradient_data();

                float x1, y1, x2, y2, w;
                std::tie(x1, y1, w) = p.get_xf().transformed(xf).apply(lg.get_x1(), lg.get_y1());
                std::tie(x2, y2, w) = p.get_xf().transformed(xf).apply(lg.get_x2(), lg.get_y2());

                SkPoint pts[2] = {{x1, y1}, {x2, y2}};

                skia_ramp_data(opacity, lg.get_color_ramp(), stops, colors);

                skp.setShader(SkGradientShader::MakeLinear(
                     pts, colors.data(), stops.data(), colors.size(),
                     spread_to_skia(lg.get_color_ramp().get_spread())));
                break;
            }
            case paint::e_type::radial_gradient: {
                std::vector<SkScalar> stops;
                std::vector<SkColor> colors;
                auto &rg = p.get_radial_gradient_data();
                auto skxf = xf_to_skia(p.get_xf().transformed(xf));

                SkPoint focus = {rg.get_fx(), rg.get_fy()};
                SkPoint center = {rg.get_cx(), rg.get_cy()};
                SkScalar radius = rg.get_r();

                skia_ramp_data(opacity, rg.get_color_ramp(), stops, colors);
                skp.setShader(SkGradientShader::MakeTwoPointConical(
                     focus, 0, center, radius, colors.data(), stops.data(), colors.size(),
                     spread_to_skia(rg.get_color_ramp().get_spread()), 0,
                     &skxf));
                break;
            }
            case paint::e_type::texture:
            default:
                break;
        }
    }

    void set_stroke_style(const stroke_style &st, float width, SkPaint &paint) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(width);
        paint.setStrokeMiter(st.get_miter_limit());

        switch (st.get_join()) {
            case e_stroke_join::miter_clip:
            case e_stroke_join::miter_or_bevel:
                paint.setStrokeJoin(SkPaint::kMiter_Join);
                break;
            case e_stroke_join::round:
                paint.setStrokeJoin(SkPaint::kRound_Join);
                break;
            case e_stroke_join::bevel:
            default:
                paint.setStrokeJoin(SkPaint::kBevel_Join);
                break;
        }

        switch (st.get_initial_cap()) {
            case e_stroke_cap::butt:
                paint.setStrokeCap(SkPaint::kButt_Cap);
                break;
            case e_stroke_cap::round:
                paint.setStrokeCap(SkPaint::kRound_Cap);
                break;
            case e_stroke_cap::square:
                paint.setStrokeCap(SkPaint::kSquare_Cap);
                break;
            default:
                break;
        }

        std::vector<SkScalar> dashes;
        for (auto d: st.get_dashes()) dashes.push_back(d*width);
        paint.setPathEffect(SkDashPathEffect::Make(dashes.data(), dashes.size(), st.get_dash_offset()*width));
    }


    void do_painted_shape(e_winding_rule wr, const shape &s, const paint &p) {
        (void) wr;

        SkPaint sk_paint;
        SkPath sk_path;
        path_data::const_ptr pd;

        xform pre_xf;
        push_xf(s.get_xf());
        if (s.is_stroke()) {
            auto st = &s.get_stroke_data().get_style();
            auto w = s.get_stroke_data().get_width();
            pre_xf = s.get_stroke_data().get_shape().get_xf();
            // convert shape to be stroked into a path
            pd = s.get_stroke_data().get_shape().as_path_data_ptr(
                pre_xf.transformed(top_xf()));

            set_stroke_style(*st, w, sk_paint);
        } else {
            pd = s.as_path_data_ptr(top_xf());
        }

        set_paint(p, sk_paint, pre_xf.transformed(s.get_xf().inverse()));

        auto iter = input_path_f_to_skpath(sk_path);
        pd->iterate(iter);

        sk_path.transform(xf_to_skia(pre_xf));
        canvas.setMatrix(xf_to_skia(top_xf()));
        canvas.drawPath(sk_path, sk_paint);


        pop_xf();
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

        auto pd = s.as_path_data_ptr();
        SkPath sk_path;

        auto iter = input_path_f_to_skpath(sk_path);

        push_xf(s.get_xf());
        pd->iterate(iter);

        canvas.setMatrix(xf_to_skia(top_xf()));
        canvas.clipPath(sk_path, SkClipOp::kIntersect, true);
        pop_xf();
    }

    void do_begin_clip(uint16_t depth) {
        (void)depth;
        canvas.save();
    }
    void do_activate_clip(uint16_t depth) {
        (void)depth;
    }
    void do_end_clip(uint16_t depth) {
        (void)depth;
        canvas.restore();
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

const accelerated accelerate(const scene &c, const window &w,
    const viewport &v) {

    int xl, yb, xr, yt;
    std::tie(xl, yb) = v.bl();
    std::tie(xr, yt) = v.tr();
    int width = std::abs(xl - xr);
    int height = std::abs(yt - yb);

    xform flip = make_translation(0.f,-static_cast<float>(yb)).
        scaled(1.f,-1.f).translated(0.f,static_cast<float>(yt));
    xform screen_xf = flip * c.get_xf().windowviewport(w, v).translated(-xl, yb);

    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording({0, 0,
        static_cast<float>(width), static_cast<float>(height)});

    auto iter = skia_scene_iterator(*canvas, screen_xf);

    //record comands
    c.get_scene_data().iterate(iter);

    return recorder.finishRecordingAsPicture();
}

static bool opt_no_output(const std::vector<std::string> &args) {
    for (const auto &s : args)
        if (s.compare("-no-output") == 0) return true;

    return false;
}

void render(const accelerated &a, const window &w, const viewport &v,
    FILE *out, const std::vector<std::string> &args) {
    (void) args; (void) w;

    int xl, yb, xr, yt;
    std::tie(xl, yb) = v.bl();
    std::tie(xr, yt) = v.tr();
    int width = std::abs(xl - xr);
    int height = std::abs(yt - yb);

    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    const size_t row_size = info.minRowBytes();

    SkBitmap bitmap;
    bitmap.allocPixels(info, row_size);
    std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirect(info, bitmap.getPixels(), row_size);

    canvas->clear(0);
    a.get_picture()->playback(canvas.get());
    canvas->flush();

    if (!opt_no_output(args)) {
        auto image = SkImage::MakeFromBitmap(bitmap);
        auto data = image->encodeToData(SkEncodedImageFormat::kPNG , 100);
        fwrite(data->data(), 1, data->size(), out);
    }
}


}}}

// Lua version of the accelerate function.
// Since there is no acceleration, we simply
// and return the input scene unmodified.
static int luaaccelerate(lua_State *L) {
    rvg_lua_push<rvg::driver::skia::accelerated>(L,
        rvg::driver::skia::accelerate(
            rvg_lua_check<rvg::scene>(L, 1),
            rvg_lua_check<rvg::window>(L, 2),
            rvg_lua_check<rvg::viewport>(L, 3)));
    return 1;
}

// Lua version of render function
static int luarender(lua_State *L) {
    auto a = rvg_lua_check<rvg::driver::skia::accelerated>(L, 1);
    auto w = rvg_lua_check<rvg::window>(L, 2);
    auto v = rvg_lua_check<rvg::viewport>(L, 3);
    auto o = rvg_lua_optargs(L, 5);
    rvg::driver::skia::render(a, w, v, rvg_lua_check_file(L, 4), o);
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
int luaopen_driver_skia(lua_State *L) {
    rvg_lua_init(L);
    if (!rvg_lua_typeexists<rvg::driver::skia::accelerated>(L, -1)) {
        rvg_lua_createtype<rvg::driver::skia::accelerated>(L,
            "skia accelerated", -1);
    }
    rvg_lua_facade_new_driver(L, modrg);
    return 1;
}
