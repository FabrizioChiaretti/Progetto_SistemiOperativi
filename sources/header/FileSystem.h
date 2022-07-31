#pragma once

#include <stdint.h>
#include "driver.h"
#include "../header/linked_list.h"


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
	ListItem item;
	int mode; // mode open file
	FirstFileBlock* first_block; 
	FileBlock* current_block;
	int pos; // in bytes, from SEEK_SET
} FileHandle; 


typedef struct {
	FirstDirBlock* parent_directory; 
	FirstDirBlock* first_block;
	DirBlock* current_block;
} DirHandle;


typedef struct {
	FirstDiskBlock* first_block;
	int fd_disk;
	int32_t* fat;
} fs;



/////////// FILESYSTEM INTERFACES


DirHandle* FS_init(fs* fs, const char* filename, int disk_dim, int block_dim);


// creates an empty file in the directory dir
// returns null if the file existing or there is not free blocks
FileHandle* FS_createFile(DirHandle* dir, const char* filename);


// removes the file in the current directory
int FS_eraseFile(DirHandle* dir, char* filename);


// reads the name of all files in a directory 
// returns the number of the dir file
int FS_listDir(char** file_names, DirHandle* dir);


// opens a file in the directory dir, the file should be exisiting
FileHandle* FS_openFile(DirHandle* dir, const char* filename);


// closes a file handle
int FS_close(FileHandle* f);

// writes in the file at current position for size bytes stored in data
// returns the number of bytes written
int FS_write(FileHandle* file, void* data, int size);

// reads size of a file bytes from current positions and stores the datas in data
// returns the number of bytes read
int FS_read(FileHandle* file, void* data, int size);

// moves the current pointer to pos
// returns pos on success , -1 on error
int FS_seek(FileHandle* file, int pos);

// seeks for a directory in the current dir
// retuns 0 on success, -1 on error
// it does side effect on the current dir handle
 int FS_changeDir(DirHandle* dir, char* dirname);

// creates a new directory in the current dir
// 0 on success -1 on error
int FS_mkDir(DirHandle* dir, char* dirname);


// erase a directory (the root dir can't be erase)
// returns -1 on failure 0 on success
int FS_eraseDir(DirHandle* dir, char* filename);












