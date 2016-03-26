KServer: main.o kserver.o KSPHP.o logger.o
	g++ -o KServer main.o kserver.o KSPHP.o logger.o
main.o: main.cpp kserver.h
	g++ -c main.cpp
kserver.o: kserver.cpp kserver.h system.h config.h
	g++ -c kserver.cpp
KSPHP.o: KSPHP.cpp KSPHP.h system.h config.h
	g++ -c KSPHP.cpp
logger.o: logger.cpp logger.h system.h config.h
	g++ -c logger.cpp

clean:
	rm KServer *.o
