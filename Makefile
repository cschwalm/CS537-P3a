CC=gcc

all: libmem1.so libmem2.so libmem3.so 

libmem1.so: mem.c 
	$(CC) -c -fpic mem.c -Wall -Werror
	$(CC) -shared -o libmem1.so mem.o 

libmem2.so: mem_wf2.c
	$(CC) -c -fpic mem_wf2.c -Wall -Werror
	$(CC) -shared -o libmem2.so mem_wf2.o 

libmem3.so: mem.c
	$(CC) -c -fpic mem.c -Wall -Werror
	$(CC) -shared -o libmem3.so mem.o 

test1: nospace.c libmem3.so
	$(CC) -lmem3 -L . -o test1 nospace.c -Wall -Werror

test: init2.c libmem1.so
	$(CC) -lmem1 -L . -o test init2.c -Wall -Werror

clean:
	rm -rf *.o libmem1.so libmem2.so libmem3.so results.txt test
