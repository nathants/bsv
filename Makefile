.PHONY: all clean test
CFLAGS=-Iutil -Wall -O3 -march=native -mtune=native
ALL=bcut bsv bucket count_each _csv csv gen_csv _read _write

all: $(ALL)

setup:
	mkdir -p bin

clean: setup
	cd bin && rm -f -- $(ALL)

test: setup
	tox

bcut: setup
	gcc $(CFLAGS) src/bcut.c -o bin/bcut

bsv: setup
	gcc $(CFLAGS) src/bsv.c -o bin/bsv

bucket: setup
	gcc $(CFLAGS) src/bucket.c -o bin/bucket

count_each: setup
	gcc $(CFLAGS) src/count_each.c -o bin/count_each

_csv: setup
	gcc $(CFLAGS) src/_csv.c -o bin/_csv

csv: setup
	gcc $(CFLAGS) src/csv.c -o bin/csv

gen_csv: setup
	gcc $(CFLAGS) src/gen_csv.c -o bin/gen_csv

_read: setup
	gcc $(CFLAGS) src/_read.c -o bin/_read

_write: setup
	gcc $(CFLAGS) src/_write.c -o bin/_write

