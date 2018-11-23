CC=gcc
CFLAGS=-c -Wall


all: static


main_dyn.o: main_dyn.c
	$(CC) $(CFLAGS) main_dyn.c 

main.o: main.c
	$(CC) $(CFLAGS) main.c

library.o: library.c
	$(CC) $(CFLAGS) library.c

libraryPic:
	$(CC) -fPIC -c library.c

library.so: libraryPic
	$(CC) -shared -fPIC -Wl,-soname,liblibrary.so -o liblibrary.so library.o -lc

static: main.o library.o
	$(CC) main.o library.o -o main

shared: main.o library.so
	$(CC) main.c -L ./ -llibrary -o main

shared_dyn: main_dyn.o library.so
	$(CC) main_dyn.o -o main_dyn -ldl

dynamic_run: shared_dyn 
	./main_dyn -n 1000 -b 30 -m 0 -f 2 -a 600

static_run: static
	./main -n 1000 -b 30 -m 0 -f 2 -a 600

shared_run: shared
	./main -n 1000 -b 30 -m 0 -f 2 -a 600

clean:
	rm *o main main_dyn

