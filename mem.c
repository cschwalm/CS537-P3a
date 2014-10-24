#include "stddef.h"
#include "stdlib.h"
#include "stdio.h"
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
const int MAGIC_CONST = 1234567;

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
		//fprintf(stderr, "updating global head pointer\n");
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
addFreeNode(node_t* prev, node_t* curr, node_t* next)
{
	if(prev == NULL)
	{
		head = curr;
		curr->prev = NULL;
		curr->next = next;
		next->prev = curr;
	}
	else if (next == NULL)
	{
		prev->next = curr;
		curr->prev = prev;
		curr->next = NULL;
	}
	else
	{
		prev->next = curr;
		curr->prev = prev;
		curr->next = next;
		next->prev = curr;
	}
}


void
coalesce()
{
	node_t* curr;
	curr = head;
	while(curr->next != NULL)
	{
		if (curr + curr->size + (int) sizeof(node_t) == curr->next)
		{
			curr->size += curr->next->size + (int) sizeof(node_t);
			curr->next = curr->next->next;
		}
		
		if (curr + curr->size + (int) sizeof(node_t) != curr->next)
			curr = curr->next;
	}
}

//
// Spits a region of memory to the size requested.
// This prevents waste. Simply splits this free
// node into two seperate free nodes and updates.
//
void
split(node_t** memBlock_i, node_t** newNode_i, int size)
{
	node_t* memBlock = *memBlock_i;
	node_t* newNode = *newNode_i; 

	//Checking to see if there is enough space for a new node (created when we split)
	//If not, return our previous block with a little extra space
	//
	//Currently if we remove this, more tests pass
	if (memBlock->size <= (int) (size + sizeof(node_t)))
	{
		fprintf(stderr, "not enough room for new node");
		*newNode_i = memBlock;
		return;
	}

	newNode = (node_t*) (memBlock + sizeof(node_t) + size);

	/* Set/Update Reference Pointers */
	newNode->next = memBlock->next;
	newNode->prev = memBlock;
	
	/* Next Node Might Be Null */
	if (memBlock->next != NULL)
	{
		memBlock->next->prev = newNode;
	}
	memBlock->next = newNode;

	/* Set/Update Size Members */
	newNode->size = memBlock->size - size - (int) sizeof(node_t); //Set the shard size
	memBlock->size = size; //Update the selection node size.
	
	//Switch these due to implementation in mem_alloc where we use newNode as our allocated node
	*memBlock_i = newNode;
	*newNode_i = memBlock;
	//fprintf(stderr, "%d\n", newNode->size);
}

int
Mem_Init(int sizeOfRegion)
{
	int memToInit = 0;
	void* ptr;

	if (sizeOfRegion<= 0 || head != NULL)
		return -1;
	
	int memSize = sizeOfRegion / getpagesize();

	if (sizeOfRegion % getpagesize() != 0)
		memSize++;

	memToInit = memSize * getpagesize();

	//open the /dev/zero device
	int fd = open("/dev/zero", O_RDWR);
	
	// sizeOfRegion (in bytes) needs to be evenly divisible by the page size
	ptr = mmap(NULL, memToInit, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (ptr == MAP_FAILED) 
	{
		return -1; 
	}

	head = (node_t*) ptr;
	
	head->size = memToInit - sizeof(node_t);
	//fprintf(stderr, "%d\n", head->size);
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

	//Requested Node Size is the total bytes we need + the amount of space we need
	//for the header. This is the total amount of memory we need. Because the free
	//memory is already storing information in the node, we need to subtract the size
	//of the free node
	requestedNodeSize = byteAlligned + (int) sizeof(header_t) - (int) sizeof(node_t);
	//fprintf(stderr, "%d\n", requestedNodeSize);
	if (requestedNodeSize > Mem_Available())
		return NULL;
	//fprintf(stderr, "size of header: %d\n", (int) sizeof(header_t));
	tmp = head;
	do 
	{
		if (tmp->size > requestedNodeSize)
		{
			//fprintf(stderr, "size Requested: %d tmp size: %d mem avail: %d\n", requestedNodeSize, tmp->size, Mem_Available());
			split(&tmp, &newNode, requestedNodeSize); //Sets new Node to the new split node.
			//fprintf(stderr, "split returned\n");
			break;
			//fprintf(stderr, "%d\n", newNode->size);
			//Mem_Dump();
		}
		else if (tmp->size == requestedNodeSize)
		{
			newNode = tmp;
			break;
		}
		tmp = tmp->next;
	} while(tmp != NULL);
	
	//fprintf(stderr, "before last null check");	
	if(newNode == NULL)
	{
		return NULL; //No free space found.
	}
	//fprintf(stderr, "Past last null check");	
	node_t* before = head;
	/* Remove the node from the free list and convert it. */
	removeFreeNode(newNode);

	allocNode = (header_t*) newNode;
	allocNode->size = byteAlligned;
	allocNode->magic = MAGIC_CONST;

	freeSpaceAddr = (void*) (allocNode + (int) sizeof(header_t));
	fprintf(stderr, "malloc: %p head: %p prevHead: %p\n", freeSpaceAddr, head, before);
	return freeSpaceAddr;
}

int
Mem_Free(void *ptr)
{
	header_t* header = NULL;
	int size = 0;
	node_t* next = NULL;
	node_t* prev = NULL;
	node_t* newNode = NULL;

	fprintf(stderr, "start of free\n");
	if (ptr == NULL)
		return -1;
	fprintf(stderr, "ptr not null");
	header = (header_t *) (ptr - sizeof(header_t));
	fprintf(stderr, "casts correctly");
	if (header->magic != MAGIC_CONST)
		return -1;
	
	size = header->size + (int) sizeof(header_t) - (int) sizeof(node_t);
	next = head;	
	newNode = (node_t* ) header;
	newNode->size = size;

	while (next < newNode)
	{
		if (next->next == NULL)
		{
			prev = next;	
			break;
		}
		next = next->next;
	}
	
	//Add to the end of the list
	if (prev != NULL)
	{
		addFreeNode(prev, newNode, NULL);
	}
	else
	{
		prev = next->prev;
		addFreeNode(prev, newNode, next);
	}

	coalesce();

	return 0;
}

//
// This function traverses the free list
// and counts the amount of space available.
//
int
Mem_Available()
{
	node_t *tmp = head;
	int freeSpace = 0;

	while (tmp != NULL)
	{
		freeSpace += tmp->size + 16;
		tmp = tmp->next;
	}

	return freeSpace;
}

//
// Used to debug the program.
// Prints out the nodes in the free list.
//
void
Mem_Dump()
{
	int nodeCnt = 0;
	node_t * node = head;
	const char *seperator = " => ";

	while (node != NULL)
	{
		if (nodeCnt > 0)
		{
			fprintf(stderr, "%s", seperator);
		}

		fprintf(stderr, "[Node #: %i, Size: %i]", nodeCnt, node->size);
		nodeCnt++;
		node = node->next;
	}

	fprintf(stderr, "\n");
}
