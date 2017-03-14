.PHONY: clean

rcut: clean
	gcc -O3 -Wall -finline-functions rcut.c -o rcut

clean:
	rm -f rcut
