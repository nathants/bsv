### experiments with alternate implementations of bcut

##### ramfs
```bash
cd /tmp
```

##### build bsv and put bin on PATH
```bash
>> (cd ~/repos/bsv && make)
>> export PATH=$PATH:~/repos/bsv/bin
```

##### increase max pipe size to 5MB
```bash
>> sudo sysctl fs.pipe-max-size=5242880
```

##### make sure we are dealing with bytes only
```bash
>> export LC_ALL=C
```

##### make some csv
```bash
>> time _gen_csv 8 25000000 >data.csv
real    0m7.360s
user    0m6.677s
sys     0m0.680s
```

##### convert it to bsv
```bash
>> bsv <data.csv >data.bsv
>> time bsv <data.csv >/dev/null
real    0m5.115s
user    0m4.893s
sys     0m0.220s
```

##### see how well the data compresses
```bash
>> time lz4 <data.csv >data.csv.lz4
real    0m5.135s
user    0m4.782s
sys     0m0.349s

>> time lz4 <data.bsv >data.bsv.lz4
real    0m6.876s
user    0m6.374s
sys     0m0.500s
```

##### check the sizes, bsv trades space for time
```bash
>> ls -lh data.* | cut -d' ' -f5,9
2.2G data.bsv
1.1G data.bsv.lz4
1.8G data.csv
779M data.csv.lz4
```

##### copy the experiments and make sure they all get the same result
```bash
>> cp ~/repos/bsv/experiments/bcut/* .
>> cp -r ~/repos/bsv/util .
>> cp -r ~/repos/bsv/vendor .

>> cut -d, -f3,7 <data.csv | xxh3
9135bc839b1f6beb

>> go build -o bcut_go bcut.go
>> ./bcut_go 3,7 <data.csv | xxh3
9135bc839b1f6beb

>> pypy bcut.py 3,7 <data.csv | xxh3
9135bc839b1f6beb

>> rustc -O -o bcut_rust bcut.rs
>> ./bcut_rust 3,7 <data.csv | xxh3
9135bc839b1f6beb

>> gcc -Ivendor -Iutil -O3 -flto -march=native -mtune=native -o bcut_c bcut.c
>> ./bcut_c 3,7 <data.csv | xxh3
9135bc839b1f6beb

>> bcut 3,7 <data.bsv | csv | xxh3
9135bc839b1f6beb

```

##### coreutils cut is a good baseline
```bash
>> time cut -d, -f3,7 <data.csv >/dev/null
real    0m5.784s
user    0m5.472s
sys     0m0.311s
```

##### pypy is slower
```bash
>> time pypy bcut.py 3,7 <data.bsv >/dev/null
real    0m9.917s
user    0m9.443s
sys     0m0.450s
```

##### go is faster
```bash
>> time ./bcut_go 3,7 <data.bsv >/dev/null
real    0m2.179s
user    0m1.870s
sys     0m0.312s
```

##### rust is faster
```bash
>> time ./bcut_rust 3,7 <data.bsv >/dev/null
real    0m1.343s
user    0m1.139s
sys     0m0.203s
```

##### c is faster
```bash
>> time ./bcut_c 3,7 <data.bsv >/dev/null
real    0m0.812s
user    0m0.622s
sys     0m0.189s
```
