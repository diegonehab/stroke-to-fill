UNAME:=$(shell uname)

CXX_Linux=g++
CXX_Darwin=g++
CC_Linux=gcc
CC_Darwin=gcc
CXX:=$(CXX_$(UNAME))
CC:=$(CC_$(UNAME))

PKG:=pkg-config
CAIROINC:=$(shell $(PKG) --cflags cairo-stroker)
CAIROLIB:=$(shell $(PKG) --libs cairo-stroker)

LIBS=$(CAIROLIB)
DEF=
WAR=
INC=-I../.. $(CAIROINC)
CXXFLAGS=$(INC) $(DEF) -Ofast -std=c++14 -Wall -W -pedantic
CFLAGS=$(INC) $(DEF) -Ofast -Wall -W -pedantic

OBJS:= main.o \
	rvg-stroker-cairo.o \
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
