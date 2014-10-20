
#include "mem.h"

typedef struct __node_t {
	int size;
	struct __node_t *next; 
} node_t

struct __node_t *head = NULL;
int freeSpace = 0;

int
Mem_Init(int sizeOfRegion)
{
	int memToInit = 0;
	if (sizeOfRegion<= 0 || head == NULL)
		return -1;
	
	int memSize = sizeOfRegion / getpagesize();

	if (sizeOfRegion % getpagesize() != 0)
		memSize++;

	memToInit = memSize * getpagesize();

	//open the /dev/zero device
	int fd = open("/dev/zero", O_RDWR);
	
	// sizeOfRegion (in bytes) needs to be evenly divisible by the page size
	head = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (ptr == MAP_FAILED) 
	{
		return -1; 
	}
	
	head->size = sizeOfRegion - sizeof(node_t);
	head->next = NULL;

	//close the device (don't worry, mapping should be unaffected)
	close(fd);
	return 0;
}

void
*Mem_Alloc(int size)
{
	return NULL;
}

int
Mem_Free(void *ptr)
{

}

