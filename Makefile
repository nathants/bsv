.PHONY: all clean

CFLAGS=-Wall -O3 -march=native -mtune=native

all: rcut bucket partition gen-csv dedupe csv read

partition:
	gcc $(CFLAGS) partition.c -o partition

dedupe:
	gcc $(CFLAGS) dedupe.c -o dedupe

rcut:
	gcc $(CFLAGS) rcut.c -o rcut

bucket:
	gcc $(CFLAGS) bucket.c -o bucket

csv:
	gcc $(CFLAGS) csv.c -o csv

read:
	gcc $(CFLAGS) read.c -o read

gen-csv:
	gcc $(CFLAGS) gen_csv.c -o gen-csv

clean:
	rm -f rcut bucket partition gen-csv dedupe csv *.8 *.11 *.17 *.64 *.256 *.1024
