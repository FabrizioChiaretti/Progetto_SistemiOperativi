#pragma once

#include <stdint.h>
#include "driver.h"
#include "../header/linked_list.h"


// integers used to indicate dimensions
#define DISK_DIM 8388608 // 8 MB
#define BLOCK_DIM 512


#define MAX_LENGHT_NAME 50


// flags for header
#define ROOTDIR 2
#define DIR 1
#define FL 0


// integers for open file mode
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
	int32_t last_idx;
	int32_t parent_block; // idx of the first parent directory block
} FileControlBlock;


typedef struct {
	FileBlockHeader header;
	FileControlBlock fcb;
	char block[BLOCK_DIM-sizeof(FileBlockHeader)-sizeof(FileControlBlock)-sizeof(int16_t)];
	int16_t occupied;
} FirstFileBlock;


typedef struct {
	FileBlockHeader header;
	FileControlBlock fcb;
	int32_t first_blocks[(BLOCK_DIM - sizeof(FileBlockHeader) - sizeof(FileControlBlock) - sizeof(int32_t) -sizeof(int32_t))/sizeof(int32_t)];
	int32_t first_free_entry;
	int32_t occupied;
} FirstDirBlock;


typedef struct {
	int16_t occupied;
	char block[BLOCK_DIM - sizeof(int16_t)];
} FileBlock;


typedef struct {
	int32_t first_free_entry;
	int32_t occupied;
	int32_t first_blocks[(BLOCK_DIM - sizeof(int32_t) - sizeof(int32_t))/sizeof(int32_t)];
} DirBlock;






///////////////////// USER STRUCTURES ////////////////


typedef struct {
	FirstDiskBlock* first_block;
	int fd_disk;
	int32_t* fat;
} fs;


typedef struct {
	ListItem item;
	int mode; // mode open file
	FirstFileBlock* first_block; 
	FileBlock* current_block;
	fs* fs;
	int32_t pos; // in bytes, from SEEK_SET
} FileHandle; 


typedef struct {
	FirstDirBlock* first_block;
	fs* fs;
	ListHead* open_files;
} DirHandle;





/////////// FILESYSTEM INTERFACES


DirHandle* FS_init(fs* fs, const char* filename, int disk_dim, int block_dim);


// creates an empty file in the directory dir
// returns null if the file existing or there is not free blocks
FileHandle* FS_createFile(DirHandle* dir, const char* filename);

// removes the file in the current directory
// returns -1 on failure 0 on success
// if a directory, it removes recursively all contained files
int FS_remove(DirHandle* dir, const char* filename);


// reads the name of all files in a directory 
// returns the number of the dir file
int FS_listing(DirHandle* dir,char** file_names);


// opens a file in the directory dir, the file should be exisiting
FileHandle* FS_openFile(DirHandle* dir, const char* filename, int mode);


// closes a file handle
int FS_close(DirHandle* dir, FileHandle* f);

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
int FS_mkdir(DirHandle* dir, const char* dirname);


// flushes written datas on mmaps
int FS_flush(FirstDiskBlock* disk);












