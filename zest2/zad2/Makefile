CC=gcc
CFLAGS=-Wall

all: main links run

main: main.c
	$(CC) $(CFLAGS) -o main main.c

links:
	ln -s /usr ./usr
	ln -s ~ ./home
run:
	./main ./ \< 30-03-2018 00:00:00

clean:
	rm main usr home

