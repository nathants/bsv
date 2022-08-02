.PHONY: all clean test
CFLAGS=${CC_EXTRA} -Wno-int-conversion -Wno-incompatible-pointer-types -Wno-discarded-qualifiers -Iutil -Ivendor -flto -O3 -march=native -mtune=native -lm
ALL=clean docs _bcopy _bcopyraw _copy _csv _gen_bsv _gen_csv bcat bcombine bcounteach bcounteach-hash bcountrows bcut bdedupe bdedupe-hash bdropuntil bhead blz4 blz4d bmerge bpartition bquantile-merge bquantile-sketch bschema bsort bsplit bsum bsumeach bsumeach-hash bsv btake btakeuntil btopn bunzip bzip csv xxh3

all: $(ALL)

setup:
	mkdir -p bin
	./scripts/version.sh &>/dev/null

clean: setup
	cd bin && rm -f -- * *.*

docs:
	./scripts/update_readme.py

test: setup
	tox

_bcopy: setup
	gcc $(CFLAGS) vendor/lz4.c src/_bcopy.c -o bin/_bcopy

_bcopyraw: setup
	gcc $(CFLAGS) vendor/lz4.c src/_bcopyraw.c -o bin/_bcopyraw

_copy: setup
	gcc $(CFLAGS) vendor/lz4.c src/_copy.c -o bin/_copy

_csv: setup
	gcc $(CFLAGS) vendor/lz4.c src/_csv.c -o bin/_csv

_gen_bsv: setup
	gcc $(CFLAGS) vendor/lz4.c src/_gen_bsv.c -o bin/_gen_bsv

_gen_csv: setup
	gcc $(CFLAGS) vendor/lz4.c src/_gen_csv.c -o bin/_gen_csv

bcat: setup
	gcc $(CFLAGS) vendor/lz4.c src/bcat.c -o bin/bcat

bcombine: setup
	gcc $(CFLAGS) vendor/lz4.c src/bcombine.c -o bin/bcombine

bcounteach: setup
	gcc $(CFLAGS) vendor/lz4.c src/bcounteach.c -o bin/bcounteach

bcounteach-hash: setup
	gcc $(CFLAGS) vendor/lz4.c src/bcounteach_hash.c -o bin/bcounteach-hash

bcountrows: setup
	gcc $(CFLAGS) vendor/lz4.c src/bcountrows.c -o bin/bcountrows

bcut: setup
	gcc $(CFLAGS) vendor/lz4.c src/bcut.c -o bin/bcut

bdedupe: setup
	gcc $(CFLAGS) vendor/lz4.c src/bdedupe.c -o bin/bdedupe

bdedupe-hash: setup
	gcc $(CFLAGS) vendor/lz4.c src/bdedupe_hash.c -o bin/bdedupe-hash

bdropuntil: setup
	gcc $(CFLAGS) vendor/lz4.c src/bdropuntil.c -o bin/bdropuntil

bhead: setup
	gcc $(CFLAGS) vendor/lz4.c src/bhead.c -o bin/bhead

blz4: setup
	gcc $(CFLAGS) vendor/lz4.c src/blz4.c -o bin/blz4

blz4d: setup
	gcc $(CFLAGS) vendor/lz4.c src/blz4d.c -o bin/blz4d

bmerge: setup
	gcc $(CFLAGS) vendor/lz4.c src/bmerge.c -o bin/bmerge

bpartition: setup
	gcc $(CFLAGS) vendor/lz4.c src/bpartition.c -o bin/bpartition

bquantile-merge: setup
	gcc $(CFLAGS) vendor/lz4.c src/bquantile_merge.c -o bin/bquantile-merge

bquantile-sketch: setup
	gcc $(CFLAGS) vendor/lz4.c src/bquantile_sketch.c -o bin/bquantile-sketch

bschema: setup
	gcc $(CFLAGS) vendor/lz4.c src/bschema.c -o bin/bschema

bsort: setup
	gcc $(CFLAGS) vendor/lz4.c src/bsort.c -o bin/bsort

bsplit: setup
	gcc $(CFLAGS) vendor/lz4.c src/bsplit.c -o bin/bsplit

bsum: setup
	gcc $(CFLAGS) vendor/lz4.c src/bsum.c -o bin/bsum

bsumeach: setup
	gcc $(CFLAGS) vendor/lz4.c src/bsumeach.c -o bin/bsumeach

bsumeach-hash: setup
	gcc $(CFLAGS) vendor/lz4.c src/bsumeach_hash.c -o bin/bsumeach-hash

bsv: setup
	gcc $(CFLAGS) vendor/lz4.c src/bsv.c -o bin/bsv

btake: setup
	gcc $(CFLAGS) vendor/lz4.c src/btake.c -o bin/btake

btakeuntil: setup
	gcc $(CFLAGS) vendor/lz4.c src/btakeuntil.c -o bin/btakeuntil

btopn: setup
	gcc $(CFLAGS) vendor/lz4.c src/btopn.c -o bin/btopn

bunzip: setup
	gcc $(CFLAGS) vendor/lz4.c src/bunzip.c -o bin/bunzip

bzip: setup
	gcc $(CFLAGS) vendor/lz4.c src/bzip.c -o bin/bzip

csv: setup
	gcc $(CFLAGS) vendor/lz4.c src/csv.c -o bin/csv

xxh3: setup
	gcc $(CFLAGS) vendor/lz4.c src/xxh3.c -o bin/xxh3

