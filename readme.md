## why

it should be possible to process data at speeds approaching that of sequential io.

## what

a simple and efficient [data](https://github.com/nathants/bsv/blob/master/util/load.h) [format](https://github.com/nathants/bsv/blob/master/util/dump.h) for sequentially manipulating chunks of rows of columns while minimizing allocating and copying.

[cli](https://github.com/nathants/bsv/blob/master/src) utilities based on [shared](https://github.com/nathants/bsv/blob/master/util) code.

## how

column: 0-65536 bytes.

row: 0-65536 columns.

chunk: up to 5MB containing 1 or more complete rows.

note: row data cannot exceed chunk size.

## layout

[chunk](https://github.com/nathants/bsv/blob/master/util/read.h): `| i32:chunk_size | row-1 | ... | row-n |`

[row](https://github.com/nathants/bsv/blob/master/util/row.h): `| u16:max | u16:size-1 | ... | u16:size-n | u8:column-1 + \0 | ... | u8:column-n + \0 |`

note: column bytes are always followed by a single nullbyte: `\0`

note: max is the maximum zero based index into the row: `max = size(row) - 1`

## non goals

support of hardware other than little endian.

explicit types and schemas.

## testing methodology

[quickcheck](https://hypothesis.readthedocs.io/en/latest/) style [testing](https://github.com/nathants/bsv/blob/master/test) with python implementations of every utility to verify correct behavior for arbitrary inputs and varying buffer sizes.

## experiments

[performance](https://github.com/nathants/bsv/blob/master/experiments/readme.md) experiments and alternate implementations.

## utilities

- [bcat](#bcat) - cat some bsv files to csv
- [bcat-lz4](#bcat-lz4) - cat some compressed bsv files to csv
- [bcopy](#bcopy) - pass through data, to benchmark load/dump performance
- [bcounteach](#bcounteach) - count as u64 each contiguous identical row by strcmp the first column
- [bcounteach-hash](#bcounteach-hash) - count as u64 by hashmap of the first column
- [bcountrows](#bcountrows) - count rows as u64
- [bcut](#bcut) - select some columns
- [bdedupe](#bdedupe) - dedupe identical contiguous rows by strcmp the first column, keeping the first
- [bdropuntil](#bdropuntil) - drop until the first column is gte to VALUE
- [blz4](#blz4) - compress bsv data
- [blz4d](#blz4d) - decompress bsv data
- [bmerge](#bmerge) - merge sorted files from stdin
- [bmerge-lz4](#bmerge-lz4) - merge compressed sorted files from stdin
- [bpartition](#bpartition) - split into multiple files by consistent hash of the first column value
- [bpartition-lz4](#bpartition-lz4) - split into multiple compressed files by consistent hash of the first column value
- [brmerge](#brmerge) - merge reverse sorted files from stdin
- [brmerge-lz4](#brmerge-lz4) - merge compressed reverse sorted files from stdin
- [brsort](#brsort) - reverse timsort rows by strcmp the first column
- [bschema](#bschema) - validate and converts row data with a schema of columns
- [bsort](#bsort) - timsort rows by strcmp the first column
- [bsplit](#bsplit) - split a stream into multiple files
- [bsumeach-f64](#bsumeach-f64) - sum as f64 the second colum of each contiguous identical row by strcmp the first column
- [bsumeach-hash-f64](#bsumeach-hash-f64) - sum as f64 the second colum by hashmap of the first column
- [bsumeach-hash-u64](#bsumeach-hash-u64) - sum as u64 the second colum by hashmap of the first column
- [bsumeach-u64](#bsumeach-u64) - sum as u64 the second colum of each contiguous identical row by strcmp the first column
- [bsum-u64](#bsum-u64) - u64 sum the first column
- [bsv](#bsv) - convert csv to bsv
- [btake](#btake) - take while the first column is VALUE
- [btakeuntil](#btakeuntil) - take until the first column is gte to VALUE
- [bunzip](#bunzip) - split a multi column input into single column outputs
- [bunzip-lz4](#bunzip-lz4) - split a multi column input into compressed single column outputs
- [bzip](#bzip) - combine single column inputs into a multi column output
- [bzip-lz4](#bzip-lz4) - combine compressed single column inputs into a multi column output
- [csv](#csv) - convert bsv to csv
- [xxh3](#xxh3) - xxh3_64 hash stdin. defaults to hex, can be --int. --stream to pass stdin through to stdout with hash on stderr

### [bcat](https://github.com/nathants/bsv/blob/master/src/bcat.c)

cat some bsv files to csv

usage: `bcat [--prefix] [--head NUM] FILE1 ... FILEN`

```
>> for char in a a b b c c; do
     echo $char | bsv >> /tmp/$char
   done

>> bcat --head 1 --prefix /tmp/{a,b,c}
/tmp/a:a
/tmp/b:b
/tmp/c:c
```

### [bcat-lz4](https://github.com/nathants/bsv/blob/master/src/bcat-lz4.c)

cat some compressed bsv files to csv

usage: `bcat-lz4 [--prefix] [--head NUM] FILE1 ... FILEN`

```
>> for char in a a b b c c; do
     echo $char | bsv | blz4 >> /tmp/$char
   done

>> bcat-lz4 --head 1 --prefix /tmp/{a,b,c}
/tmp/a:a
/tmp/b:b
/tmp/c:c
```

### [bcopy](https://github.com/nathants/bsv/blob/master/src/bcopy.c)

pass through data, to benchmark load/dump performance

usage: `... | bcopy`

```
>> echo a,b,c | bsv | bcopy | csv
a,b,c
```

### [bcounteach](https://github.com/nathants/bsv/blob/master/src/bcounteach.c)

count as u64 each contiguous identical row by strcmp the first column

usage: `... | bcounteach`

```
echo '
a
a
b
b
b
a
' | bsv | bcounteach | bschema *,u64:a | csv
a,2
b,3
a,1
```

### [bcounteach-hash](https://github.com/nathants/bsv/blob/master/src/bcounteach-hash.c)

count as u64 by hashmap of the first column

usage: `... | bcounteach-hash`

```
echo '
a
a
b
b
b
a
' | bsv | bcounteach-hash | bschema *,u64:a | bsort | csv
a,3
b,3
```

### [bcountrows](https://github.com/nathants/bsv/blob/master/src/bcountrows.c)

count rows as u64

usage: `... | bcountrows`

```
>> echo '
1
2
3
4.1
' | bsv | bcountrows | csv
4
```

### [bcut](https://github.com/nathants/bsv/blob/master/src/bcut.c)

select some columns

usage: `... | bcut COL1,...,COLN`

```
>> echo a,b,c | bsv | bcut 3,3,3,2,2,1 | csv
c,c,c,b,b,a
```

### [bdedupe](https://github.com/nathants/bsv/blob/master/src/bdedupe.c)

dedupe identical contiguous rows by strcmp the first column, keeping the first

usage: `... | bdedupe`

```
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

### [bdropuntil](https://github.com/nathants/bsv/blob/master/src/bdropuntil.c)

drop until the first column is gte to VALUE

usage: `... | bdropuntil VALUE`

```
>> echo '
a
b
c
d
' | bsv | bdropuntil c | csv
c
d
```

### [blz4](https://github.com/nathants/bsv/blob/master/src/blz4.c)

compress bsv data

usage: `... | blz4`

```
>> echo a,b,c | bsv | blz4 | blz4d | csv
a,b,c
```

### [blz4d](https://github.com/nathants/bsv/blob/master/src/blz4d.c)

decompress bsv data

usage: `... | blz4d`

```
>> echo a,b,c | bsv | blz4 | blz4d | csv
a,b,c
```

### [bmerge](https://github.com/nathants/bsv/blob/master/src/bmerge.c)

merge sorted files from stdin

usage: `echo FILE1 ... FILEN | bmerge`

```
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

### [bmerge-lz4](https://github.com/nathants/bsv/blob/master/src/bmerge-lz4.c)

merge compressed sorted files from stdin

usage: `echo FILE1 ... FILEN | bmerge-lz4`

```
>> echo -e 'a
c
e
' | bsv | blz4 > a.bsv
>> echo -e 'b
d
f
' | bsv | blz4 > b.bsv
>> echo a.bsv b.bsv | bmerge-lz4
a
b
c
d
e
f
```

### [bpartition](https://github.com/nathants/bsv/blob/master/src/bpartition.c)

split into multiple files by consistent hash of the first column value

usage: `... | bpartition NUM_BUCKETS [PREFIX]`

```
>> echo '
a\b
c
' | bsv | bpartition 10 prefix
prefix03
prefix06
```

### [bpartition-lz4](https://github.com/nathants/bsv/blob/master/src/bpartition-lz4.c)

split into multiple compressed files by consistent hash of the first column value

usage: `... | bpartition-lz4 NUM_BUCKETS [PREFIX]`

```
>> echo '
a
b
c
' | bsv | bpartition-lz4 10 prefix
prefix03
prefix06
```

### [brmerge](https://github.com/nathants/bsv/blob/master/src/brmerge.c)

merge reverse sorted files from stdin

usage: `echo FILE1 ... FILEN | brmerge`

```
>> echo -e 'e
c
a
' | bsv > a.bsv
>> echo -e 'f
d
b
' | bsv > b.bsv
>> echo a.bsv b.bsv | brmerge
f
e
d
c
b
a
```

### [brmerge-lz4](https://github.com/nathants/bsv/blob/master/src/brmerge-lz4.c)

merge compressed reverse sorted files from stdin

usage: `echo FILE1 ... FILEN | brmerge-lz4`

```
>> echo -e 'e
c
a
' | bsv | blz4 > a.bsv
>> echo -e 'f
d
b
' | bsv | blz4 > b.bsv
>> echo a.bsv b.bsv | brmerge-lz4
f
e
d
c
b
a
```

### [brsort](https://github.com/nathants/bsv/blob/master/src/brsort.c)

reverse timsort rows by strcmp the first column

usage: `... | bsort`

```
>> echo '
a
b
c
' | bsv | brsort | csv
c
b
a
```

### [bschema](https://github.com/nathants/bsv/blob/master/src/bschema.c)

validate and converts row data with a schema of columns

usage: `... | bschema SCHEMA [--filter]`

```
>> echo aa,bbb,cccc | bsv | bschema 2,3,4 | csv
aa,bbb,cccc
```

### [bsort](https://github.com/nathants/bsv/blob/master/src/bsort.c)

timsort rows by strcmp the first column

usage: `... | bsort`

```
>> echo '
c
b
a
' | bsv | bsort | csv
a
b
c
```

### [bsplit](https://github.com/nathants/bsv/blob/master/src/bsplit.c)

split a stream into multiple files

usage: `... | bsplit [chunks_per_file=1]`

```
>> echo -n a,b,c | bsv | bsplit
1595793589_0000000000
```

### [bsumeach-f64](https://github.com/nathants/bsv/blob/master/src/bsumeach-f64.c)

sum as f64 the second colum of each contiguous identical row by strcmp the first column

usage: `... | bsumeach-f64`

```
echo '
a,1.1
a,2.1
b,3.1
b,4.1
b,5.1
a,6.1
' | bsv | bschema *,a:f64 | bsumeach-f64 | bschema *,f64:a | csv
a,3.200000
b,12.300000
a,6.100000
```

### [bsumeach-hash-f64](https://github.com/nathants/bsv/blob/master/src/bsumeach-hash-f64.c)

sum as f64 the second colum by hashmap of the first column

usage: `... | bsumeach-hash-f64`

```
echo '
a,1
a,2
b,3
b,4
b,5
a,6
' | bsv | bschema *,a:f64 | bsumeach-hash-f64 | bschema *,f64:a | csv
a,3
b,12
a,6
```

### [bsumeach-hash-u64](https://github.com/nathants/bsv/blob/master/src/bsumeach-hash-u64.c)

sum as u64 the second colum by hashmap of the first column

usage: `... | bsumeach-hash-u64`

```
echo '
a,1
a,2
b,3
b,4
b,5
a,6
' | bsv | bschema *,a:u64 | bsumeach-hash-u64 | bschema *,u64:a | csv
a,3
b,12
a,6
```

### [bsumeach-u64](https://github.com/nathants/bsv/blob/master/src/bsumeach-u64.c)

sum as u64 the second colum of each contiguous identical row by strcmp the first column

usage: `... | bsumeach-u64`

```
echo '
a,1
a,2
b,3
b,4
b,5
a,6
' | bsv | bschema *,a:u64 | bsumeach-u64 | bschema *,u64:a | csv
a,3
b,12
a,6
```

### [bsum-u64](https://github.com/nathants/bsv/blob/master/src/bsum-u64.c)

u64 sum the first column

usage: `... | bsum-u64`

```
>> echo -e '1
2
3
4
' | bsv | bschema a:u64 | bsum-u64 | bschema u64:a | csv
10
```

### [bsv](https://github.com/nathants/bsv/blob/master/src/bsv.c)

convert csv to bsv

usage: `... | bsv`

```
>> echo a,b,c | bsv | bcut 3,2,1 | csv
c,b,a
```

### [btake](https://github.com/nathants/bsv/blob/master/src/btake.c)

take while the first column is VALUE

usage: `... | btake VALUE`

```
>> echo '
a
b
c
d
' | bsv | bdropntil c | btake c | csv
c
```

### [btakeuntil](https://github.com/nathants/bsv/blob/master/src/btakeuntil.c)

take until the first column is gte to VALUE

usage: `... | btakeuntil VALUE`

```
>> echo '
a
b
c
d
' | bsv | btakeuntil c | csv
a
b
```

### [bunzip](https://github.com/nathants/bsv/blob/master/src/bunzip.c)

split a multi column input into single column outputs

usage: `... | bunzip PREFIX`

```
>> echo '
a,b,c
1,2,3
' | bsv | bunzip col && echo col_0 col_2 | bzip | csv
a,c
1,3
```

### [bunzip-lz4](https://github.com/nathants/bsv/blob/master/src/bunzip-lz4.c)

split a multi column input into compressed single column outputs

usage: `... | bunzip-lz4 PREFIX`

```
>> echo '
a,b,c
1,2,3
' | bsv | bunzip-lz4 column && ls column_* | bzip-lz4 1,3 | csv
a,c
1,3
```

### [bzip](https://github.com/nathants/bsv/blob/master/src/bzip.c)

combine single column inputs into a multi column output

usage: `ls column_* | bzip [COL1,...COLN]`

```
>> echo '
a,b,c
1,2,3
' | bsv | bunzip column && ls column_* | bzip 1,3 | csv
a,c
1,3
```

### [bzip-lz4](https://github.com/nathants/bsv/blob/master/src/bzip-lz4.c)

combine compressed single column inputs into a multi column output

usage: `ls column_* | bzip-lz4 [COL1,...COLN]`

```
>> echo '
a,b,c
1,2,3
' | bsv | bunzip-lz4 column && ls column_* | bzip-lz4 1,3 | csv
a,c
1,3
```

### [csv](https://github.com/nathants/bsv/blob/master/src/csv.c)

convert bsv to csv

usage: `... | csv`

```
>> echo a,b,c | bsv | csv
a,b,c
```

### [xxh3](https://github.com/nathants/bsv/blob/master/src/xxh3.c)

xxh3_64 hash stdin. defaults to hex, can be --int. --stream to pass stdin through to stdout with hash on stderr

usage: `... | xxh3 [--stream|--int]`

```
>> echo abc | xxh3
B5CA312E51D77D64
```
