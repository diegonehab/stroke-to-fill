# Stroke-to-fill conversion program and test harness
# Copyright (C) 2020 Diego Nehab
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# Contact information: diego.nehab@gmail.com
#
UNAME:=$(shell uname)

# ??D Separate intermediary and output files by UNAME?

#DEF?=-DRVG_THICKEN_WITH_CUBICS

SOLDFLAGS_Darwin:= -L/opt/local/lib -L/opt/local/lib/libomp -bundle -undefined dynamic_lookup
INC_Darwin= -I/opt/local/include
CP_Darwin=gcp
CC_Darwin=clang-mp-9.0
CXX_Darwin=clang++-mp-9.0
LP_INC_Darwin=$(shell PKG_CONFIG_PATH=/opt/local/lib/lapack/pkgconfig $(PKG) --cflags lapacke)
LP_LIB_Darwin=$(shell PKG_CONFIG_PATH=/opt/local/lib/lapack/pkgconfig $(PKG) --libs lapack)

SOLDFLAGS_Linux:= -shared -fpic
CP_Linux=cp
CC_Linux=gcc
CXX_Linux=g++
LP_INC_Linux=$(shell $(PKG) --cflags lapacke)
LP_LIB_Linux=$(shell $(PKG) --libs lapack)

#---
# Subset of targets to build
#---
vg_build_strokers?=no
vg_build_cairo?=no
vg_build_qt5?=no
vg_build_skia?=no
vg_build_distroke?=no
vg_build_nvpr?=no
vg_build_tests?=no
vg_build_gperftools?=no

#---
# Probably no need to change anything below here
#---
CP=$(CP_$(UNAME))
CC=$(CC_$(UNAME))
CXX=$(CXX_$(UNAME))

# Load includes and libs from pkg-config
PKG:=pkg-config

ifeq ($(vg_build_luajit),yes)
LUA_INC:=$(shell $(PKG) --cflags luajit)
else
V?=53
LUA_INC:=$(shell $(PKG) --cflags luapp$V) -DRVG_LUAPP
endif

FT_INC:=$(shell $(PKG) --cflags freetype2)
FT_LIB:=$(shell $(PKG) --libs freetype2)
PNG_INC:=$(shell $(PKG) --cflags libpng16)
PNG_LIB:=$(shell $(PKG) --libs libpng16)
B64_LIB=$(shell $(PKG) --libs b64)
B64_INC=$(shell $(PKG) --cflags b64)
HB_LIB=$(shell $(PKG) --libs harfbuzz-icu)
HB_INC=$(shell $(PKG) --cflags harfbuzz-icu)
#LP_INC=$(LP_INC_$(UNAME))
#LP_LIB=$(LP_LIB_$(UNAME))

ifeq ($(vg_build_gperftools),yes)
GPERFTOOLS_LIB=-lprofiler
endif

OMP_LIB= -fopenmp -lgomp

ifeq ($(vg_build_cairo),yes)
CAIRO_LIB=$(shell $(PKG) --libs cairo)
CAIRO_INC=$(shell $(PKG) --cflags cairo)
endif

ifeq ($(vg_build_qt5),yes)
QT5_INC=$(shell $(PKG) --cflags Qt5Gui)
QT5_LIB=$(shell $(PKG) --libs Qt5Gui)
endif

ifeq ($(vg_build_skia),yes)
SKIA_INC=$(shell $(PKG) --cflags skia)
SKIA_LIB=$(shell $(PKG) --libs skia)
endif

EGL_LIB:=-lGLEW -lEGL -lGL
EGL_INC:=

strokers/rvg-lua-strokers.o: INC += $(LUA_INC)

ST_OBJ:=
ST_LIB:= $(B64_LIB) $(PNG_LIB)

ST_DEF:= -DSTROKER_RVG
ST_RVG_OBJ:= strokers/rvg-stroker-rvg.o
ST_OBJ+= $(ST_RVG_OBJ)

