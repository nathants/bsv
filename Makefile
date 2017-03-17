.PHONY: all clean

all: rcut bucket

rcut: clean
	gcc -O3 -Wall -finline-functions rcut.c -o rcut

bucket: clean
	gcc -O3 -Wall -finline-functions bucket.c -o bucket

clean:
	rm -f rcut bucket
