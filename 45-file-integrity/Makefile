
ALL = check-xor test-xor-fletcher check-fletcher
COMMON = common.h checksums.h

all: $(ALL)

clean:
	rm -f $(ALL) *~

#check-xor: check-xor.c $(COMMON)
check-xor: check-xor.c $(COMMON)
	gcc -o check-xor.o check-xor.c -Wall -pthread -O

check-fletcher: check-fletcher.c $(COMMON)
	gcc -o check-fletcher.o check-fletcher.c -Wall -pthread -O

test-xor-fletcher: test-xor-fletcher.c $(COMMON)
	gcc -o test-xor-fletcher.o test-xor-fletcher.c -Wall -pthread -O
#vector-global-order: vector-global-order.c $(COMMON)
#	gcc -o vector-global-order vector-global-order.c -Wall -pthread -O
#
#vector-try-wait: vector-try-wait.c $(COMMON)
#	gcc -o vector-try-wait vector-try-wait.c -Wall -pthread -O
#
#vector-avoid-hold-and-wait: vector-avoid-hold-and-wait.c $(COMMON)
#	gcc -o vector-avoid-hold-and-wait vector-avoid-hold-and-wait.c -Wall -pthread -O
#
#vector-nolock: vector-nolock.c $(COMMON)
#	gcc -o vector-nolock vector-nolock.c -Wall -pthread -O
#
