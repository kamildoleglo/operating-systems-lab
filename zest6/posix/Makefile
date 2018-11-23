CC=gcc
CFLAGS=-Wall 

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c -lrt

client: client.c
	$(CC) $(CFLAGS) -o client client.c -lrt

run:
	./server &
	./client

clean:
	rm server client

