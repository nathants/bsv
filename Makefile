.PHONY: all clean

all: rcut bucket partition

partition:
	gcc -O3 -Wall -finline-functions partition.c -o partition

rcut:
	gcc -O3 -Wall -finline-functions rcut.c -o rcut

bucket:
	gcc -O3 -Wall -finline-functions bucket.c -o bucket

clean:
	rm -f rcut bucket partition