ifeq ($(vg_build_strokers),yes)
ST_DEF+= -DSTROKER_AGG
ST_AGG_INC:=$(shell $(PKG) --cflags agg-stroker)
ST_AGG_LIB:=$(shell $(PKG) --libs agg-stroker)
ST_AGG_OBJ:= strokers/agg/rvg-stroker-agg.o
ST_OBJ+= $(ST_AGG_OBJ)
ST_LIB+= $(ST_AGG_LIB)

ST_DEF+= -DSTROKER_CAIRO
ST_CAIRO_LIB=$(shell $(PKG) --libs cairo-stroker)
ST_CAIRO_INC=$(shell $(PKG) --cflags cairo-stroker)
ST_CAIRO_OBJ:= strokers/cairo/rvg-stroker-cairo.o
ST_OBJ+= $(ST_CAIRO_OBJ)
ST_LIB+= $(ST_CAIRO_LIB)

ST_DEF+= -DSTROKER_GS
ST_GS_LIB=$(shell $(PKG) --libs ghostscript-stroker)
ST_GS_INC=$(shell $(PKG) --cflags ghostscript-stroker)
ST_GS_OBJ:= strokers/gs/rvg-stroker-gs.o strokers/gs/rvg-gs.o
ST_OBJ+= $(ST_GS_OBJ)
ST_LIB+= $(ST_GS_LIB)

ST_DEF+= -DSTROKER_MUPDF
ST_MUPDF_LIB=$(shell $(PKG) --libs mupdf-stroker)
ST_MUPDF_INC=$(shell $(PKG) --cflags mupdf-stroker)
ST_MUPDF_OBJ:= strokers/mupdf/rvg-stroker-mupdf.o
ST_OBJ+= $(ST_MUPDF_OBJ)
ST_LIB+= $(ST_MUPDF_LIB)

ST_DEF+= -DSTROKER_QT5
ST_QT5_INC:=$(shell $(PKG) --cflags Qt5Gui)
ST_QT5_LIB:=$(shell $(PKG) --libs Qt5Gui)
ST_QT5_OBJ:= strokers/qt5/rvg-stroker-qt5.o
ST_OBJ+= $(ST_QT5_OBJ)
ST_LIB+= $(ST_QT5_LIB)

ST_DEF+= -DSTROKER_SKIA
ST_SKIA_INC:=$(shell $(PKG) --cflags skia)
ST_SKIA_LIB:=$(shell $(PKG) --libs skia)
ST_SKIA_OBJ:= strokers/skia/rvg-stroker-skia.o
ST_OBJ+= $(ST_SKIA_OBJ)
ST_LIB+= $(ST_SKIA_LIB)

ST_DEF+= -DSTROKER_LIVAROT
ST_LIVAROT_INC:=$(shell $(PKG) --cflags livarot-stroker)
ST_LIVAROT_LIB:=$(shell $(PKG) --libs livarot-stroker)
ST_LIVAROT_OBJ:= strokers/livarot/rvg-stroker-livarot.o
ST_OBJ+= $(ST_LIVAROT_OBJ)
ST_LIB+= $(ST_LIVAROT_LIB)

ST_DEF+= -DSTROKER_OPENVG_RI
ST_OPENVGRI_OBJ:= strokers/openvg-ri/rvg-stroker-openvg-ri.o strokers/openvg-ri/riMath.o strokers/openvg-ri/riPath.o
ST_OBJ+= $(ST_OPENVGRI_OBJ)

ifeq ($(UNAME),Darwin)
ST_DEF+= -DSTROKER_QUARTZ
ST_QUARTZ_OBJ:= strokers/quartz/rvg-stroker-quartz.o
ST_OBJ+= $(ST_QUARTZ_OBJ)
ST_LIB+= -framework CoreGraphics
endif
endif

# Automatically generate dependencies
DEPFLAGS=-MT $@ -MMD -MP -MF $*.d
CXXFLAGS=-fopenmp -O2 -g -W -std=c++14 -pedantic -Wall -fpic -fvisibility=hidden $(DEPFLAGS) $(ST_DEF)
#CXXFLAGS+=-ftemplate-backtrace-limit=0
SOLDFLAGS:=$(SOLDFLAGS_$(UNAME))

