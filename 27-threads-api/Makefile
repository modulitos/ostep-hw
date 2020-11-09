FLAGS = -Wall -pthread -g

all: main-race main-deadlock main-deadlock-global main-signal main-signal-cv

clean:
	rm -f main-race.o main-deadlock.o main-deadlock-global.o main-signal.o main-signal-cv.o

main-race: main-race.c common_threads.h
	gcc -o main-race.o main-race.c $(FLAGS)

main-deadlock: main-deadlock.c common_threads.h
	gcc -o main-deadlock.o main-deadlock.c $(FLAGS)

main-deadlock-global: main-deadlock-global.c common_threads.h
	gcc -o main-deadlock-global.o main-deadlock-global.c $(FLAGS)

main-signal: main-signal.c common_threads.h
	gcc -o main-signal.o main-signal.c $(FLAGS)

main-signal-cv: main-signal-cv.c common_threads.h
	gcc -o main-signal-cv.o main-signal-cv.c $(FLAGS)

