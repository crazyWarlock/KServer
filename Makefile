KServer: main.o kserver.o
	g++ -o KServer main.o kserver.o

main.o: main.cpp kserver.h
	g++ -c main.cpp
kserver.o: kserver.cpp kserver.h system.h config.h
	g++ -c kserver.cpp

clean:
	rm KServer main.o kserver.o