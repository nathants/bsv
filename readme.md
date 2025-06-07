# BSV

## Why

It should be simple and easy to process data at the speed of sequential IO.

## What

A simple and efficient [data](https://github.com/nathants/bsv/blob/master/util/load.h) [format](https://github.com/nathants/bsv/blob/master/util/dump.h) for easily manipulating chunks of rows of columns while minimizing allocations and copies.

Minimal CLI [tools](#tools) for rapidly composing performant data flow pipelines.

## How

Column: 0-65536 bytes.

Row: 0-65536 columns.

Chunk: Up to 5MB containing 1 or more complete rows.

Note: Row data cannot exceed chunk size.

## Layout

[chunk](https://github.com/nathants/bsv/blob/master/util/read.h):

```bash
| i32:size | u8[]:row | ... |
```

[row](https://github.com/nathants/bsv/blob/master/util/load.h):

```bash
| u16:max | u16:size | ... | u8[]:column | ... |
```

Note: Column bytes are always followed by a single null byte.

Note: Max is the maximum zero based index into the row.

## Install

```bash
>> curl https://raw.githubusercontent.com/nathants/bsv/master/scripts/install_archlinux.sh | bash
```

```bash
>> git clone https://github.com/nathants/bsv
>> cd bsv
>> make -j
>> sudo mv -fv bin/* /usr/local/bin
```

Note: For best pipeline performance increase maximum pipe size

```bash
>> sudo sysctl fs.pipe-max-size=5242880
```

## Test

```bash
>> tox
```

```bash
>> docker build -t bsv:debian -f Dockerfile.debian .

>> docker run -v $(pwd):/code --rm -it bsv:debian bash -c 'cd /code && py.test -vvx --tb native -n auto test/'
```

```bash
>> docker build -t bsv:alpine -f Dockerfile.alpine .

>> docker run -v $(pwd):/code --rm -it bsv:alpine bash -c 'cd /code && py.test -vvx --tb native -n auto test/'
```

Increase the number of generated tests cases with environment variable: `TEST_FACTOR=5`

## Example

Add `bsumall.c` to `bsv/src/`:

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

Build and run:

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

## Non Goals

Support of hardware other than little endian.

Types and schemas as a part of the data format.

## Testing Methodology

[Quickcheck](https://hypothesis.readthedocs.io/en/latest/) style [testing](https://github.com/nathants/bsv/blob/master/test) with Python implementations to verify correct behavior for arbitrary inputs and varying buffer sizes.

## Experiments

[Performance](https://github.com/nathants/bsv/blob/master/experiments/) experiments and alternate implementations.

## Related Projects

[s4](https://github.com/nathants/s4) - A storage cluster that is cheap and fast, with data local compute and efficient shuffle.

## Related Posts

[Optimizing A Bsv Data Processing Pipeline](https://nathants.com/posts/optimizing-a-bsv-data-processing-pipeline)

[Performant Batch Processing With Bsv, S4, And Presto](https://nathants.com/posts/performant-batch-processing-with-bsv-s4-and-presto)

[Discovering A Baseline For Data Processing Performance](https://nathants.com/posts/discovering-a-baseline-for-data-processing-performance)

[Refactoring Common Distributed Data Patterns Into S4](https://nathants.com/posts/refactoring-common-distributed-data-patterns-into-s4)

[Scaling Python Data Processing Horizontally](https://nathants.com/posts/scaling-python-data-processing-horizontally)

[Scaling Python Data Processing Vertically](https://nathants.com/posts/scaling-python-data-processing-vertically)

## More Examples

[Structured Analysis Of NYC Taxi Data With Bsv And Hive](https://github.com/nathants/s4/blob/master/examples/nyc_taxi_bsv)

## Tools

| name | description |
| -- | -- |
| [bcat](#bcat) | Cat some BSV files to CSV |
| [bcombine](#bcombine) | Prepend a new column by combining values from existing columns |
| [bcounteach](#bcounteach) | Count as i64 each contiguous identical row by the first column |
| [bcounteach-hash](#bcounteach-hash) | Count as i64 by hash of the first column |
| [bcountrows](#bcountrows) | Count rows as i64 |
| [bcut](#bcut) | Select some columns |
| [bdedupe](#bdedupe) | Dedupe identical contiguous rows by the first column, keeping the first |
| [bdedupe-hash](#bdedupe-hash) | Dedupe rows by hash of the first column, keeping the first |
| [bdropuntil](#bdropuntil) | For sorted input, drop until the first column is GTE to VALUE |
| [bhead](#bhead) | Keep the first n rows |
| [blz4](#blz4) | Compress BSV data |
| [blz4d](#blz4d) | Decompress BSV data |
| [bmerge](#bmerge) | Merge sorted files from stdin |
| [bpartition](#bpartition) | Split into multiple files by consistent hash of the first column value |
| [bquantile-merge](#bquantile-merge) | Merge DDSketches and output quantile value pairs as f64 |
| [bquantile-sketch](#bquantile-sketch) | Collapse the first column into a single row DDSketch |
| [bschema](#bschema) | Validate and converts row data with a schema of columns |
| [bsort](#bsort) | TimSort rows by the first column |
| [bsplit](#bsplit) | Split a stream into multiple files |
| [bsum](#bsum) | Sum the first column |
| [bsumeach](#bsumeach) | Sum the second column of each contiguous identical row by the first column |
| [bsumeach-hash](#bsumeach-hash) | Sum as i64 the second column by hash of the first column |
| [bsv](#bsv) | Convert CSV to BSV |
| [btake](#btake) | Take while the first column is VALUE |
| [btakeuntil](#btakeuntil) | For sorted input, take until the first column is GTE to VALUE |
| [btopn](#btopn) | Accumulate the top n rows in a heap by first column value |
| [bunzip](#bunzip) | Split a multi column input into single column outputs |
| [bzip](#bzip) | Combine single column inputs into a multi column output |
| [csv](#csv) | Convert BSV to CSV |
| [xxh3](#xxh3) | XXH3_64 hash stdin |

### [bcat](https://github.com/nathants/bsv/blob/master/src/bcat.c)

Cat some BSV files to CSV

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

Prepend a new column by combining values from existing columns

```bash
usage: ... | bcombine COL1,...,COLN
```

```bash
>> echo a,b,c | bsv | bcombine 3,2 | csv
b:a,a,b,c
```

### [bcounteach](https://github.com/nathants/bsv/blob/master/src/bcounteach.c)

Count as i64 each contiguous identical row by the first column

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

Count as i64 by hash of the first column

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

Count rows as i64

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

Select some columns

```bash
usage: ... | bcut COL1,...,COLN
```

```bash
>> echo a,b,c | bsv | bcut 3,3,3,2,2,1 | csv
c,c,c,b,b,a
```

### [bdedupe](https://github.com/nathants/bsv/blob/master/src/bdedupe.c)

Dedupe identical contiguous rows by the first column, keeping the first

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

Dedupe rows by hash of the first column, keeping the first

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

For sorted input, drop until the first column is GTE to VALUE

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

Keep the first n rows

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

Compress BSV data

```bash
usage: ... | blz4
```

```bash
>> echo a,b,c | bsv | blz4 | blz4d | csv
a,b,c
```

### [blz4d](https://github.com/nathants/bsv/blob/master/src/blz4d.c)

Decompress BSV data

```bash
usage: ... | blz4d
```

```bash
>> echo a,b,c | bsv | blz4 | blz4d | csv
a,b,c
```

### [bmerge](https://github.com/nathants/bsv/blob/master/src/bmerge.c)

Merge sorted files from stdin

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

Split into multiple files by consistent hash of the first column value

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

Merge DDSketches and output quantile value pairs as f64

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

Collapse the first column into a single row DDSketch

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

Validate and converts row data with a schema of columns

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

TimSort rows by the first column

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

Split a stream into multiple files

```bash
usage: ... | bsplit PREFIX [chunks_per_file=1]
```

```bash
>> echo -n a,b,c | bsv | bsplit prefix
prefix_0000000000
```

### [bsum](https://github.com/nathants/bsv/blob/master/src/bsum.c)

Sum the first column

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

Sum the second column of each contiguous identical row by the first column

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

Sum as i64 the second column by hash of the first column

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

Convert CSV to BSV

```bash
usage: ... | bsv
```

```bash
>> echo a,b,c | bsv | bcut 3,2,1 | csv
c,b,a
```

### [btake](https://github.com/nathants/bsv/blob/master/src/btake.c)

Take while the first column is VALUE

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

For sorted input, take until the first column is GTE to VALUE

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

Accumulate the top n rows in a heap by first column value

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

Split a multi column input into single column outputs

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

Combine single column inputs into a multi column output

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

Convert BSV to CSV

```bash
usage: ... | csv
```

```bash
>> echo a,b,c | bsv | csv
a,b,c
```

### [xxh3](https://github.com/nathants/bsv/blob/master/src/xxh3.c)

XXH3_64 hash stdin

```bash
usage: ... | xxh3 [--stream|--int]
```

```bash
  --stream pass stdin through to stdout with hash on stderr

  --int output hash as int not hash

>> echo abc | xxh3
079364cbfdf9f4cb
```