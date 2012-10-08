# global settings
ifdef NO_DEBUG
	CC=g++
else
	CC=g++ -g
endif

.PHONY: clean all p1a p1b bug

p1a: hw1.2.p1a

hw1.2.p1a: hw1.2.p1a.o hw1.2.p1a.main.o
	${CC} -o hw1.2.p1a hw1.2.p1a.o hw1.2.p1a.main.o; \
	./hw1.2.p1a > hw1.2.p1a.out

hw1.2.p1a.o: hw1.2.p1a.cpp hw1.2.p1a.h
	${CC} -c  hw1.2.p1a.cpp

hw1.2.p1a.main.o: hw1.2.p1a.main.cpp hw1.2.p1a.h
	${CC} -c  hw1.2.p1a.main.cpp

p1b: hw1.2.p1b

hw1.2.p1b: hw1.2.p1b.o hw1.2.p1b.main.o hw1.2.p1a.o
	${CC} -o hw1.2.p1b hw1.2.p1b.o hw1.2.p1b.main.o hw1.2.p1a.o; \
	./hw1.2.p1b > hw1.2.p1b.out

hw1.2.p1b.o: hw1.2.p1b.cpp hw1.2.p1b.h
	${CC} -c  hw1.2.p1b.cpp

hw1.2.p1b.main.o: hw1.2.p1b.main.cpp hw1.2.p1b.h
	${CC} -c  hw1.2.p1b.main.cpp

bug: hw1.2.p1.bug

hw1.2.p1.bug: hw1.2.p1.bug.o hw1.2.p1a.o
	${CC} -o hw1.2.p1.bug hw1.2.p1.bug.o hw1.2.p1a.o

hw1.2.p1.bug.o: hw1.2.p1.bug.cpp hw1.2.p1a.h
	${CC} -c  hw1.2.p1.bug.cpp

all: p1a p1b bug

clean:
	rm -f hw1.2.p1a hw1.2.p1b hw1.2.p1.bug *.o