INC := -I./

rvg-lua-image.o: INC += $(LUA_INC)
rvg-lua.o: INC += $(LUA_INC)
rvg-lua-facade.o: INC += $(LUA_INC)
rvg-lua-path-data.o: INC += $(LUA_INC)
rvg-lua-filter.o: INC += $(LUA_INC)
rvg-lua-path-filters.o: INC += $(LUA_INC)
rvg-lua-scene-filters.o: INC += $(LUA_INC) -Istrokers
rvg-lua-scene-data.o: INC += $(LUA_INC)
rvg-lua-triangle-data.o: INC += $(LUA_INC)
rvg-lua-circle-data.o: INC += $(LUA_INC)
rvg-lua-rect-data.o: INC += $(LUA_INC)
rvg-lua-polygon-data.o: INC += $(LUA_INC)
rvg-lua-patch-data.o: INC += $(LUA_INC)
rvg-lua-patch.o: INC += $(LUA_INC)
rvg-lua-xform.o: INC += $(LUA_INC)
rvg-lua-bbox.o: INC += $(LUA_INC)
rvg-lua-stroke-style.o: INC += $(LUA_INC)
rvg-lua-shape.o: INC += $(LUA_INC)
rvg-lua-rgba.o: INC += $(LUA_INC)
rvg-lua-spread.o: INC += $(LUA_INC)
rvg-lua-winding-rule.o: INC += $(LUA_INC)
rvg-lua-color-ramp.o: INC += $(LUA_INC)
rvg-lua-linear-gradient-data.o: INC += $(LUA_INC)
rvg-lua-radial-gradient-data.o: INC += $(LUA_INC)
rvg-lua-texture-data.o: INC += $(LUA_INC)
rvg-lua-paint.o: INC += $(LUA_INC)
rvg-lua-scene.o: INC += $(LUA_INC)
rvg-xform-svd.o: INC += $(LP_INC)

rvg-lua-chronos.o: INC += $(LUA_INC)
rvg-lua-util.o: INC += $(LUA_INC)
ifeq ($(vg_build_gperftools),yes)
rvg-lua-util.o: DEF += -DUSE_GPERFTOOLS
endif
rvg-lua-base64.o: INC += $(LUA_INC) $(B64_INC)
rvg-lua-harfbuzz.o: INC += $(LUA_INC) $(FT_INC) $(HB_INC)
rvg-lua-freetype.o: INC += $(LUA_INC) $(FT_INC)
rvg-lua-freetype-typeface.o: INC += $(LUA_INC) $(FT_INC)

rvg-pngio.o: INC += $(PNG_INC)
rvg-base64.o: INC += $(B64_INC)
rvg-util.o: INC += $(LP_INC)

rvg-driver-distroke.o: INC += $(LUA_INC)
rvg-driver-svg.o: INC += $(LUA_INC)
rvg-driver-eps.o: INC += $(LUA_INC)
rvg-driver-rvg-lua.o: INC += $(LUA_INC)
rvg-driver-rvg-cpp.o: INC += $(LUA_INC)
rvg-driver-nvpr.o: INC += $(LUA_INC) $(EGL_INC)
rvg-driver-cairo.o: INC += $(LUA_INC) $(CAIRO_INC)
rvg-driver-qt5.o: INC += $(LUA_INC) $(QT5_INC)
rvg-driver-skia.o: INC += $(LUA_INC) $(SKIA_INC)

rvg-freetype.o: INC += $(FT_INC)
test-text.o: INC += $(FT_INC)

strokers/livarot/rvg-stroker-livarot.o: INC += $(ST_LIVAROT_INC)
strokers/cairo/rvg-stroker-cairo.o: INC += $(ST_CAIRO_INC)
strokers/gs/rvg-gs.o: INC += $(ST_GS_INC)
strokers/qt5/rvg-stroker-qt5.o: INC += $(ST_QT5_INC)
strokers/mupdf/rvg-stroker-mupdf.o: INC += $(ST_MUPDF_INC)
strokers/skia/rvg-stroker-skia.o: INC += $(ST_SKIA_INC)
strokers/agg/rvg-stroker-agg.o: INC += $(ST_AGG_INC)

