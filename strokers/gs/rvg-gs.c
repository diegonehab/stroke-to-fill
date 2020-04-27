/* Stroke-to-fill conversion program and test harness
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
*/
#include "base/gsmalloc.h"
#include "base/gsstate.h"
#include "base/gspath.h"
#include "base/gxpath.h"
#include "base/gxpaint.h"
#include "base/gsdevice.h"
#include "base/gxdevice.h"
#include "base/gxgstate.h"
#include "base/gxfixed.h"

#include "rvg-gs.h"

const int rvg_gs_cap_butt = gs_cap_butt;
const int rvg_gs_cap_round = gs_cap_round;
const int rvg_gs_cap_square = gs_cap_square;
const int rvg_gs_cap_triangle = gs_cap_triangle;
const int rvg_gs_join_bevel = gs_join_bevel;
const int rvg_gs_join_miter = gs_join_miter;
const int rvg_gs_join_round = gs_join_round;
const int rvg_gs_pe_moveto = gs_pe_moveto;
const int rvg_gs_pe_lineto = gs_pe_lineto;
const int rvg_gs_pe_gapto = gs_pe_gapto;
const int rvg_gs_pe_closepath = gs_pe_closepath;
const int rvg_gs_pe_curveto = gs_pe_curveto;
const size_t rvg_gs_fixed_point_sizeof = sizeof(gs_fixed_point);

const int rvg_gx_pco_none = pco_none;
const int rvg_gx_pco_monotonize = pco_monotonize;
const int rvg_gx_pco_accurate = pco_accurate;
const int rvg_gx_pco_for_stroke = pco_for_stroke;
const int rvg_gx_pco_small_curves = pco_small_curves;

static const char *cname = "rvg::stroke::gs";

void rvg_gx_path_add_line(void *vpath, float x1, float y1) {
    gx_path_add_line((gx_path *) vpath, float2fixed(x1), float2fixed(y1));
}

void rvg_gx_path_add_curve(void *vpath, float x1, float y1, float x2, float y2,
    float x3, float y3) {
    gx_path_add_curve((gx_path *) vpath, float2fixed(x1), float2fixed(y1),
        float2fixed(x2), float2fixed(y2),
        float2fixed(x3), float2fixed(y3));
}

void rvg_gx_path_add_point(void *vpath, float x0, float y0) {
    gx_path_add_point((gx_path *) vpath, float2fixed(x0), float2fixed(y0));
}

void rvg_gx_path_close_subpath(void *vpath) {
    gx_path_close_subpath((gx_path *) vpath);
}

void *rvg_gs_malloc_init(void) {
    return gs_malloc_init();
}

void rvg_gs_copydevice(void *vpdev, const void *vdev, void *vpmem) {
    gs_copydevice((gx_device **) vpdev, (gx_device *) vdev, (gs_memory_t *) vpmem);
}

void *rvg_gs_gstate_alloc(void *vpmem) {
    return gs_gstate_alloc((gs_memory_t *) vpmem);
}

void rvg_gs_setcpsimode(void *vpmem, int value) {
    if (value) gs_setcpsimode((gs_memory_t *) vpmem, true);
    else gs_setcpsimode((gs_memory_t *) vpmem, false);
}

void *rvg_gx_path_alloc(void *vpmem) {
    return gx_path_alloc_shared(NULL, (gs_memory_t *) vpmem, cname);
}

void rvg_gs_setlinewidth(void *vpgs, float width) {
    gs_setlinewidth((gs_gstate *) vpgs, width);
}

void rvg_gs_setlinecap(void *vpgs, int cap) {
    gs_setlinecap((gs_gstate *) vpgs, cap);
}

void rvg_gs_setlinejoin(void *vpgs, int join) {
    gs_setlinejoin((gs_gstate *) vpgs, join);
}

void rvg_gs_setdash(void *vpgs, const float *dashes, int ndashes, float phase) {
    gs_setdash((gs_gstate *) vpgs, dashes, ndashes, phase);
}

void rvg_gx_add_flattened_for_stroke(void *vinput, void *voutput,
    float flatness, void *vpgs) {
    gx_path_copy_reducing((gx_path *) vinput, (gx_path *) voutput,
         float2fixed(flatness), (gs_gstate *) vpgs,
         rvg_gx_pco_accurate | rvg_gx_pco_for_stroke);
}

void rvg_gx_stroke_path_only(void *vinput, void *voutput, void *vpdev,
    void *vpgs, float flatness, int mode) {
    gx_stroke_params params;
    params.flatness = flatness;
    if (mode == RVG_STROKER_GS_COMPAT) params.traditional = true;
    else params.traditional = false;
    params.mode = mode;
    ((gs_gstate *) vpgs)->accurate_curves = true;
    ((gs_gstate *) vpgs)->stroke_adjust = false;
    gx_stroke_path_only((gx_path *) vinput, (gx_path *) voutput,
        (gx_device *) vpdev, (gs_gstate *) vpgs, &params, NULL, NULL);
}

void *rvg_gs_path_enum_alloc(void *vpmem) {
   return gs_path_enum_alloc((gs_memory_t *) vpmem, cname);
}

void rvg_gx_path_enum_init(void *vcenum, void *vpath) {
    gx_path_enum_init((gs_path_enum *) vcenum, (gx_path *) vpath);
}

void rvg_gs_fixed_point_get(const void *vvs, int idx, float *x, float *y) {
   const gs_fixed_point *vs = (const gs_fixed_point *) vvs;
    *x = fixed2float(vs[idx].x);
    *y = fixed2float(vs[idx].y);
}

int rvg_gx_path_enum_next(void *vcenum, void *vvs) {
    return gx_path_enum_next((gs_path_enum *) vcenum, (gs_fixed_point *) vvs);
}

void rvg_gx_path_free(void *vpath) {
    gx_path_free((gx_path *) vpath, cname);
}

void rvg_gs_gstate_free(void *vpgs) {
    gs_gstate_free((gs_gstate *) vpgs);
}

void rvg_gs_malloc_release(void *vpmem) {
    gs_malloc_release((gs_memory_t *) vpmem);
}

const void *rvg_gs_getdefaultdevice(void) {
    return gs_getdefaultdevice();
}
