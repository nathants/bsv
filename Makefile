.PHONY: all clean test
CFLAGS=${CC_EXTRA} -Wno-int-conversion -Wno-incompatible-pointer-types -Wno-discarded-qualifiers -Iutil -Ivendor -flto -O3 -march=native -mtune=native -lm
ALL=clean docs _bcopy _bcopyraw _copy _csv _gen_bsv _gen_csv _queue bcat bcombine bcounteach bcounteach-hash bcountrows bcut bdedupe bdedupe-hash bdropuntil bhead blz4 blz4d bmerge bpartition bquantile-merge bquantile-sketch bschema bsort bsplit bsum bsumeach bsumeach-hash bsv btake btakeuntil btopn bunzip bzip csv xxh3

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
	gcc vendor/lz4.c src/_bcopy.c -o bin/_bcopy $(CFLAGS)

_bcopyraw: setup
	gcc vendor/lz4.c src/_bcopyraw.c -o bin/_bcopyraw $(CFLAGS)

_copy: setup
	gcc vendor/lz4.c src/_copy.c -o bin/_copy $(CFLAGS)

_csv: setup
	gcc vendor/lz4.c src/_csv.c -o bin/_csv $(CFLAGS)

_gen_bsv: setup
	gcc vendor/lz4.c src/_gen_bsv.c -o bin/_gen_bsv $(CFLAGS)

_gen_csv: setup
	gcc vendor/lz4.c src/_gen_csv.c -o bin/_gen_csv $(CFLAGS)

_queue: setup
	gcc vendor/lz4.c src/_queue.c -o bin/_queue $(CFLAGS)

bcat: setup
	gcc vendor/lz4.c src/bcat.c -o bin/bcat $(CFLAGS)

bcombine: setup
	gcc vendor/lz4.c src/bcombine.c -o bin/bcombine $(CFLAGS)

bcounteach: setup
	gcc vendor/lz4.c src/bcounteach.c -o bin/bcounteach $(CFLAGS)

bcounteach-hash: setup
	gcc vendor/lz4.c src/bcounteach_hash.c -o bin/bcounteach-hash $(CFLAGS)

bcountrows: setup
	gcc vendor/lz4.c src/bcountrows.c -o bin/bcountrows $(CFLAGS)

bcut: setup
	gcc vendor/lz4.c src/bcut.c -o bin/bcut $(CFLAGS)

bdedupe: setup
	gcc vendor/lz4.c src/bdedupe.c -o bin/bdedupe $(CFLAGS)

bdedupe-hash: setup
	gcc vendor/lz4.c src/bdedupe_hash.c -o bin/bdedupe-hash $(CFLAGS)

bdropuntil: setup
	gcc vendor/lz4.c src/bdropuntil.c -o bin/bdropuntil $(CFLAGS)

bhead: setup
	gcc vendor/lz4.c src/bhead.c -o bin/bhead $(CFLAGS)

blz4: setup
	gcc vendor/lz4.c src/blz4.c -o bin/blz4 $(CFLAGS)

blz4d: setup
	gcc vendor/lz4.c src/blz4d.c -o bin/blz4d $(CFLAGS)

bmerge: setup
	gcc vendor/lz4.c src/bmerge.c -o bin/bmerge $(CFLAGS)

bpartition: setup
	gcc vendor/lz4.c src/bpartition.c -o bin/bpartition $(CFLAGS)

bquantile-merge: setup
	gcc vendor/lz4.c src/bquantile_merge.c -o bin/bquantile-merge $(CFLAGS)

bquantile-sketch: setup
	gcc vendor/lz4.c src/bquantile_sketch.c -o bin/bquantile-sketch $(CFLAGS)

bschema: setup
	gcc vendor/lz4.c src/bschema.c -o bin/bschema $(CFLAGS)

bsort: setup
	gcc vendor/lz4.c src/bsort.c -o bin/bsort $(CFLAGS)

bsplit: setup
	gcc vendor/lz4.c src/bsplit.c -o bin/bsplit $(CFLAGS)

bsum: setup
	gcc vendor/lz4.c src/bsum.c -o bin/bsum $(CFLAGS)

bsumeach: setup
	gcc vendor/lz4.c src/bsumeach.c -o bin/bsumeach $(CFLAGS)

bsumeach-hash: setup
	gcc vendor/lz4.c src/bsumeach_hash.c -o bin/bsumeach-hash $(CFLAGS)

bsv: setup
	gcc vendor/lz4.c src/bsv.c -o bin/bsv $(CFLAGS)

btake: setup
	gcc vendor/lz4.c src/btake.c -o bin/btake $(CFLAGS)

btakeuntil: setup
	gcc vendor/lz4.c src/btakeuntil.c -o bin/btakeuntil $(CFLAGS)

btopn: setup
	gcc vendor/lz4.c src/btopn.c -o bin/btopn $(CFLAGS)

bunzip: setup
	gcc vendor/lz4.c src/bunzip.c -o bin/bunzip $(CFLAGS)

bzip: setup
	gcc vendor/lz4.c src/bzip.c -o bin/bzip $(CFLAGS)

csv: setup
	gcc vendor/lz4.c src/csv.c -o bin/csv $(CFLAGS)

xxh3: setup
	gcc vendor/lz4.c src/xxh3.c -o bin/xxh3 $(CFLAGS)

