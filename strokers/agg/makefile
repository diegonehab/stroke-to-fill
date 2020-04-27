UNAME:=$(shell uname)

CXX_Linux=g++
CXX_Darwin=g++
CC_Linux=gcc
CC_Darwin=gcc
CXX:=$(CXX_$(UNAME))
CC:=$(CC_$(UNAME))

PKG:=pkg-config
AGGINC:=$(shell $(PKG) --cflags agg-stroker)
AGGLIB:=$(shell $(PKG) --libs agg-stroker)

LIBS=$(AGGLIB)
DEF=
WAR=
INC=-I../.. $(AGGINC)
CXXFLAGS=$(INC) $(DEF) -Ofast -std=c++14 -Wall -W -pedantic
CFLAGS=$(INC) $(DEF) -Ofast -Wall -W -pedantic

OBJS:= main.o \
       rvg-stroker-agg.o \
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
