## why

it should be possible to process data faster than io, and io is fast now.

## what

small fast cli utilites to combine into processing pipelines.

## utilities

- [bbucket](#bbucket) - prefix each row with a consistent hash of the first column

- [bcounteach](#bcounteach) - count and collapse each contiguous identical row

- [bcut](#bcut) - select some columns

- [bsv](#bsv) - convert csv to bsv

- [csv](#csv) - convert bsv to csv

### bbucket

prefix each row with a consistent hash of the first column

usage: `bbucket NUM_BUCKETS`

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

usage: `bcounteach`

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

usage: `bcut FIELD1,...,FIELDN`

```
>> echo a,b,c | bsv | bcut 3,3,3,2,2,1 | csv
c,c,c,b,b,a
```
### bsv

convert csv to bsv

usage: `bsv`

```
>> echo a,b,c | bsv | bcut 3,2,1 | csv
c,b,a
```
### csv

convert bsv to csv

usage: `csv`

```
>> echo a,b,c | bsv | csv
a,b,c
```