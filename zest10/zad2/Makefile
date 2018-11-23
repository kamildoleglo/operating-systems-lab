CC=gcc
server=server
client=client
ARGS=-std=gnu99 -Wall

all:
	$(CC) -c $(server).c -o $(server).o $(ARGS)
	$(CC) $(server).o -o $(server) -lpthread -lm $(ARGS)
	$(CC) -c $(client).c -o $(client).o $(ARGS)
	$(CC) $(client).o -o $(client) $(ARGS)

clean:
	rm -f *.o
	rm -f $(server)
	rm -f $(client)

server:
	./server 8083 ala

client:
	./client ala 1 ala

client2:
	./client ala 0 127.0.0.1 8083
