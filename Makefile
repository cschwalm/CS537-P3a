CC=gcc

all: mem libmem1 libmem2 libmem3

mem: mem.c
	$(CC) -c -fpic mem.c -Wall -Werror

libmem1: mem.o
	$(CC) -shared -o libmem1.so mem.o 


libmem2: mem.o
	$(CC) -shared -o libmem2.so mem.o 

libmem3: mem.o
	$(CC) -shared -o libmem3.so mem.o 

clean:
	rm -rf *.o libmem1.so libmem2.so libmem3.so results.txt
