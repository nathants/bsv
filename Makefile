.PHONY: all clean test
CFLAGS=-Iutil -Wall -O3 -march=native -mtune=native
ALL=bsv csv gen_csv rcut _csv _read _write

all: $(ALL)

setup:
	mkdir -p bin

clean: setup
	cd bin && rm -f -- $(ALL)

test: setup
	tox

bsv: setup
	gcc $(CFLAGS) src/bsv.c -o bin/bsv

csv: setup
	gcc $(CFLAGS) src/csv.c -o bin/csv

gen_csv: setup
	gcc $(CFLAGS) src/gen_csv.c -o bin/gen_csv

rcut: setup
	gcc $(CFLAGS) src/rcut.c -o bin/rcut

_csv: setup
	gcc $(CFLAGS) util/_csv.c -o bin/_csv

_read: setup
	gcc $(CFLAGS) util/_read.c -o bin/_read

_write: setup
	gcc $(CFLAGS) util/_write.c -o bin/_write

