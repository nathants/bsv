.PHONY: all clean

CFLAGS=-Wall -O3 -march=native -mtune=native

all: rcut bucket partition gen-csv dedupe

partition:
	gcc $(CFLAGS) partition.c -o partition

dedupe:
	gcc $(CFLAGS) dedupe.c -o dedupe

rcut:
	gcc $(CFLAGS) rcut.c -o rcut

bucket:
	gcc $(CFLAGS) bucket.c -o bucket

gen-csv:
	gcc $(CFLAGS) gen_csv.c -o gen-csv

clean:
	rm -f rcut bucket partition gen-csv dedupe
