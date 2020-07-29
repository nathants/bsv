## why

it should be simple and easy to process data at the speed of sequential io.

## what

a simple and efficient [data](https://github.com/nathants/bsv/blob/master/util/load.h) [format](https://github.com/nathants/bsv/blob/master/util/dump.h) for easily manipulating chunks of rows of columns while minimizing allocations and copies.

minimal cli [tools](#tools) for rapidly composing performant data flow pipelines.

## how

column: 0-65536 bytes.

row: 0-65536 columns.

chunk: up to 5MB containing 1 or more complete rows.

note: row data cannot exceed chunk size.

## layout

[chunk](https://github.com/nathants/bsv/blob/master/util/read.h):

```bash
| i32:size | row-1 | ... | row-n |
```

[row](https://github.com/nathants/bsv/blob/master/util/load.h):

```bash
| u16:max | u16:size-1 | ... | u16:size-n | u8:column-1 | ... | u8:column-n |
```

note: column bytes are always followed by a single nullbyte.

note: max is the maximum zero based index into the row.

## example

add `bsumall.c` to `bsv/src/`:

```c
#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "sum columns of u16 as i64\n\n"
#define USAGE "... | bsumall \n\n"
#define EXAMPLE ">> echo '\n1,2\n3,4\n' | bsv | bschema a:u16,a:u16 | bsumall i64 | bschema i64:a,i64:a | csv\n4,6\n"

int main(int argc, char **argv) {

    // setup state
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);
    i64 sums[MAX_COLUMNS] = {0};
    row_t row;

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        for (i32 i = 0; i <= row.max; i++) {
            ASSERT(sizeof(u16) == row.sizes[i], "fatal: bad data\n");
            sums[i] += *(u16*)row.columns[i];
        }
    }

    // generate output row
    row.max = -1;
    for (i32 i = 0; i < MAX_COLUMNS; i++) {
        if (!sums[i])
            break;
        row.sizes[i] = sizeof(i64);
        row.columns[i] = &sums[i];
        row.max++;
    }

    // dump output
    if (row.max >= 0)
        dump(&wbuf, &row, 0);
    dump_flush(&wbuf, 0);
}
```

build and run:

```bash
>> ./scripts/makefile.sh

>> make bsumall

>> bsumall -h
sum columns of u16 as i64

usage: ... | bsumall

>> echo '
1,2
3,4
' | bsv | bschema a:u16,a:u16 | bsumall i64 | bschema i64:a,i64:a | csv
4,6
```

## non goals

support of hardware other than little endian.

types and schemas as a part of the data format.

## testing methodology

[quickcheck](https://hypothesis.readthedocs.io/en/latest/) style [testing](https://github.com/nathants/bsv/blob/master/test) with python implementations to verify correct behavior for arbitrary inputs and varying buffer sizes.

## experiments

[performance](https://github.com/nathants/bsv/blob/master/experiments/) experiments and alternate implementations.

## related work

[s4](https://github.com/nathants/s4) - a storage cluster that is cheap and fast, with data local compute and efficient shuffle.

## more examples

[structured analysis of nyc taxi data with bsv and hive](https://github.com/nathants/s4/blob/master/examples/nyc_taxi_bsv)

## tools

| name | description |
| -- | -- |
| [bcat](#bcat) | cat some bsv files to csv |
| [bcombine](#bcombine) | prepend a new column by combining values from existing columns |
| [bcopy](#bcopy) | pass through data, to benchmark load/dump performance |
| [bcounteach](#bcounteach) | count as i64 each contiguous identical row by the first column |
| [bcounteach-hash](#bcounteach-hash) | count as i64 by hash of the first column |
| [bcountrows](#bcountrows) | count rows as i64 |
| [bcut](#bcut) | select some columns |
| [bdedupe](#bdedupe) | dedupe identical contiguous rows by the first column, keeping the first |
| [bdedupe-hash](#bdedupe-hash) | dedupe rows by hash of the first column, keeping the first |
| [bdropuntil](#bdropuntil) | for sorted input, drop until the first column is gte to VALUE |
| [bhead](#bhead) | keep the first n rows |
| [blz4](#blz4) | compress bsv data |
| [blz4d](#blz4d) | decompress bsv data |
| [bmerge](#bmerge) | merge sorted files from stdin |
| [bpartition](#bpartition) | split into multiple files by consistent hash of the first column value |
| [bquantile-merge](#bquantile-merge) | merge ddsketches and output quantile value pairs as f64 |
| [bquantile-sketch](#bquantile-sketch) | collapse the first column into a single row ddsketch |
| [bschema](#bschema) | validate and converts row data with a schema of columns |
| [bsort](#bsort) | timsort rows by the first column |
| [bsplit](#bsplit) | split a stream into multiple files |
| [bsum](#bsum) | sum the first column |
| [bsumeach](#bsumeach) | sum the second colum of each contiguous identical row by the first column |
| [bsumeach-hash](#bsumeach-hash) | sum as i64 the second colum by hash of the first column |
| [bsv](#bsv) | convert csv to bsv |
| [btake](#btake) | take while the first column is VALUE |
| [btakeuntil](#btakeuntil) | for sorted input, take until the first column is gte to VALUE |
| [btopn](#btopn) | accumulate the top n rows in a heap by first column value |
| [bunzip](#bunzip) | split a multi column input into single column outputs |
| [bzip](#bzip) | combine single column inputs into a multi column output |
| [csv](#csv) | convert bsv to csv |
| [xxh3](#xxh3) | xxh3_64 hash stdin |

### [bcat](https://github.com/nathants/bsv/blob/master/src/bcat.c)

cat some bsv files to csv

```bash
usage: bcat [-l|--lz4] [-p|--prefix] [-h N|--head N] FILE1 ... FILEN
```

```bash
>> for char in a a b b c c; do
     echo $char | bsv >> /tmp/$char
   done

>> bcat --head 1 --prefix /tmp/{a,b,c}
/tmp/a:a
/tmp/b:b
/tmp/c:c
```

### [bcombine](https://github.com/nathants/bsv/blob/master/src/bcombine.c)

prepend a new column by combining values from existing columns

```bash
usage: ... | bcombine COL1,...,COLN
```

```bash
>> echo a,b,c | bsv | bcombine 3,2 | csv
b:a,a,b,c
```

### [bcopy](https://github.com/nathants/bsv/blob/master/src/bcopy.c)

pass through data, to benchmark load/dump performance

```bash
usage: ... | bcopy
```

```bash
>> echo a,b,c | bsv | bcopy | csv
a,b,c
```

### [bcounteach](https://github.com/nathants/bsv/blob/master/src/bcounteach.c)

count as i64 each contiguous identical row by the first column

```bash
usage: ... | bcounteach
```

```bash
echo '
a
a
b
b
b
a
' | bsv | bcounteach | bschema *,i64:a | csv
a,2
b,3
a,1
```

### [bcounteach-hash](https://github.com/nathants/bsv/blob/master/src/bcounteach_hash.c)

count as i64 by hash of the first column

```bash
usage: ... | bcounteach-hash
```

```bash
echo '
a
a
b
b
b
a
' | bsv | bcounteach-hash | bschema *,i64:a | bsort | csv
a,3
b,3
```

### [bcountrows](https://github.com/nathants/bsv/blob/master/src/bcountrows.c)

count rows as i64

```bash
usage: ... | bcountrows
```

```bash
>> echo '
1
2
3
4
' | bsv | bcountrows | csv
4
```

### [bcut](https://github.com/nathants/bsv/blob/master/src/bcut.c)

select some columns

```bash
usage: ... | bcut COL1,...,COLN
```

```bash
>> echo a,b,c | bsv | bcut 3,3,3,2,2,1 | csv
c,c,c,b,b,a
```

### [bdedupe](https://github.com/nathants/bsv/blob/master/src/bdedupe.c)

dedupe identical contiguous rows by the first column, keeping the first

```bash
usage: ... | bdedupe
```

```bash
>> echo '
a
a
b
b
a
a
' | bsv | bdedupe | csv
a
b
a
```

### [bdedupe-hash](https://github.com/nathants/bsv/blob/master/src/bdedupe_hash.c)

dedupe rows by hash of the first column, keeping the first

```bash
usage: ... | bdedupe-hash
```

```bash
>> echo '
a
a
b
b
a
a
' | bsv | bdedupe-hash | csv
a
b
```

### [bdropuntil](https://github.com/nathants/bsv/blob/master/src/bdropuntil.c)

for sorted input, drop until the first column is gte to VALUE

```bash
usage: ... | bdropuntil VALUE [TYPE]
```

```bash
>> echo '
a
b
c
d
' | bsv | bdropuntil c | csv
c
d
```

### [bhead](https://github.com/nathants/bsv/blob/master/src/bhead.c)

keep the first n rows

```bash
usage: ... | bhead N
```

```bash
>> echo '
a
b
c
' | bsv | btail 2 | csv
a
b
```

### [blz4](https://github.com/nathants/bsv/blob/master/src/blz4.c)

compress bsv data

```bash
usage: ... | blz4
```

```bash
>> echo a,b,c | bsv | blz4 | blz4d | csv
a,b,c
```

### [blz4d](https://github.com/nathants/bsv/blob/master/src/blz4d.c)

decompress bsv data

```bash
usage: ... | blz4d
```

```bash
>> echo a,b,c | bsv | blz4 | blz4d | csv
a,b,c
```

### [bmerge](https://github.com/nathants/bsv/blob/master/src/bmerge.c)

merge sorted files from stdin

```bash
usage: echo FILE1 ... FILEN | bmerge [TYPE] [-r|--reversed] [-l|--lz4]
```

```bash
>> echo -e 'a
c
e
' | bsv > a.bsv
>> echo -e 'b
d
f
' | bsv > b.bsv
>> echo a.bsv b.bsv | bmerge
a
b
c
d
e
f
```

### [bpartition](https://github.com/nathants/bsv/blob/master/src/bpartition.c)

split into multiple files by consistent hash of the first column value

```bash
usage: ... | bpartition NUM_BUCKETS [PREFIX] [-l|--lz4]
```

```bash
>> echo '
a
b
c
' | bsv | bpartition 10 prefix
prefix03
prefix06
```

### [bquantile-merge](https://github.com/nathants/bsv/blob/master/src/bquantile_merge.c)

merge ddsketches and output quantile value pairs as f64

```bash
usage: ... | bquantile-merge QUANTILES
```

```bash
>> seq 1 100 | bsv | bschema a:i64 | bquantile-sketch i64 | bquantile-merge .2,.5,.7 | bschema f64:a,f64:a | csv
0.2,19.88667024086646
0.5,49.90296094906742
0.7,70.11183939140405
```

### [bquantile-sketch](https://github.com/nathants/bsv/blob/master/src/bquantile_sketch.c)

collapse the first column into a single row ddsketch

```bash
usage: ... | bquantile-sketch TYPE [-a|--alpha] [-b|--max-bins] [-m|--min-value]
```

```bash
>> seq 1 100 | bsv | bschema a:i64 | bquantile-sketch i64 | bquantile-merge .2,.5,.7 | bschema f64:a,f64:a | csv
0.2,19.88667024086646
0.5,49.90296094906742
0.7,70.11183939140405
```

### [bschema](https://github.com/nathants/bsv/blob/master/src/bschema.c)

validate and converts row data with a schema of columns

```bash
usage: ... | bschema SCHEMA [--filter]
```

```bash
  --filter remove bad rows instead of erroring

  example schemas:
    *,*,*             = 3 columns of any size
    8,*               = a column with 8 bytes followed by a column of any size
    8,*,...           = same as above, but ignore any trailing columns
    a:u16,a:i32,a:f64 = convert ascii to numerics
    u16:a,i32:a,f64:a = convert numerics to ascii
    4*,*4             = keep the first 4 bytes of column 1 and the last 4 of column 2

>> echo aa,bbb,cccc | bsv | bschema 2,3,4 | csv
aa,bbb,cccc
```

### [bsort](https://github.com/nathants/bsv/blob/master/src/bsort.c)

timsort rows by the first column

```bash
usage: ... | bsort [-r|--reversed] [TYPE]
```

```bash
>> echo '
3
2
1
' | bsv | bschema a:i64 | bsort i64 | bschema i64:a | csv
1
2
3
```

### [bsplit](https://github.com/nathants/bsv/blob/master/src/bsplit.c)

split a stream into multiple files

```bash
usage: ... | bsplit PREFIX [chunks_per_file=1]
```

```bash
>> echo -n a,b,c | bsv | bsplit prefix
prefix_0000000000
```

### [bsum](https://github.com/nathants/bsv/blob/master/src/bsum.c)

sum the first column

```bash
usage: ... | bsum TYPE
```

```bash
>> echo '
1
2
3
4
' | bsv | bschema a:i64 | bsum i64 | bschema i64:a | csv
10
```

### [bsumeach](https://github.com/nathants/bsv/blob/master/src/bsumeach.c)

sum the second colum of each contiguous identical row by the first column

```bash
usage: ... | bsumeach TYPE
```

```bash
echo '
a,1
a,2
b,3
b,4
b,5
a,6
' | bsv | bschema *,a:i64 | bsumeach i64 | bschema *,i64:a | csv
a,3
b,12
a,6
```

### [bsumeach-hash](https://github.com/nathants/bsv/blob/master/src/bsumeach_hash.c)

sum as i64 the second colum by hash of the first column

```bash
usage: ... | bsumeach-hash i64
```

```bash
echo '
a,1
a,2
b,3
b,4
b,5
a,6
' | bsv | bschema *,a:i64 | bsumeach-hash i64 | bschema *,i64:a | csv
a,3
b,12
a,6
```

### [bsv](https://github.com/nathants/bsv/blob/master/src/bsv.c)

convert csv to bsv

```bash
usage: ... | bsv
```

```bash
>> echo a,b,c | bsv | bcut 3,2,1 | csv
c,b,a
```

### [btake](https://github.com/nathants/bsv/blob/master/src/btake.c)

take while the first column is VALUE

```bash
usage: ... | btake VALUE
```

```bash
>> echo '
a
b
c
d
' | bsv | bdropntil c | btake c | csv
c
```

### [btakeuntil](https://github.com/nathants/bsv/blob/master/src/btakeuntil.c)

for sorted input, take until the first column is gte to VALUE

```bash
usage: ... | btakeuntil VALUE [TYPE]
```

```bash
>> echo '
a
b
c
d
' | bsv | btakeuntil c | csv
a
b
```

### [btopn](https://github.com/nathants/bsv/blob/master/src/btopn.c)

accumulate the top n rows in a heap by first column value

```bash
usage: ... | btopn N [TYPE] [-r|--reversed]
```

```bash
>> echo '
1
3
2
' | bsv | bschema a:i64 | btopn 2 i64 | bschema i64:a | csv
3
2
```

### [bunzip](https://github.com/nathants/bsv/blob/master/src/bunzip.c)

split a multi column input into single column outputs

```bash
usage: ... | bunzip PREFIX [-l|--lz4]
```

```bash
>> echo '
a,b,c
1,2,3
' | bsv | bunzip col && echo col_1 col_3 | bzip | csv
a,c
1,3
```

### [bzip](https://github.com/nathants/bsv/blob/master/src/bzip.c)

combine single column inputs into a multi column output

```bash
usage: ls column_* | bzip [COL1,...COLN] [-l|--lz4]
```

```bash
>> echo '
a,b,c
1,2,3
' | bsv | bunzip column && ls column_* | bzip 1,3 | csv
a,c
1,3
```

### [csv](https://github.com/nathants/bsv/blob/master/src/csv.c)

convert bsv to csv

```bash
usage: ... | csv
```

```bash
>> echo a,b,c | bsv | csv
a,b,c
```

### [xxh3](https://github.com/nathants/bsv/blob/master/src/xxh3.c)

xxh3_64 hash stdin

```bash
usage: ... | xxh3 [--stream|--int]
```

```bash
  --stream pass stdin through to stdout with hash on stderr

  --int output hash as int not hash

>> echo abc | xxh3
B5CA312E51D77D64
```