DRV_OBJ := rvg-lua-base64.o \
	rvg-base64.o \
	rvg-lua-image.o \
	rvg-pngio.o \
	rvg-xform.o \
	rvg-xform-svd.o \
	rvg-path-data.o \
	rvg-shape.o \
	rvg-svg-path-commands.o \
	rvg-svg-path-token.o \
	rvg-unorm.o \
	rvg-util.o \
	rvg-stroke-style.o \
	rvg-named-colors.o \
	rvg-facade-scene-data.o \
	rvg-gaussian-quadrature.o \
	rvg-lua-facade.o \
	rvg-lua-scene-data.o \
	rvg-lua-path-data.o \
	rvg-lua-triangle-data.o \
	rvg-lua-polygon-data.o \
	rvg-lua-circle-data.o \
	rvg-lua-rect-data.o \
	rvg-lua-patch-data.o \
	rvg-lua-patch.o \
	rvg-lua-xform.o \
	rvg-lua-bbox.o \
	rvg-lua-stroke-style.o \
	rvg-lua-shape.o \
	rvg-lua-rgba.o \
	rvg-lua-winding-rule.o \
	rvg-lua-spread.o \
	rvg-lua-color-ramp.o \
	rvg-lua-linear-gradient-data.o \
	rvg-lua-radial-gradient-data.o \
	rvg-lua-texture-data.o \
	rvg-lua-paint.o \
	rvg-lua-scene.o \
	rvg-lua.o \
	strokers/rvg-stroker-rvg.o

SO_STROKERS_OBJ := strokers/rvg-lua-strokers.o $(DRV_OBJ) $(ST_OBJ)
SO_BASE64_OBJ:= rvg-lua-base64.o rvg-base64.o rvg-lua.o
SO_UTIL_OBJ:= rvg-lua-util.o rvg-lua.o rvg-xform.o rvg-xform-svd.o
SO_CHRONOS_OBJ:= rvg-lua-chronos.o rvg-chronos.o rvg-lua.o
SO_IMAGE_OBJ:= rvg-lua-image.o rvg-pngio.o rvg-lua.o
SO_FACADE_OBJ:= $(DRV_OBJ)

SO_FREETYPE_OBJ:= \
	rvg-freetype.o \
	rvg-lua-freetype.o \
	rvg-lua-freetype-typeface.o \
	rvg-lua-path-data.o \
	rvg-path-data.o \
	rvg-lua.o

SO_FILTER_OBJ:= \
	rvg-lua-filter.o \
	rvg-lua-path-filters.o \
	rvg-lua-scene-filters.o \
	$(DRV_OBJ) \
	$(ST_OBJ)

SO_HARFBUZZ_OBJ:= rvg-lua-harfbuzz.o rvg-lua.o

SO_SVG_DRV_OBJ:= rvg-driver-svg.o $(DRV_OBJ)
SO_EPS_DRV_OBJ:= rvg-driver-eps.o $(DRV_OBJ)
SO_NVPR_DRV_OBJ:= rvg-driver-nvpr.o $(DRV_OBJ)
SO_RVG_LUA_DRV_OBJ:= rvg-driver-rvg-lua.o rvg-scene-f-print-rvg.o $(DRV_OBJ)
SO_RVG_CPP_DRV_OBJ:= rvg-driver-rvg-cpp.o rvg-scene-f-print-rvg.o $(DRV_OBJ)
SO_CAIRO_DRV_OBJ:= rvg-driver-cairo.o $(DRV_OBJ)
SO_QT5_DRV_OBJ:= rvg-driver-qt5.o $(DRV_OBJ)
SO_SKIA_DRV_OBJ:= rvg-driver-skia.o $(DRV_OBJ)
SO_DISTROKE_DRV_OBJ:= rvg-driver-distroke.o $(DRV_OBJ)

