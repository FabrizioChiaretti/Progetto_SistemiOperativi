#pragma once
 

// integers used to indicate block type
#define DIR 0
#define FILE 1
#define ROOT_DIR 2
#define DISK_HEADER 3
#define FAT 4


// integers used to indicate dimensions
#define DISK_DIM 8388608 // 8 MB
#define BLOCK_DIM 512
#define NUM_BLOCKS DISK_DIM/BLOCK_DIM


// special idx for FAT
#define LAST_BLOCK -1
#define FREE_BLOCK -2



/////////////////////////////////// DISK STRUCTURES ///////////////////////////////


// first field of all disk blocks
typedef struct {
	int type_block;
} BlockHeader;



typedef struct {
	int dim; // blocks number for file, num_emtries for directory
	int first_idx;
} FileControlBlock;



typedef struct {
	int flag; // 1 for file, 0 for dir
	char name[11]; // max_lenght_name = 10
	FileControlBlock fcb;
} DirectoryEntry;



typedef struct {
	int dim;
	int free_num; // free blocks number
	int first_free; // first free block idx
	int* fat;
} Fat;



typedef struct {
	BlockHeader header;
	int disk_dim;
	int num_blocks;
	int block_dim;
	BlockHeader* root_dir;
	Fat* fat;
} DiskHeader;



typedef struct {
	BlockHeader header;
	char name[1];
	FileControlBlock fcb; 
} RootDir;



typedef struct {
	BlockHeader header;
	int used;
	char block[BLOCK_DIM-sizeof(BlockHeader)-sizeof(used)]; 
} FileBlock;



typedef struct {
	BlockHeader header;
	int num_entries;
	DirectoryEntry block[BLOCK_DIM-sizeof(BlockHeader)-sizeof(num_entries)]; 
} DirectoryBlock;



///////////////////// KERNEL STRUCTURES ////////////////


typedef struct {
	char* path;
	BlockHeader* directory; // directory where the file is stored
	BlockHeader* current_block;
	int pos;
	FileControlBlock fcb;
} FileHandle; 



typedef struct {
	char* path;
	BlockHeader* directory; // parent directory
	BlockHeader* current_block;
	int pos; // current entry number
	FileControlBlock fcb;
} DirectoryHandle;






