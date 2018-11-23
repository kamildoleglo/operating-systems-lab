CC=gcc
CFLAGS=-Wall

all: master slave run

master: master.c
	$(CC) $(CFLAGS) -o master master.c
slave: slave.c
	$(CC) $(CFLAGS) -o slave slave.c

run:
	./master ./temp &
	./slave ./temp 5 &

clean:
	rm master slave

