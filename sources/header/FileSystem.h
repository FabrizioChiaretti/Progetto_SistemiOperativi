#pragma once
 

// integers used to indicate entry type in the directory structure
#define DIR 0
#define FILE 1


// integers used to indicate dimensions
#define DISK_DIM 8388608 // 8 MB
#define BLOCK_DIM 512
#define NUM_BLOCKS DISK_DIM/BLOCK_DIM


// special idx for FAT
#define LAST_BLOCK -1
#define FREE_BLOCK -2



/////////////////////////////////// DISK STRUCTURES ///////////////////////////////


typedef struct {
	int dim; // in bytes for file, num_emtries for directory
	int first_idx;
} FileControlBlock;



typedef struct {
	int flag; // 1 for file, 0 for dir
	char name[52]; // max_lenght_name = 51
	FileControlBlock fcb;
} DirectoryEntry;



typedef struct {
	int dim; // number of the Fat elements
	int first_idx; // idx of the first available block
	int free_blocks;
	int* fat; // pointer to the first Fat block
} Fat;



typedef struct {
	int disk_dim;
	int block_dim;
	int total_blocks;
	int num_blocks; // number of the 'user' blocks 
	RootDir* root_dir;
	Fat fat;
} DiskHeader;



typedef struct {
	char name[56]; // name = 'root', dim = 56 for memory alignment
	FileControlBlock fcb; 
	DirectoryEntry block[7];
} RootDir;



typedef struct {
	char block[DIM_BLOCK];
} FileBlock;



typedef struct {
	DirectoryEntry block[DIM_BLOCK / sizeof(DirectoryEntry)];
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






