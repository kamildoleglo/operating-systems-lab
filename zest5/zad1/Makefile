CC=gcc
CFLAGS=-Wall

all: main run

main: main.c
	$(CC) $(CFLAGS) -o main main.c

run:
	./main ./commands.txt

clean:
	rm main

