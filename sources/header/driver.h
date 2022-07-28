#pragma once

#include "FileSystem.h"


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
	int32_t* fat; // pointer to the first Fat block (next of the diskheader block)
} Fat;


typedef struct {
	DiskHeader header;
	int32_t RootDir_idx;
	Fat fat;
} FirstDiskBlock;



// initializes the DiskHeader 
void disk_init(DiskHeader* disk, const char* filename, int disk_dim);


// reads the block in position block_num and saves the contents in *block, 
// returns -1 if the block is free, 0 otherwise
int driver_readBlock(DiskHeader* disk, int32_t block_num, void* block);


// writes the contents of *source in the block in position block_num,
// returns -1 if is not possibile to write the block, 0 otherwise 
int driver_writeBlock(DiskHeader* disk, int32_t block_num, void* source);


// deletes the content of the block in position block_num,
// returns -1 if the is free yet, 0 otherwise
int driver_freeBlock(DiskHeader* disk, int32_t block_num);


// returns a index of a free block
int driver_getfreeBlock(DiskHeader* disk);


// write the contents of blocks_num on the disk, blocks is the number of the block to write
int driver_flush(DiskHeader* disk, int32_t blocks, int32_t blocks_num[blocks]);



