CC=gcc
CFLAGS=-Wall 

all: barber clients

barber: barber.c
	$(CC) $(CFLAGS) -o barber barber.c -lrt

client: clients.c
	$(CC) $(CFLAGS) -o clients clients.c -lrt



clean:
	rm barber  client

