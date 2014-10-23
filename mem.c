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
	
	node_t *prevNode, *nextNode;

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

//
// Spits a region of memory to the size requested.
// This prevents waste. Simply splits this free
// node into two seperate free nodes and updates.
//
void
split(node_t* memBlock, node_t* newNode, int size)
{
	if (memBlock->size <= (int) (size + sizeof(node_t)))
		return;

	newNode = (node_t*) (memBlock + sizeof(node_t) + size);

	/* Set/Update Reference Pointers */
	newNode->next = memBlock->next;
	newNode->prev = memBlock;

	/* Next Node Might Be Null */
	if (memBlock->next->prev != NULL)
	{
		memBlock->next->prev = newNode;
	}
	memBlock->next = newNode;

	/* Set/Update Size Members */
	newNode->size = memBlock->size - size; //Set the shard size
	memBlock->size = size; //Update the selection node size.
}

int
Mem_Init(int sizeOfRegion)
{
	int memToInit = 0;
	void* ptr;

	if (sizeOfRegion<= 0 || head == NULL)
		return -1;
	
	int memSize = sizeOfRegion / getpagesize();

	if (sizeOfRegion % getpagesize() != 0)
		memSize++;

	memToInit = memSize * getpagesize();

	//open the /dev/zero device
	int fd = open("/dev/zero", O_RDWR);
	
	// sizeOfRegion (in bytes) needs to be evenly divisible by the page size
	ptr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (ptr == MAP_FAILED) 
	{
		return -1; 
	}

	head = (node_t*) ptr;
	
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
	node_t *tmp = NULL, *newNode = NULL;
	header_t *allocNode;

  int byteAlligned = size / 8;
	int requestedNodeSize;
	void *freeSpaceAddr;

  if (size % 8 != 0)
    byteAlligned++;

  byteAlligned = byteAlligned * 8;

	requestedNodeSize = byteAlligned + (int) sizeof(header_t);

	if (requestedNodeSize > freeSpace)
		return NULL;

	tmp = head;
	do 
	{
		if (tmp->size > requestedNodeSize)
		{
			split(tmp, newNode, requestedNodeSize); //Sets new Node to the new split node.
		}
		else if (tmp->size == requestedNodeSize)
		{
			newNode = tmp;
		}

	} while(tmp->next != NULL && newNode != NULL);
		
	if(newNode == NULL)
	{
		return NULL;
	}

	/* Remove the node from the free list and convert it. */
	removeFreeNode(newNode);

	allocNode = (header_t*) newNode;
	allocNode->size = requestedNodeSize;
	allocNode->magic = 1234567;

	freeSpaceAddr = allocNode + (int) sizeof(header_t);
	return freeSpaceAddr;
}

int
Mem_Free(void *ptr)
{
	return 0;
}
