.PHONY: all clean test
CFLAGS=-Iutil -Wall -O3 -march=native -mtune=native
ALL=bbucket bcounteach bcut bsv _csv csv _gen_csv _read _write

all: $(ALL)

setup:
	mkdir -p bin

clean: setup
	cd bin && rm -f -- $(ALL)

test: setup
	tox

bbucket: setup
	gcc $(CFLAGS) src/bbucket.c -o bin/bbucket

bcounteach: setup
	gcc $(CFLAGS) src/bcounteach.c -o bin/bcounteach

bcut: setup
	gcc $(CFLAGS) src/bcut.c -o bin/bcut

bsv: setup
	gcc $(CFLAGS) src/bsv.c -o bin/bsv

_csv: setup
	gcc $(CFLAGS) src/_csv.c -o bin/_csv

csv: setup
	gcc $(CFLAGS) src/csv.c -o bin/csv

_gen_csv: setup
	gcc $(CFLAGS) src/_gen_csv.c -o bin/_gen_csv

_read: setup
	gcc $(CFLAGS) src/_read.c -o bin/_read

_write: setup
	gcc $(CFLAGS) src/_write.c -o bin/_write

