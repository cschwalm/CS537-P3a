CC=gcc

all: libmem1 libmem2 libmem3

libmem1: 
	$(CC) -c -fpic mem.c -Wall -Werror
	$(CC) -shared -o libmem1.so mem.o 

libmem2: 
	$(CC) -c -fpic mem_wf2.c -Wall -Werror
	$(CC) -shared -o libmem2.so mem_wf2.o 

libmem3: 
	$(CC) -c -fpic mem.c -Wall -Werror
	$(CC) -shared -o libmem3.so mem.o 

test: libmem1
	$(CC) -lmem1 -L . -o test init2.c -Wall -Werror

clean:
	rm -rf *.o libmem1.so libmem2.so libmem3.so results.txt test
