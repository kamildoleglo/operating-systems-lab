CC=gcc
CFLAGS=-Wall

all: main run

main:	
	$(CC) $(CFLAGS) -o main main.c

run: generate copy sort

generate: 	
	./main generate data_2000_4.t 2000 4
	./main generate data_4000_4.t 4000 4
	./main generate data_2000_512.t 2000 512
	./main generate data_4000_512.t 4000 512
	./main generate data_2000_4096.t 2000 4096
	./main generate data_4000_4096.t 4000 4096
	./main generate data_2000_8192.t 2000 8192
	./main generate data_4000_8192.t 4000 8192

copy:
	./main copy data_2000_4.t 2_data_2000_4.t 2000 4 > results.txt
	./main copy data_2000_4.t 2_data_2000_4.sys.t 2000 4 sys >> results.txt
	./main copy data_4000_4.t 2_data_4000_4.t 4000 4 >> results.txt
	./main copy data_4000_4.t 2_data_4000_4.sys.t 4000 4 sys >> results.txt
	./main copy data_2000_512.t 2_data_2000_512.t 2000 512 >> results.txt
	./main copy data_2000_512.t 2_data_2000_512.sys.t 2000 512 sys >> results.txt
	./main copy data_4000_512.t 2_data_4000_512.t 4000 512 >> results.txt
	./main copy data_4000_512.t 2_data_4000_512.sys.t 4000 512 sys >> results.txt
	./main copy data_2000_4096.t 2_data_2000_4096.t 2000 4096 >> results.txt
	./main copy data_2000_4096.t 2_data_2000_4096.sys.t 2000 4096 sys >> results.txt
	./main copy data_4000_4096.t 2_data_4000_4096.t 4000 4096 >> results.txt
	./main copy data_4000_4096.t 2_data_4000_4096.sys.t 4000 4096 sys >> results.txt
	./main copy data_2000_8192.t 2_data_2000_8192.t 2000 8192 >> results.txt
	./main copy data_2000_8192.t 2_data_2000_8192.sys.t 2000 8192 sys >> results.txt
	./main copy data_4000_8192.t 2_data_4000_8192.t 4000 8192 >> results.txt
	./main copy data_4000_8192.t 2_data_4000_8192.sys.t 4000 8192 sys >> results.txt

sort:
	./main sort data_2000_4.t 2000 4 >> results.txt
	./main sort 2_data_2000_4.sys.t 2000 4 sys >> results.txt
	./main sort data_4000_4.t 4000 4 >> results.txt
	./main sort 2_data_4000_4.sys.t 4000 4 sys >> results.txt
	./main sort data_2000_512.t 2000 512 >> results.txt
	./main sort 2_data_2000_512.sys.t 2000 512 sys >> results.txt
	./main sort data_4000_512.t 4000 512 >> results.txt
	./main sort 2_data_4000_512.sys.t 4000 512 sys >> results.txt
	./main sort data_2000_4096.t 2000 4096 >> results.txt
	./main sort 2_data_2000_4096.sys.t 2000 4096 sys >> results.txt
	./main sort data_4000_4096.t 4000 4096 >> results.txt
	./main sort 2_data_4000_4096.sys.t 4000 4096 sys >> results.txt
	./main sort data_2000_8192.t 2000 8192 >> results.txt
	./main sort 2_data_2000_8192.sys.t 2000 8192 sys >> results.txt
	./main sort data_4000_8192.t 4000 8192 >> results.txt
	./main sort 2_data_4000_8192.sys.t 4000 8192 sys >> results.txt

clean:
	rm main *.t

