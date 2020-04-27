UNAME:=$(shell uname)

CXX_Linux=g++
CXX_Darwin=g++
CXX:=$(CXX_$(UNAME))

PKG:=pkg-config
SKIAINC:=$(shell $(PKG) --cflags skia)
SKIALIB:=$(shell $(PKG) --libs skia)

INC=-I../.. $(SKIAINC)

CXXFLAGS=$(INC) -Ofast -std=c++14 -Wall -W -pedantic
CFLAGS=$(INC) -Ofast -Wall -W -pedantic

LIBS=$(SKIALIB)

OBJS:= main.o \
       rvg-stroker-skia.o \
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
