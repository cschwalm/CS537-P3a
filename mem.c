#include "stddef.h"
#include "stdlib.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "mem.h"
#include "unistd.h"
#include "sys/mman.h"

//Free Node
typedef struct __node_t {
	int size;
	struct __node_t *next; 
	struct __node_t *prev;
} node_t;

//Allocated Memory header
typedef struct __header_t {
	int size;
	int magic;
} header_t;

node_t* head = NULL;
int freeSpace = 0;

//
// Modifies the free list so that a new mem node can be added.
// This removes a node from the free list.
//
void
removeFreeNode(node_t* freeNode)
{
	
	node_t* prevNode, nextNode;

	prevNode = freeNode->prev;
	nextNode = freeNode->next;

	if (prevNode != NULL)
	{
		prevNode->next = nextNode;
	}
	else
	{
		/* Update the global head pointer. This is the start. */
		head = nextNode;
	}

	if (nextNode != NULL)
	{
		nextNode->prev = prevNode;
	}

	freeNode->next = NULL;
	freeNode->prev = NULL;
}

void
split(node_t* memBlock, node_t* newNode, int size)
{
	newNode = NULL;
	if (memBlock->size <= size + sizeof(node_t))
		return;

	newNode = (node_t*) (memBlock + sizeof(node_t) + size);
}

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
	head->prev = NULL;

	//close the device (don't worry, mapping should be unaffected)
	close(fd);
	return 0;
}

void
*Mem_Alloc(int size)
{
	node_t tmp;
	struct __header_t header;

  int byteAlligned = size / 8;
  if (size % 8 != 0)
    byteAlligned++;

  byteAlligned = byteAlligned * 8;


	if (byteAlligned + sizeof(header_t) > freeSpace)
		return NULL;

	tmp = head;
	do 
	{
		if(tmp->size >= (byteAlligned + sizeof(header_t)))
		{
			header = 
		}	
	} while(tmp->next != NULL);
		
	return NULL;
}

int
Mem_Free(void *ptr)
{

}

