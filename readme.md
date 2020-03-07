## why

it should be possible to process data faster than sequential io, and sequential io is fast now.

## what

cli utilites to combine into pipelines.

## utilities

- [bbucket](#bbucket) - prefix each row with a consistent hash of the first column
- [bcat](#bcat) - cat some bsv file to csv
- [bcounteach](#bcounteach) - count and collapse each contiguous identical row
- [bcountrows](#bcountrows) - count rows
- [bcut](#bcut) - select some columns
- [bdedupe](#bdedupe) - dedupe identical contiguous lines
- [bdisjoint](#bdisjoint) - given sorted files, create new files with deduped values not in multiple files
- [bdropuntil](#bdropuntil) - drop until the first column is gte to VALUE
- [bmerge](#bmerge) - merge sorted files
- [bpartition](#bpartition) - split into multiple files by the first column value
- [brmerge](#brmerge) - merge reverse sorted files
- [brsort](#brsort) - reverse sort rows
- [bsort](#bsort) - PROBABLY DO NOT USE THIS, use bsv,csv,coreutils-sort
- [bsplit](#bsplit) - split a stream into a file per chunk. files are named after the hash of the first chunk and then numbered
- [bsum](#bsum) - integer sum numbers in the first column and output a single value
- [bsv](#bsv) - convert csv to bsv
- [btake](#btake) - take while the first column is VALUE
- [btakeuntil](#btakeuntil) - take until the first column is gte to VALUE
- [csv](#csv) - convert bsv to csv
- [xxh3](#xxh3) - xxh3_64 hash stdin, defaults to hex, can be --int, or --stream to hex and pass stdin through

### [bbucket](https://github.com/nathants/bsv/blob/master/src/bbucket.c)

prefix each row with a consistent hash of the first column

usage: `... | bbucket NUM_BUCKETS`

```
>> echo '
a
b
c
' | bsv | bbucket 100 | csv
50,a
39,b
83,c
```

### [bcat](https://github.com/nathants/bsv/blob/master/src/bcat.c)

cat some bsv file to csv

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

### [bcounteach](https://github.com/nathants/bsv/blob/master/src/bcounteach.c)

count and collapse each contiguous identical row

usage: `... | bcounteach`

```
echo 'a
a
b
b
b
a
' | bsv | bcounteach | csv
a,2
b,3
a,1
```

### [bcountrows](https://github.com/nathants/bsv/blob/master/src/bcountrows.c)

count rows

usage: `... | bcountrows`

```
>> echo -e '1
2
3
4.1
' | bsv | bcountrows | csv
4
```

### [bcut](https://github.com/nathants/bsv/blob/master/src/bcut.c)

select some columns

usage: `... | bcut FIELD1,...,FIELDN`

```
>> echo a,b,c | bsv | bcut 3,3,3,2,2,1 | csv
c,c,c,b,b,a
```

### [bdedupe](https://github.com/nathants/bsv/blob/master/src/bdedupe.c)

dedupe identical contiguous lines

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

### [bdisjoint](https://github.com/nathants/bsv/blob/master/src/bdisjoint.c)

given sorted files, create new files with deduped values not in multiple files

usage: `... | bdisjoint SUFFIX FILE1 ... FILEN`

```
>> echo '
1
2
' | bsv > a

>> echo '
2
3
4
' | bsv > b

>> echo '
4
5
5
' | bsv > c

>> bdisjoint out a b c

>> csv < a.out
1

>> csv < b.out
3

>> csv < c.out
5
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

### [bmerge](https://github.com/nathants/bsv/blob/master/src/bmerge.c)

merge sorted files

usage: `bmerge FILE1 FILE2`

```
>> echo -e 'a
c
e
' | bsv > a.bsv
>> echo -e 'b
d
f
' | bsv > b.bsv
>> bmerge a.bsv b.bsv
a
b
c
d
e
f
```

### [bpartition](https://github.com/nathants/bsv/blob/master/src/bpartition.c)

split into multiple files by the first column value

usage: `... | bbucket NUM_BUCKETS | bpartition PREFIX NUM_BUCKETS`

```
>> echo '
0,a
1,b
2,c
' | bsv | bpartition prefix 10
prefix00
prefix01
prefix02
```

### [brmerge](https://github.com/nathants/bsv/blob/master/src/brmerge.c)

merge reverse sorted files

usage: `brmerge FILE1 FILE2`

```
>> echo -e 'e
c
a
' | bsv > a.bsv
>> echo -e 'f
d
b
' | bsv > b.bsv
>> brmerge a.bsv b.bsv
f
e
d
c
b
a
```

### [brsort](https://github.com/nathants/bsv/blob/master/src/brsort.c)

reverse sort rows

usage: `... | brsort`

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

### [bsort](https://github.com/nathants/bsv/blob/master/src/bsort.c)

PROBABLY DO NOT USE THIS, use bsv,csv,coreutils-sort

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

split a stream into a file per chunk. files are named after the hash of the first chunk and then numbered

usage: `... | bsplit`

```
>> echo -n a,b,c | bsv | bsplit
1595793589_0000000000
```

### [bsum](https://github.com/nathants/bsv/blob/master/src/bsum.c)

integer sum numbers in the first column and output a single value

usage: `... | bsum`

```
>> echo -e '1
2
3
4.1
' | bsv | bsum | csv
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

### [csv](https://github.com/nathants/bsv/blob/master/src/csv.c)

convert bsv to csv

usage: `... | csv`

```
>> echo a,b,c | bsv | csv
a,b,c
```

### [xxh3](https://github.com/nathants/bsv/blob/master/src/xxh3.c)

xxh3_64 hash stdin, defaults to hex, can be --int, or --stream to hex and pass stdin through

usage: `... | xxh3 [--stream|--int]`

```
>> echo abc | xxh3
B5CA312E51D77D64
```