all: rps rand_gen

main.o: main.cpp
	g++ -c main.cpp

rps: main.o
	g++ main.o -o rps

rand_gen.o: rand_gen.cpp
	g++ -c rand_gen.cpp

rand_gen: rand_gen.o
	g++ rand_gen.o -o rand_gen

clean:
	rm *.o rps rand_gen
