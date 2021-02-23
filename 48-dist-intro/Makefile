
ALL = UDP-client UDP-server
COMMON = UDP-lib.h

all: $(ALL)

clean:
	rm -f $(ALL) *~

UDP-client: UDP-client.c $(COMMON)
	gcc -o UDP-client.o UDP-client.c -Wall -pthread -O

UDP-server: UDP-server.c $(COMMON)
	gcc -o UDP-server.o UDP-server.c -Wall -pthread -O

