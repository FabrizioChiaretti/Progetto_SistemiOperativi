#pragma once

#include <stdint.h>

// integers used to indicate dimensions
#define DISK_DIM 8388608 // 8 MB
#define BLOCK_DIM 512
#define NUM_BLOCKS DISK_DIM/BLOCK_DIM


// special idx for FAT
#define LAST_BLOCK -1
#define FREE_BLOCK -2


// flags for header
#define ROOT 2
#define DIR 1
#define FL 0


#define MAX_LENGHT_NAME 50


/////////////////////////////////// DISK STRUCTURES ///////////////////////////////


typedef struct {
	uint8_t flag; // 1 for direrctory, 0 for file
	char name[MAX_LENGHT_NAME+1];
} FileBlockHeader; // header of the first file/directory block


typedef struct {
	int32_t dim; // in bytes for file, num_emtries for directory
	int32_t first_idx;
	int32_t parent_block; // idx of the first parent directory block
} FileControlBlock;


typedef struct {
	FileBlockHeader header;
	FileControlBlock fcb;
	char block[BLOCK_DIM-sizeof(FileBlockHeader)-sizeof(FileControlBlock)];
} FirstFileBlock;


typedef struct {
	FileBlockHeader header;
	FileControlBlock fcb;
	int32_t first_blocks[(BLOCK_DIM - sizeof(FileBlockHeader) - sizeof(FileControlBlock))/sizeof(int32_t)];
} FirstDirBlock;


typedef struct {
	FileBlockHeader header;
	FileControlBlock fcb;
	int32_t first_blocks[(BLOCK_DIM - sizeof(FileBlockHeader) - sizeof(FileControlBlock))/sizeof(int32_t)]; 
} RootDir;


typedef struct {
	char block[BLOCK_DIM];
} FileBlock;


typedef struct {
	int32_t first_blocks[BLOCK_DIM/sizeof(int32_t)];
} DirBlock;






///////////////////// KERNEL STRUCTURES ////////////////


typedef struct {
	char* path;
	DirBlock* parent_directory; 
	FileBlock* current_block;
	int pos; // in bytes, from SEEK_SET
	int modified; // 0 if the file is not modified, else 1
	FileControlBlock fcb;
} FileHandle; 



typedef struct {
	char* path;
	DirBlock* parent_directory; 
	DirBlock* current_block;
	int pos; // current entry number
	FileControlBlock fcb;
} DirHandle;






