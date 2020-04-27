UNAME:=$(shell uname)

CXX_Linux=g++
CXX_Darwin=g++
CC_Linux=gcc
CC_Darwin=gcc
CXX:=$(CXX_$(UNAME))
CC:=$(CC_$(UNAME))

PKG:=pkg-config

LIVAROTINC:=$(shell $(PKG) --cflags livarot-stroker)
LIVAROTLIB:=$(shell $(PKG) --libs livarot-stroker)

LIBS=$(LIVAROTLIB)
DEF=
WAR=
INC=-I../.. $(LIVAROTINC)
CXXFLAGS=$(INC) $(DEF) -O2 -std=c++14 -Wall -W -pedantic -fpic
CFLAGS=$(INC) $(DEF) -O2 -Wall -W -pedantic -fpic

OBJS:= main.o \
       rvg-stroker-livarot.o \
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
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	\rm -f $(OBJS)
