
BINARIES = main-two-cvs-while.o main-two-cvs-if.o main-one-cv-while.o main-two-cvs-while-extra-unlock.o
HEADERS = common.h common_threads.h main-header.h main-common.c pc-header.h

all: $(BINARIES)

clean:
	rm -f $(BINARIES)

main-one-cv-while.o: main-one-cv-while.c $(HEADERS)
	gcc -o main-one-cv-while.o main-one-cv-while.c -Wall -pthread

main-two-cvs-if.o: main-two-cvs-if.c $(HEADERS)
	gcc -o main-two-cvs-if.o main-two-cvs-if.c -Wall -pthread

main-two-cvs-while.o: main-two-cvs-while.c $(HEADERS)
	gcc -o main-two-cvs-while.o main-two-cvs-while.c -Wall -pthread

main-two-cvs-while-extra-unlock.o: main-two-cvs-while-extra-unlock.c $(HEADERS)
	gcc -o main-two-cvs-while-extra-unlock.o main-two-cvs-while-extra-unlock.c -Wall -pthread


