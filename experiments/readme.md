### experiments with alternate implementations of csv and bsv serialization and processing

##### running on this spec
```
>> uname -a
Linux 5.4.33-3-lts x86_64 GNU/Linux

>> lscpu | grep CPU
CPU(s):                          6
CPU max MHz:                     4400.0000
CPU min MHz:                     800.0000
```

##### ramfs
```
cd /tmp
```

##### build bsv and put bin on PATH
```
>> (cd ~/repos/bsv && make)
>> export PATH=$PATH:~/repos/bsv/bin
```

##### make some csv
```
>> time _gen_csv 8 25000000 >data.csv
real    0m7.360s
user    0m6.677s
sys     0m0.680s
```

##### convert it to bsv
```
>> time bsv_plain <data.csv >data.bsv
real    0m6.864s
user    0m5.801s
sys     0m1.058s
```

##### parsing numerics with `bsv` currently has some extra overhead, so we use `bsv_plain` since we have no numeric data
```
>> time bsv <data.csv >/dev/null
real    0m9.839s
user    0m9.606s
sys     0m0.230s
```

##### see how well the data compresses
```
> time lz4 <data.csv >data.csv.lz4
real    0m5.135s
user    0m4.782s
sys     0m0.349s

>> time lz4 <data.bsv >data.bsv.lz4
real    0m6.876s
user    0m6.374s
sys     0m0.500s
```

##### check the sizes, bsv trades space for time compared with csv
```
>> ls -lh data.* | cut -d' ' -f5,9
2.4G data.bsv
1.1G data.bsv.lz4
1.8G data.csv
779M data.csv.lz4
```

##### check versions
```
>> lz4 --version
*** LZ4 command line interface 64-bits v1.9.2, by Yann Collet ***

>> xsv --version
0.13.0

>> sort --version
sort (GNU coreutils) 8.32

>> cut --version
cut (GNU coreutils) 8.32

>> gcc --version
gcc (Arch Linux 9.3.0-1) 9.3.0

>> python --version
Python 3.8.2

>> pypy --version
[PyPy 7.3.1 with GCC 9.3.0]

>> go version
go version go1.14.2 linux/amd64

>> java -version
openjdk version "13.0.2" 2020-01-14
```

##### copy the experiments and make sure they all get the same result
```
>> cp ~/repos/bsv/experiments/* .

>> cut -d, -f3,7 <data.csv | xxh3
9135bc839b1f6beb

>> go build bcut.go
>> ./bcut 3,7 <data.csv | xxh3
9135bc839b1f6beb

>> python bcut.py 3,7 <data.csv | xxh3
9135bc839b1f6beb

>> pypy bcut.py 3,7 <data.csv | xxh3
9135bc839b1f6beb

>> javac BCut.java
>> java BCut 3,7 <data.csv | xxh3
9135bc839b1f6beb

>> cargo install xsv
>> xsv select 3,7 <data.csv | xxh3
9135bc839b1f6beb

>> bcut 3,7 <data.bsv | csv | xxh3
9135bc839b1f6beb

>> bsv_plain <data.csv | bcut 3,7 | csv | xxh3
9135bc839b1f6beb

```

##### coreutils cut is a good baseline
```
>> time cut -d, -f3,7 <data.csv >/dev/null
real    0m6.017s
user    0m5.657s
sys     0m0.340s
```

##### xsv meets that baseline
```
>> time xsv select 3,7 <data.csv >/dev/null
real    0m6.066s
user    0m5.774s
sys     0m0.290s
```

##### java is slower
```
>> time java BCut 3,7 <data.csv >/dev/null
real    0m11.727s
user    0m11.601s
sys     0m0.463s
```

##### go is slower
```
>> time ./bcut 3,7 <data.csv >/dev/null
real    0m10.765s
user    0m11.515s
sys     0m0.519s
```

##### pypy is slower
```
>> time pypy bcut.py 3,7 <data.csv >/dev/null
real    0m13.766s
user    0m13.372s
sys     0m0.360s
```

##### cpython is even slower
```
>> time python bcut.py 3,7 <data.csv >/dev/null
real    0m25.491s
user    0m25.323s
sys     0m0.160s
```

##### bcut is faster
```
>> time bcut 3,7 <data.bsv >/dev/null
real    0m1.126s
user    0m0.875s
sys     0m0.251s
```

##### bcut going back to csv adds some overhead, aggregating before going back to csv is ideal
```
>> time bcut 3,7 <data.bsv | csv >/dev/null
real    0m2.018s
user    0m1.522s
sys     0m0.509s
```

##### the only random access that should ever be happening is sort, again coreutils is a good baseline
```
>> LC_ALL=C sort --parallel=1 -S50% -k1,1 <data.csv | cut -d, -f1 | xxh3
60ea4f93b87d0cf5

>> time bash -c 'LC_ALL=C sort --parallel=1 -S50% -k1,1 <data.csv >/dev/null'
real    0m22.406s
user    0m21.516s
sys     0m0.880s
```

##### bsort is faster, all credit to [tim and swenson](https://github.com/swenson/sort)
```
>> bsort <data.bsv | bcut 1 | csv | xxh3
60ea4f93b87d0cf5

>> time bsort <data.bsv >/dev/null
real    0m13.558s
user    0m12.266s
sys     0m1.139s
```

##### merge is a complement to sort, first lets split our file into smaller pieces, and then duplicate those to csv
```
>> bsplit 5 <data.bsv >filenames.txt
>> cat filenames.txt | while read path; do csv <$path >csv.$path; done
```

##### check the sizes, we did 5 chunks per file with bsplit, and the chunksize is 5MB
```
>> ls -lh $(head filenames.txt) | cut -d' ' -f5,9
25M 5081573886961224_0000000000
25M 5081573886961224_0000000001
25M 5081573886961224_0000000002

>> ls -lh csv.* | head -n3 | cut -d' ' -f5,9
19M csv.5081573886961224_0000000000
19M csv.5081573886961224_0000000001
19M csv.5081573886961224_0000000002
```

##### sort each piece
```
>> time for path in $(cat filenames.txt); do bsort <$path >$path.sorted; done
real    0m10.768s
user    0m8.722s
sys     0m2.012s

>> time bash -c 'for path in csv.*; do LC_ALL=C sort -k1,1 --parallel=1 -S50% <$path >$path.sorted; done'
real    0m12.033s
user    0m10.590s
sys     0m1.428s
```

##### merge the sorted pieces and make sure they get the same result
```
>> LC_ALL=C sort -m -k1,1 -S50% csv.*.sorted | cut -d, -f1 | xxh3
60ea4f93b87d0cf5

bmerge $(cat filenames.txt | while read path; do echo $path.sorted; done) | bcut 1 | csv | xxh3
60ea4f93b87d0cf5
```

##### coreutils sort -m is a good baseline, bmerge is faster, all credit to [armon](https://github.com/statsite/statsite/blob/master/src/heap.c)
```
>> time bash -c 'LC_ALL=C sort -m -k1,1 -S50% csv.*.sorted >/dev/null'
real    0m8.846s
user    0m6.692s
sys     0m2.149s

>> time bmerge $(cat filenames.txt | while read path; do echo $path.sorted; done) >/dev/null
real    0m1.361s
user    0m0.911s
sys     0m0.450s
```
