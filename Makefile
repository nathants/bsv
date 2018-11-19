.PHONY: all clean test
CFLAGS=-Iutil -Wall -O3 -march=native -mtune=native
ALL=bsv csv gen_csv rcut _csv _read _write

all: $(ALL)

clean:
	cd bin && rm -f -- $(ALL)

test:
	py.test -vx --tb native test/*.py

bsv:
	gcc $(CFLAGS) src/bsv.c -o bin/bsv

csv:
	gcc $(CFLAGS) src/csv.c -o bin/csv

gen_csv:
	gcc $(CFLAGS) src/gen_csv.c -o bin/gen_csv

rcut:
	gcc $(CFLAGS) src/rcut.c -o bin/rcut

_csv:
	gcc $(CFLAGS) util/_csv.c -o bin/_csv

_read:
	gcc $(CFLAGS) util/_read.c -o bin/_read

_write:
	gcc $(CFLAGS) util/_write.c -o bin/_write

