all: tlb

clean:
	rm -f null.o array-of-ints.o vector.o

tlb: tlb.c
	# compiling this program with symbol information included (with the -g flag)
	gcc -o tlb.o tlb.c -Wall -g -O0

