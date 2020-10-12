all: null array vector

clean:
	rm -f null.o array-of-ints.o vector.o

null: null.c
	# compiling this program with symbol information included (with the -g flag)
	gcc -o null.o null.c -Wall -g

array: array-of-ints.c
	gcc -o array-of-ints.o array-of-ints.c -Wall -g

vector: vector.c
	gcc -o vector.o vector.c -Wall -g
