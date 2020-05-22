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

##### increase max pipe size to 5MB
```
>> sudo sysctl fs.pipe-max-size=5242880
```

##### make sure we dealing with bytes only
```
>> export LC_ALL=C
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
>> bsv <data.csv >data.bsv
>> time bsv <data.csv >/dev/null
real    0m5.115s
user    0m4.893s
sys     0m0.220s
```

##### see how well the data compresses
```
>> time lz4 <data.csv >data.csv.lz4
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
2.2G data.bsv
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

>> rustc --version
rustc 1.43.1
```

##### copy the experiments and make sure they all get the same result
```
>> cp -r ~/repos/bsv/experiments/* .

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

>> bsv <data.csv | bcut 3,7 | csv | xxh3
9135bc839b1f6beb

>> (cd bcut_rust && cargo build --release)
>> ./bcut_rust/target/release/bcut 3,7 <data.csv | xxh3
9135bc839b1f6beb

```

##### coreutils cut is a good baseline
```
>> time cut -d, -f3,7 <data.csv >/dev/null
real    0m5.784s
user    0m5.472s
sys     0m0.311s
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

##### rust is faster
```
>> time ./bcut_rust/target/release/bcut 3,7 <data.csv >/dev/null
real    0m4.336s
user    0m4.073s
sys     0m0.184s
```

##### bcut is faster
```
>> time bcut 3,7 <data.bsv >/dev/null
real    0m1.010s
user    0m0.729s
sys     0m0.280s
```

##### conversions to and from csv have a cost, best to minimze them
```
>> time bsv < data.csv | bcut 3,7 | csv >/dev/null
real    0m6.253s
user    0m6.927s
sys     0m1.766s
```

##### the only random access that should ever be happening is sort, again coreutils is a good baseline
```
>> sort --parallel=1 -S50% -k1,1 <data.csv | cut -d, -f1 | xxh3
60ea4f93b87d0cf5

>> time sort --parallel=1 -S50% -k1,1 <data.csv >/dev/null
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

>> time for path in csv.*; do sort -k1,1 --parallel=1 -S50% <$path >$path.sorted; done
real    0m12.033s
user    0m10.590s
sys     0m1.428s
```

##### merge the sorted pieces and make sure they get the same result
```
>> sort -m -k1,1 -S50% csv.*.sorted | cut -d, -f1 | xxh3
60ea4f93b87d0cf5

bmerge $(cat filenames.txt | while read path; do echo $path.sorted; done) | bcut 1 | csv | xxh3
60ea4f93b87d0cf5
```

##### coreutils sort -m is a good baseline, bmerge is faster, all credit to [armon](https://github.com/statsite/statsite/blob/master/src/heap.c)
```
>> time sort -m -k1,1 -S50% csv.*.sorted >/dev/null
real    0m8.846s
user    0m6.692s
sys     0m2.149s

>> time bmerge $(cat filenames.txt | while read path; do echo $path.sorted; done) >/dev/null
real    0m1.361s
user    0m0.911s
sys     0m0.450s
```

##### if you have sorted data, you can drop rows before a given value efficiently

```
>> bsort <data.bsv > data.bsv.sorted

>> token=$(csv < data.bsv.sorted | tail -n+23000000 | head -n1 | cut -d, -f1)

>> time grep "^$token," < data.csv | wc -l
24933

real    0m1.063s
user    0m0.891s
sys     0m0.189s

>> time bdropuntil $token < data.bsv.sorted | btake $token | bcountrows | csv
24933

real    0m0.246s
user    0m0.035s
sys     0m0.225s
```

##### something like coreutils cut with go and protobuf

```
>> cd psv_go
>> go build csv.go
>> go build psv.go

>> time ./psv < /tmp/data.csv > /tmp/data.psv

real    0m20.410s
user    0m20.985s
sys     0m1.350s

>> time ./csv < /tmp/data.psv >/dev/null

real    0m27.454s
user    0m29.279s
sys     0m0.818s

```

##### try protobuf decode again with c

```
>> cd psv_c
>> gcc -Irow -flto -O3 -march=native -mtune=native csv.c row/*.c -o csv

>> time ./csv < /tmp/data.psv >/dev/null

real    0m4.367s
user    0m4.115s
sys     0m0.250s
```
