CC=gcc
CFLAGS=-Wall

all: main cpu mem run

main: main.c
	$(CC) $(CFLAGS) -o main main.c

cpu:
	$(CC) $(CFLAGS) -o cpu_limit_violation cpu_limit_violation.c

mem:
	$(CC) $(CFLAGS) -o mem_limit_violation mem_limit_violation.c

run:
	./main ./list 1 1

clean:
	rm main cpu_limit_violation mem_limit_violation