OBJ:= \
	$(SO_BASE64_OBJ) \
	$(SO_UTIL_OBJ) \
	$(SO_CHRONOS_OBJ) \
	$(SO_IMAGE_OBJ) \
	$(SO_FACADE_OBJ) \
	$(SO_FREETYPE_OBJ) \
	$(SO_HARFBUZZ_OBJ) \
	$(SO_FILTER_OBJ) \
	$(SO_SVG_DRV_OBJ) \
	$(SO_EPS_DRV_OBJ) \
	$(SO_RVG_CPP_DRV_OBJ) \
	$(SO_RVG_LUA_DRV_OBJ) \
    $(SO_STROKERS_OBJ)

TARGETS:= \
	driver/rvg_cpp.so \
	driver/rvg_lua.so \
	driver/eps.so \
	driver/svg.so \
	facade.so \
	filter.so \
	harfbuzz.so \
	freetype.so \
	image.so \
	base64.so \
	util.so \
	chronos.so \
	strokers.so

ifeq ($(vg_build_nvpr),yes)
OBJ += $(SO_NVPR_DRV_OBJ)
TARGETS += driver/nvpr.so
endif

ifeq ($(vg_build_cairo),yes)
OBJ+= $(SO_CAIRO_DRV_OBJ)
TARGETS += driver/cairo.so
endif

ifeq ($(vg_build_qt5),yes)
OBJ+= $(SO_QT5_DRV_OBJ)
TARGETS += driver/qt5.so
endif

ifeq ($(vg_build_skia),yes)
OBJ+= $(SO_SKIA_DRV_OBJ)
TARGETS += driver/skia.so
endif

ifeq ($(vg_build_distroke),yes)
OBJ += $(SO_DISTROKE_DRV_OBJ)
TARGETS += driver/distroke.so
endif

OBJ:=$(sort $(OBJ))

# The dependency file for each each OBJ
DEP:=$(OBJ:.o=.d)

# Do not delete dependency files
.PRECIOUS: $(DEP)

# Targets
all: $(TARGETS)

GPPOPT:= -U "" "" "(" "," ")" "(" ")" "$$" "\\" -M "$$" "\n" " " " " "\n" "(" ")" +c "--" "\n" +c "--[[" "]]" +c "--[=[" "]=]" +c "--[==[" "]==]" +s "\"" "\"" "\\" +s "[[" "]]" "" +s "[=[" "]=]" "" +s "[==[" "]==]" ""

# Don't worry if dependency files are not found
$(DEP): ;

# Include all dependency files to make them active
-include $(DEP)

strokers.so: $(SO_STROKERS_OBJ)
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(ST_LIB) $(LP_LIB)

base64.so: $(SO_BASE64_OBJ)
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(B64_LIB)

util.so: $(SO_UTIL_OBJ)
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(LP_LIB) $(GPERFTOOLS_LIB)

chronos.so: $(SO_CHRONOS_OBJ)
	$(CXX) $(SOLDFLAGS) -o $@ $^

image.so: $(SO_IMAGE_OBJ)
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(PNG_LIB)

facade.so: $(SO_FACADE_OBJ)
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(PNG_LIB) $(B64_LIB)

freetype.so: $(SO_FREETYPE_OBJ)
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(FT_LIB)

harfbuzz.so: $(SO_HARFBUZZ_OBJ)
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(HB_LIB)

filter.so: $(SO_FILTER_OBJ)
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(ST_LIB)

driver/distroke.so: $(SO_DISTROKE_DRV_OBJ)
	mkdir -p driver
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(PNG_LIB) $(B64_LIB) $(OMP_LIB) $(LP_LIB)

driver/svg.so: $(SO_SVG_DRV_OBJ)
	mkdir -p driver
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(PNG_LIB) $(B64_LIB) $(LP_LIB)

