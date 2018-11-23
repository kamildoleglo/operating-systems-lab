CC=gcc
CFLAGS=-Wall -std=gnu11

all: main client 

main: main.c
	$(CC) $(CFLAGS) -o main main.c 

client: client.c
	$(CC) $(CFLAGS) -o client client.c 


clean:
	rm main client

