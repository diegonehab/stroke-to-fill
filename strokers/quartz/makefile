UNAME:=$(shell uname)

CXX_Linux=g++
CXX_Darwin=clang++
CXX:=$(CXX_$(UNAME))

INC=-I../.. -I/opt/local/include
DEF=

CXXFLAGS=$(DEF) $(INC) -Ofast -std=c++14 -Wall -W -pedantic -fPIC
CFLAGS=$(DEF) $(INC) -Ofast -Wall -W -pedantic -fPIC

LIBS=-framework CoreGraphics

OBJS:= main.o rvg-stroker-quartz.o ../../rvg-path.o  ../../rvg-xform-svd.o ../../rvg-stroke-style.o

main: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -o $@ -c $<

%.o: %.mm
	$(CXX) $(CXXFLAGS) $(INC) -o $@ -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

clean:
	\rm -f $(OBJS)