driver/eps.so: $(SO_EPS_DRV_OBJ)
	mkdir -p driver
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(PNG_LIB) $(B64_LIB) $(LP_LIB)

driver/rvg_lua.so: $(SO_RVG_LUA_DRV_OBJ)
	mkdir -p driver
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(PNG_LIB) $(B64_LIB) $(LP_LIB)

driver/rvg_cpp.so: $(SO_RVG_CPP_DRV_OBJ)
	mkdir -p driver
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(PNG_LIB) $(B64_LIB) $(LP_LIB)

driver/nvpr.so: $(SO_NVPR_DRV_OBJ)
	mkdir -p driver
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(PNG_LIB) $(EGL_LIB) $(B64_LIB) $(LP_LIB)

driver/cairo.so: $(SO_CAIRO_DRV_OBJ)
	mkdir -p driver
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(PNG_LIB) $(B64_LIB) $(CAIRO_LIB) $(LP_LIB)

driver/qt5.so: $(SO_QT5_DRV_OBJ)
	mkdir -p driver
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(PNG_LIB) $(B64_LIB) $(QT5_LIB) $(LP_LIB)

driver/skia.so: $(SO_SKIA_DRV_OBJ)
	mkdir -p driver
	$(CXX) $(SOLDFLAGS) -o $@ $^ $(PNG_LIB) $(B64_LIB) $(SKIA_LIB) $(LP_LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) $(DEF) $(INC_$(UNAME)) -o $@ -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INC) $(DEF) $(INC_$(UNAME)) -o $@ -c $<

%.o: %.mm
	$(CXX) $(CXXFLAGS) $(INC) $(DEF) $(INC_$(UNAME)) -o $@ -c $<

.PHONY: clean config

clean:
	\rm -rf $(OBJ) $(DEP) $(TARGETS)

# Show paths
config:
	@echo LUA_INC: $(LUA_INC)
	@echo FT_INC: $(FT_INC)
	@echo FT_LIB: $(FT_LIB)
	@echo PNG_INC: $(PNG_INC)
	@echo PNG_LIB: $(PNG_LIB)
	@echo B64_LIB: $(B64_LIB)
	@echo B64_INC: $(B64_INC)
	@echo HB_LIB: $(HB_LIB)
	@echo HB_INC: $(HB_INC)
	@echo LP_LIB: $(LP_LIB)
	@echo LP_INC: $(LP_INC)


.PHONY: test-stroker replicability

replicability:
	docker build fig-17 -t fig-17 && docker run -v `pwd`:/replicate --rm fig-17 /bin/bash -c '/stroke-to-fill/fig-17/fig-17.sh && cp /stroke-to-fill/fig-17*.svg /replicate'

STROKER?=rvg

GT_DIR := a/distroke

GT := $(addsuffix _distroke.png,$(addprefix $(GT_DIR)/, $(shell ./test-strokers.lua -test:list)))

$(GT_DIR):
	mkdir -p $(GT_DIR)

$(GT_DIR)/%_distroke.png: $(GT_DIR)
	./test-strokers.lua -stroker:native -driver:distroke -test:$(notdir $(@:_distroke.png=)) > $@

test-stroker: strokers.so $(GT)
	strokers/stroke-all $(STROKER)
	for t in a/distroke/*distroke.png; do \
		s=$$(basename $$t _distroke.png); \
		compare -fuzz 20% -metric rmse $$t a/$${s}.png a/$${s}_compare.png 2>&1; \
		echo " "$$s; \
	done | sort -n | tail;

stats-stroker: strokers.so
	@\rm -f a/*png;
	@strokers/stroke-all $(STROKER) -no-background -driver:stats
	@for t in linear_segment quadratic_segment rational_quadratic_segment cubic_segment; do \
		echo $${t}s; \
		cat a/*.png | grep \\\<$$t\\\> | cut -f 2 -d ' ' | awk '{s+=$$1} END {print s}'; \
	done
	@echo total; \
	cat a/*.png | grep segment | cut -f 2 -d ' ' | awk '{s+=$$1} END {print s}';
