.PHONY: all clean test
CFLAGS=-Iutil -O3 -march=native -mtune=native
ALL=docs bbucket bcat bcounteach bcut bdedupe bdisjoint bdropuntil bpartition bsort bsplit bsv _copy _csv csv _gen_csv_c xxh3

all: $(ALL)

setup:
	mkdir -p bin

clean: setup
	cd bin && rm -f -- $(ALL) *.*

docs:
	./readme.py

test: setup
	tox

bbucket: setup
	gcc $(CFLAGS) src/bbucket.c -o bin/bbucket

bcat: setup
	gcc $(CFLAGS) src/bcat.c -o bin/bcat

bcounteach: setup
	gcc $(CFLAGS) src/bcounteach.c -o bin/bcounteach

bcut: setup
	gcc $(CFLAGS) src/bcut.c -o bin/bcut

bdedupe: setup
	gcc $(CFLAGS) src/bdedupe.c -o bin/bdedupe

bdisjoint: setup
	gcc $(CFLAGS) src/bdisjoint.c -o bin/bdisjoint

bdropuntil: setup
	gcc $(CFLAGS) src/bdropuntil.c -o bin/bdropuntil

bpartition: setup
	gcc $(CFLAGS) src/bpartition.c -o bin/bpartition

bsort: setup
	gcc $(CFLAGS) src/bsort.c -o bin/bsort

bsplit: setup
	gcc $(CFLAGS) src/bsplit.c -o bin/bsplit

bsv: setup
	gcc $(CFLAGS) src/bsv.c -o bin/bsv

_copy: setup
	gcc $(CFLAGS) src/_copy.c -o bin/_copy

_csv: setup
	gcc $(CFLAGS) src/_csv.c -o bin/_csv

csv: setup
	gcc $(CFLAGS) src/csv.c -o bin/csv

_gen_csv_c: setup
	gcc $(CFLAGS) src/_gen_csv_c.c -o bin/_gen_csv_c

xxh3: setup
	gcc $(CFLAGS) src/xxh3.c -o bin/xxh3

