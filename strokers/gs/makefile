UNAME:=$(shell uname)

CXX_Linux=g++
CXX_Darwin=g++
CXX:=$(CXX_$(UNAME))

PKG:=pkg-config
GSINC:=$(shell $(PKG) --cflags ghostscript-stroker)
GSLIB:=$(shell $(PKG) --libs ghostscript-stroker)

INC=-I../.. $(GSINC)
DEF=

CXXFLAGS=$(DEF) $(INC) -Ofast -std=c++14 -Wall -W -pedantic -fPIC
CFLAGS=$(DEF) $(INC) -Ofast -Wall -W -pedantic -fPIC

LIBS=$(GSLIB) -ldl

OBJS:= main.o \
	rvg-gs.o \
	rvg-stroker-gs.o \
       ../rvg-stroker-rvg.o  \
       ../../rvg-path-data.o  \
       ../../rvg-shape.o  \
       ../../rvg-xform-svd.o \
       ../../rvg-util.o \
       ../../rvg-gaussian-quadrature.o \
       ../../rvg-stroke-style.o

main: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -o $@ -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

clean:
	\rm -f $(OBJS)
