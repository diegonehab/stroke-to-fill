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
#ifndef RVG_GS
#define RVG_GS

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const int rvg_gs_cap_butt;
extern const int rvg_gs_cap_round;
extern const int rvg_gs_cap_square;
extern const int rvg_gs_cap_triangle;
extern const int rvg_gs_join_bevel;
extern const int rvg_gs_join_miter;
extern const int rvg_gs_join_round;
extern const int rvg_gs_pe_moveto;
extern const int rvg_gs_pe_lineto;
extern const int rvg_gs_pe_gapto;
extern const int rvg_gs_pe_closepath;
extern const int rvg_gs_pe_curveto;
extern const size_t rvg_gs_fixed_point_sizeof;

extern const int rvg_gx_pco_none;
extern const int rvg_gx_pco_monotonize;
extern const int rvg_gx_pco_accurate;
extern const int rvg_gx_pco_for_stroke;
extern const int rvg_gx_pco_small_curves;

void rvg_gx_path_add_line(void *vpath, float x1, float y1);
void rvg_gx_path_add_curve(void *vpath, float x1, float y1, float x2, float y2,
    float x3, float y3);
void rvg_gx_path_add_point(void *vpath, float x0, float y0);
void rvg_gx_path_close_subpath(void *vpath);
void *rvg_gs_malloc_init(void);
void rvg_gs_copydevice(void *vpdev, const void *vdev, void *vpmem);
const void *rvg_gs_getdefaultdevice(void);
void *rvg_gs_gstate_alloc(void *vpmem);
void rvg_gs_setcpsimode(void *vpmem, int value);
void *rvg_gx_path_alloc(void *vpmem);
void rvg_gs_setlinewidth(void *vpgs, float width);
void rvg_gs_setlinecap(void *pgs, int cap);
void rvg_gs_setlinejoin(void *vpgs, int join);
void rvg_gs_setdash(void *vpgs, const float *dashes, int ndashes, float phase);
void rvg_gx_stroke_path_only(void *vinput, void *voutput, void *vpdev,
    void *vpgs, float flatness, int traditional);
void *rvg_gs_path_enum_alloc(void *vpmem);
void rvg_gx_path_enum_init(void *vcenum, void *vpath);
void rvg_gs_fixed_point_get(const void *vvs, int idx, float *x, float *y);
int rvg_gx_path_enum_next(void *vcenum, void *vvs);
void rvg_gx_path_free(void *vpath);
void rvg_gs_gstate_free(void *vpgs);
void rvg_gs_malloc_release(void *vpmem);

enum {
    RVG_STROKER_GS_FAST = 0,
    RVG_STROKER_GS_COMPAT = 1,
    RVG_STROKER_GS = 2
};

#ifdef __cplusplus
}
#endif

#endif
