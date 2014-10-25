Name: Corbin Schwalm 
CSL-ID: schwalm
Name: Zach York
CSL-ID: zyork
(Leave the second name and ID blank if working alone)

Linux-Implementation Details: We implemented a version of malloc and free where we had a linked list for keeping track of where free nodes were in memory. Then when allocate was called, we would traverse this list
and return a block of memory to the user. If this was part of a larger free block, we would split this into two chunks one of which would remain free. In our free, we would add the node back into the free
list. If there were multiple blocks of free memory together, we would coallesce them.

XV6-Implementation Details:

