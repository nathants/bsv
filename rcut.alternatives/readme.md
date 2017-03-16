c: baseline

```
cd ..
make
time cat /tmp/input | ./rcut , 3,1 > /tmp/out.c
```

java: x3+ slower

```
javac RCut.java
time cat /tmp/input | java RCut , 3,1 > /tmp/out.java
```

python: x10+ slower

```
time cat /tmp/input | python3 rcut.py , 3,1 > /tmp/out.pyhton
```
