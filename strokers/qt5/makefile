UNAME:=$(shell uname)

CXX_Linux=g++
CXX_Darwin=g++
CXX:=$(CXX_$(UNAME))

PKG:=pkg-config
QT5INC:=$(shell $(PKG) --cflags Qt5Gui)
QT5LIB:=$(shell $(PKG) --libs Qt5Gui)

INC=-I../.. $(QT5INC)
DEF=

CXXFLAGS=$(DEF) $(INC) -Ofast -std=c++14 -Wall -W -pedantic -fPIC
CFLAGS=$(DEF) $(INC) -Ofast -Wall -W -pedantic -fPIC

LIBS=$(QT5LIB)

OBJS:= main.o \
       rvg-stroker-qt5.o \
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
