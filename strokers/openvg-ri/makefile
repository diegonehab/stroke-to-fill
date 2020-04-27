CXX=g++
CC=gcc

LIBS=
INC=-I../..
CXXFLAGS=$(INC) -Ofast -std=c++14 -Wall -W -pedantic
CFLAGS=$(INC) -Ofast -Wall -W -pedantic

OBJS:= main.o \
       riMath.o \
       riPath.o \
       rvg-stroker-openvg-ri.o \
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
