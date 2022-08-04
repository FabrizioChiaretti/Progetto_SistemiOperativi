#pragma once

#include <stdint.h>




// special idx for FAT
#define LAST_BLOCK -1
#define FREE_BLOCK -2





typedef struct {
	int32_t disk_dim;
	int32_t block_dim;
	int32_t total_blocks;
	int32_t num_blocks; // number of the 'user' blocks 
	uint32_t fat_blocks;
} DiskHeader;


typedef struct {
	int32_t dim; // number of the Fat elements
	int32_t first_free; // idx of the first available block
	int32_t free_blocks;
	int32_t first_fatBlock; // pointer to the first Fat block (next of the diskheader block)
} Fat;


typedef struct {
	DiskHeader header;
	int32_t rootDir_idx;
	Fat fat;
	char padding[512-sizeof(DiskHeader)-sizeof(Fat)-sizeof(int32_t)];
} FirstDiskBlock;



// initializes the DiskHeader 
void disk_init(FirstDiskBlock* disk, int disk_dim, int block_num);


// reads the block in position block_num and saves the contents in *block, 
// returns -1 if the block is free, 0 otherwise
int driver_readBlock(FirstDiskBlock* disk, int32_t block_num, void* block);


// writes the contents of *source in the block in position block_num,
// returns -1 if is not possibile to write the block, 0 otherwise 
int driver_writeBlock(FirstDiskBlock* disk, int32_t block_num, void* source);


// deletes the content of the block in position block_num,
// returns -1 if the is free yet, 0 otherwise
int driver_freeBlock(FirstDiskBlock* disk, int32_t block_num);


// returns a index of a free block
int driver_getfreeBlock(FirstDiskBlock* disk);





