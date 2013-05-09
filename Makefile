
CFLAGS=-std=c++11 -Wall -pedantic -ggdb3

%.o: %.cpp %.h
	$(CXX) -c $(CFLAGS) $^

all: aeq

aeq: main.cpp eq.o ui.o knob.o
	g++ $(CFLAGS) -o $@ $^ `pkg-config --cflags --libs jack sndfile ncurses fftw3f` -lm

clean:
	rm -rf *.o aeq *.gch
