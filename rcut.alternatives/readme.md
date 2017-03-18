performace varies wildy based on the width of the lines and the number of columns, but for a rough idea here are some timings with 1GB of data, 50 character width lines, 3 columns, on an i3.xlarge.

c: 15s

```
cd ..
make
time cat /tmp/input | ./rcut , 3,2,1 > /tmp/out.c
```

java: 28s

```
javac RCut.java
time cat /tmp/input | java RCut , 3,2,1 > /tmp/out.java
```

awk: 52s
```
cat /tmp/input | awk -F, '{print $3 "," $2 "," $1}' > /tmp/out.awk
```

python: 112s

```
time cat /tmp/input | python3 rcut.py , 3,2,1 > /tmp/out.python
```
