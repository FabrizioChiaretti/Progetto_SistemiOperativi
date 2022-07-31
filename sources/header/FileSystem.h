#pragma once

#include <stdint.h>
#include "driver.h"


// integers used to indicate dimensions
#define DISK_DIM 8388608 // 8 MB
#define BLOCK_DIM 512


#define MAX_LENGHT_NAME 50


#define RD 0
#define WR 1
#define RDWR 2




/////////////////////////////////// DISK STRUCTURES ///////////////////////////////


// header of the first file/directory block
typedef struct {
	uint8_t flag; // 1 for direrctory, 0 for file
	char name[MAX_LENGHT_NAME+1];
} FileBlockHeader; 


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
	char block[BLOCK_DIM];
} FileBlock;


typedef struct {
	int32_t first_blocks[BLOCK_DIM/sizeof(int32_t)];
} DirBlock;






///////////////////// USER STRUCTURES ////////////////


typedef struct {
	int mode; // mode open file
	FirstFileBlock* first_block; 
	FileBlock* current_block;
	int pos; // in bytes, from SEEK_SET
} FileHandle; 



typedef struct {
	FirstDirBlock* parent_directory; 
	FirstDirBlock* first_block;
	DirBlock* current_block;
	int pos; // current entry number
} DirHandle;


typedef struct {
	FirstDiskBlock* first_block;
	int fd_disk;
	int32_t* fat;
} fs;



/////////// FILESYSTEM INTERFACE


DirHandle* FS_init(fs* fs, const char* filename, int disk_dim, int block_dim);













