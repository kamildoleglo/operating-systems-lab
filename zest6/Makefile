all: server client

server: server.c
	gcc -pthread -Wall -o server server.c -lrt

client: client.c
	gcc -pthread -Wall -o client client.c -lrt


clean:
	rm server client
