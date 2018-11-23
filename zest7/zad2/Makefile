CC=gcc
CFLAGS=-Wall 

all: barber clients

barber: barber.c
	$(CC) $(CFLAGS) -o barber barber.c -std=gnu99 -lpthread -lrt

clients: clients.c
	$(CC) $(CFLAGS) -o clients clients.c -std=gnu99 -lpthread -lrt



clean:
	rm barber clients

