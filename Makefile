
CFLAGS=-std=c++11 -Wall -pedantic -ggdb3 -msse -msse2 -msse3 -mfpmath=sse -ftree-vectorize -march=native -O3 

%.o: %.cpp %.h
	$(CXX) -c $(CFLAGS) $^

all: aeq

aeq: main.cpp eq.o ui.o knob.o logbuf.h
	g++ $(CFLAGS) -o $@ $^ `pkg-config --cflags --libs jack sndfile ncurses fftw3f` -lm

pkg:
	read -r -p "Zadej login: " login; \
	zip -r "$$login.zip" *.cpp *.h Makefile eqdump.txt README.pdf pila.wav

clean:
	rm -rf *.o aeq *.gch
