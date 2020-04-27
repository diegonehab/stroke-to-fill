UNAME:=$(shell uname)

CXX_Linux=g++
CXX_Darwin=g++
CXX:=$(CXX_$(UNAME))

PKG:=pkg-config
MUINC:=$(shell $(PKG) --cflags mupdf-stroker)
MULIB:=$(shell $(PKG) --libs mupdf-stroker)

INC=-I../.. $(MUINC)
DEF=

CXXFLAGS=$(DEF) $(INC) -Ofast -std=c++14 -Wall -W -pedantic -fPIC
CFLAGS=$(DEF) $(INC) -Ofast -Wall -W -pedantic -fPIC

LIBS=$(MULIB)

OBJS:= main.o \
       rvg-stroker-mupdf.o \
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
