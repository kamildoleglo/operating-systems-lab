CC=gcc
CFLAGS=-Wall

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

run:
	./server &
	./client

clean:
	rm server client

