.PHONY: all clean test
CFLAGS=-Wno-int-conversion -Wno-incompatible-pointer-types -Wno-discarded-qualifiers -Iutil -Ivendor -flto -O3 -march=native -mtune=native
ALL=docs bcat bcat-lz4 bcopy bcounteach bcounteach-hash bcountrows bcut bdedupe bdropuntil blz4 blz4d bmerge bmerge-lz4 bpartition bpartition-lz4 bschema bsort bsplit bsum bsumeach bsumeach-hash bsv btake btakeuntil bunzip bunzip-lz4 bzip bzip-lz4 _copy _csv csv _gen_bsv _gen_csv xxh3

all: $(ALL)

setup:
	mkdir -p bin

clean: setup
	cd bin && rm -f -- $(ALL) *.*

docs:
	./scripts/readme.py

test: setup
	tox

bcat: setup
	gcc $(CFLAGS) src/bcat.c -o bin/bcat

bcat-lz4: setup
	gcc $(CFLAGS) vendor/lz4.c src/bcat_lz4.c -o bin/bcat-lz4

bcopy: setup
	gcc $(CFLAGS) src/bcopy.c -o bin/bcopy

bcounteach: setup
	gcc $(CFLAGS) src/bcounteach.c -o bin/bcounteach

bcounteach-hash: setup
	gcc $(CFLAGS) src/bcounteach_hash.c -o bin/bcounteach-hash

bcountrows: setup
	gcc $(CFLAGS) src/bcountrows.c -o bin/bcountrows

bcut: setup
	gcc $(CFLAGS) src/bcut.c -o bin/bcut

bdedupe: setup
	gcc $(CFLAGS) src/bdedupe.c -o bin/bdedupe

bdropuntil: setup
	gcc $(CFLAGS) src/bdropuntil.c -o bin/bdropuntil

blz4: setup
	gcc $(CFLAGS) vendor/lz4.c src/blz4.c -o bin/blz4

blz4d: setup
	gcc $(CFLAGS) vendor/lz4.c src/blz4d.c -o bin/blz4d

bmerge: setup
	gcc $(CFLAGS) src/bmerge.c -o bin/bmerge

bmerge-lz4: setup
	gcc $(CFLAGS) vendor/lz4.c src/bmerge_lz4.c -o bin/bmerge-lz4

bpartition: setup
	gcc $(CFLAGS) src/bpartition.c -o bin/bpartition

bpartition-lz4: setup
	gcc $(CFLAGS) vendor/lz4.c src/bpartition_lz4.c -o bin/bpartition-lz4

bschema: setup
	gcc $(CFLAGS) src/bschema.c -o bin/bschema

bsort: setup
	gcc $(CFLAGS) src/bsort.c -o bin/bsort

bsplit: setup
	gcc $(CFLAGS) src/bsplit.c -o bin/bsplit

bsum: setup
	gcc $(CFLAGS) src/bsum.c -o bin/bsum

bsumeach: setup
	gcc $(CFLAGS) src/bsumeach.c -o bin/bsumeach

bsumeach-hash: setup
	gcc $(CFLAGS) src/bsumeach_hash.c -o bin/bsumeach-hash

bsv: setup
	gcc $(CFLAGS) src/bsv.c -o bin/bsv

btake: setup
	gcc $(CFLAGS) src/btake.c -o bin/btake

btakeuntil: setup
	gcc $(CFLAGS) src/btakeuntil.c -o bin/btakeuntil

bunzip: setup
	gcc $(CFLAGS) src/bunzip.c -o bin/bunzip

bunzip-lz4: setup
	gcc $(CFLAGS) vendor/lz4.c src/bunzip_lz4.c -o bin/bunzip-lz4

bzip: setup
	gcc $(CFLAGS) src/bzip.c -o bin/bzip

bzip-lz4: setup
	gcc $(CFLAGS) vendor/lz4.c src/bzip_lz4.c -o bin/bzip-lz4

_copy: setup
	gcc $(CFLAGS) src/_copy.c -o bin/_copy

_csv: setup
	gcc $(CFLAGS) src/_csv.c -o bin/_csv

csv: setup
	gcc $(CFLAGS) src/csv.c -o bin/csv

_gen_bsv: setup
	gcc $(CFLAGS) src/_gen_bsv.c -o bin/_gen_bsv

_gen_csv: setup
	gcc $(CFLAGS) src/_gen_csv.c -o bin/_gen_csv

xxh3: setup
	gcc $(CFLAGS) src/xxh3.c -o bin/xxh3

