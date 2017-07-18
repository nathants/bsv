.PHONY: all clean

CFLAGS=-Wall -O3 -march=native -mtune=native

all: rcut bucket partition gen-csv dedupe csv csvs read reads take sum sums sums-each

partition:
	gcc $(CFLAGS) partition.c -o partition

take:
	gcc $(CFLAGS) take.c -o take

takes:
	gcc $(CFLAGS) takes.c -o takes

dedupe:
	gcc $(CFLAGS) dedupe.c -o dedupe

rcut:
	gcc $(CFLAGS) rcut.c -o rcut

bucket:
	gcc $(CFLAGS) bucket.c -o bucket

csv:
	gcc $(CFLAGS) csv.c -o csv

csvs:
	gcc $(CFLAGS) csvs.c -o csvs

read:
	gcc $(CFLAGS) read.c -o read

reads:
	gcc $(CFLAGS) reads.c -o reads

gen-csv:
	gcc $(CFLAGS) gen_csv.c -o gen-csv

sum:
	gcc $(CFLAGS) sum.c -o sum

sums:
	gcc $(CFLAGS) sums.c -o sums


sums-each:
	gcc $(CFLAGS) sums_each.c -o sums-each

clean:
	rm -f rcut bucket partition gen-csv dedupe csv csvs *.8 *.11 *.17 *.64 *.256 *.1024 read reads take sum takes sums sums-each
