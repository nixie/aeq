
CFLAGS=-std=c++11 -Wall -pedantic -ggdb3

%.o: %.cpp %.h
	$(CXX) -c $(CFLAGS) $^

all: aeq

aeq: main.cpp eq.o ui.o knob.o
	g++ $(CFLAGS) -o $@ $^ -lcurses `pkg-config --cflags --libs jack sndfile` -lm

clean:
	rm -rf *.o aeq *.gch
