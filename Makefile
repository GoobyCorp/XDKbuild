PROG = xdkbuild
CXX = g++
CFLAGS = -g -O3 -w -std=c++2a
CPPFLAGS = 
SRCS := $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

$(PROG): distclean debug
	# wat?!
all: clean debug
	# wat?!
debug: $(OBJS)
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $(PROG) $(OBJS)
release: $(OBJS)
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $(PROG) $(OBJS)
	strip -s $(PROG)
main.o: stdafx.hpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c main.cpp
clean:
	rm -f *.o
distclean: clean
	rm -f $(PROG)
