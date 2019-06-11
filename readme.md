## why

it should be possible to process data faster than sequential io, and sequential io is fast now.

## what

small fast cli utilites to combine into processing pipelines.

## utilities

- [bbucket](#bbucket) - prefix each row with a consistent hash of the first column
- [bcounteach](#bcounteach) - count and collapse each contiguous identical row
- [bcut](#bcut) - select some columns
- [bdedupe](#bdedupe) - dedupe identical contiguous lines
- [bdisjoint](#bdisjoint) - given sorted files, create new files with deduped values not in multiple files
- [bdropuntil](#bdropuntil) - drop until the first column is gte to VALUE
- [bsort](#bsort) - sort rows
- [bsv](#bsv) - convert csv to bsv
- [csv](#csv) - convert bsv to csv

### bbucket

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

### bcounteach

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

### bcut

select some columns

usage: `... | bcut FIELD1,...,FIELDN`

```
>> echo a,b,c | bsv | bcut 3,3,3,2,2,1 | csv
c,c,c,b,b,a
```

### bdedupe

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

### bdisjoint

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

### bdropuntil

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

### bsort

sort rows

usage: `... | sort`

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

### bsv

convert csv to bsv

usage: `... | bsv`

```
>> echo a,b,c | bsv | bcut 3,2,1 | csv
c,b,a
```

### csv

convert bsv to csv

usage: `... | csv`

```
>> echo a,b,c | bsv | csv
a,b,c
```