### performance experiments and alternate implementations of cut and bcut

##### tldr;

```
>> export LC_ALL=C
>> time cut -d, -f3,7 <data.csv >/dev/null
real    0m5.784s
user    0m5.472s
sys     0m0.311s
```

```
>> time bcut 3,7 <data.bsv >/dev/null
real    0m1.010s
user    0m0.729s
sys     0m0.280s
```

```
>> time sort --parallel=1 -S50% -k1,1 <data.csv >/dev/null
real    0m22.406s
user    0m21.516s
sys     0m0.880s
```

```
>> time bsort <data.bsv >/dev/null
real    0m13.558s
user    0m12.266s
sys     0m1.139s
```

```
>> time sort -m -k1,1 -S50% csv.*.sorted >/dev/null
real    0m8.846s
user    0m6.692s
sys     0m2.149s
```

```
>> time bmerge $(cat filenames.txt | while read path; do echo $path.sorted; done) >/dev/null
real    0m1.361s
user    0m0.911s
sys     0m0.450s
```

##### alternate implementations and performance

[cut](https://github.com/nathants/bsv/blob/master/experiments/cut/readme.md) in c, rust, go, and pypy

[bcut](https://github.com/nathants/bsv/blob/master/experiments/bcut/readme.md) in c, rust, go and pypy

[sort and merge](https://github.com/nathants/bsv/blob/master/experiments/cut/readme.md) with bsv and coreutils

[linear scan](https://github.com/nathants/bsv/blob/master/experiments/cut/readme.md) with bsv and grep
