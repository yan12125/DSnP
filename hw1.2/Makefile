# global settings
ifdef NO_DEBUG
	CC=g++
else
	CC=g++ -g
endif

p1a: hw1.2.p1a.o hw1.2.p1a.main.o
	${CC} -o hw1.2.p1a hw1.2.p1a.o hw1.2.p1a.main.o; \
	./hw1.2.p1a > hw1.2.p1a.out

hw1.2.p1a.o: hw1.2.p1a.cpp hw1.2.p1a.h
	${CC} -c  hw1.2.p1a.cpp

hw1.2.p1a.main.o: hw1.2.p1a.main.cpp hw1.2.p1a.h
	${CC} -c  hw1.2.p1a.main.cpp

p1b: hw1.2.p1b.o hw1.2.p1b.main.o hw1.2.p1a.o
	${CC} -o hw1.2.p1b hw1.2.p1b.o hw1.2.p1b.main.o hw1.2.p1a.o; \
	./hw1.2.p1b > hw1.2.p1b.out

hw1.2.p1b.o: hw1.2.p1b.cpp hw1.2.p1b.h
	${CC} -c  hw1.2.p1b.cpp

hw1.2.p1b.main.o: hw1.2.p1b.main.cpp hw1.2.p1b.h
	${CC} -c  hw1.2.p1b.main.cpp

bug: hw1.2.p1.bug.o hw1.2.p1a.o
	${CC} -o hw1.2.p1.bug hw1.2.p1.bug.o hw1.2.p1a.o

hw1.2.p1.bug.o: hw1.2.p1.bug.cpp hw1.2.p1a.h
	${CC} -c  hw1.2.p1.bug.cpp

p2: hw1.2.p2a.o hw1.2.p2b.o
	${CC} -o hw1.2.p2a hw1.2.p2a.o
	${CC} -o hw1.2.p2b hw1.2.p2b.o

hw1.2.p2a.o: hw1.2.p2a.cpp
	${CC} -c hw1.2.p2a.cpp

hw1.2.p2b.o: hw1.2.p2b.cpp
	${CC} -c hw1.2.p2b.cpp

p3: hw1.2.p3a.o hw1.2.p3b.o hw1.2.p3c.o
	${CC} -o hw1.2.p3a hw1.2.p3a.o
	${CC} -o hw1.2.p3b hw1.2.p3b.o
	${CC} -o hw1.2.p3c hw1.2.p3c.o

hw1.2.p3a.o: hw1.2.p3a.cpp
	${CC} -c hw1.2.p3a.cpp

hw1.2.p3b.o: hw1.2.p3b.cpp
	${CC} -c hw1.2.p3b.cpp

hw1.2.p3c.o: hw1.2.p3c.cpp
	${CC} -c hw1.2.p3c.cpp

all: p1a p1b bug p2 p3

clean:
	rm -f hw1.2.p1a hw1.2.p1b hw1.2.p2a hw1.2.p2b hw1.2.p3a hw1.2.p3b hw1.2.p3c hw1.2.p1.bug *.o
