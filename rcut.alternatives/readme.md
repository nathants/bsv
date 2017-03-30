performace varies wildy based on the width of the lines and the number of columns, but for a rough idea here are some timings with 1GB of data, 50 character width lines, 3 columns, on an i3.xlarge.

make sample data:

```
make
./gen-csv 10000000 3 > sample
```

c: 18s

```
time cat sample | ./rcut 3,2,1 > out.c
```

coreutils cut: 20s

```
time cat sample | cut -d, -f1,2,3 > out.coreutils
```

java: 65s

```
javac RCut.java
time cat ../sample | java RCut 3,2,1 > out.java
```

awk: 127s
```
time cat sample | awk -F, '{print $3 "," $2 "," $1}' > out.awk
```

python: 238s

```
time cat ../sample | python3 rcut.py 3,2,1 > out.python
```
