
#include "../header/FileSystem.h"
#include "../header/driver.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>



DirHandle* FS_init(fs* fs, const char* filename, int disk_dim, int block_dim) {
	
	int fd = open(filename, O_CREAT | O_EXCL | O_RDWR | O_SYNC, 0666);
	
	DirHandle* root_handle;
	int ret;

	if (fd == -1) {
		printf("DISKFILE ALREADY EXISTS\n");
		fd = open(filename, O_RDWR | O_SYNC, 0666);
		if (fd == -1) {
			printf("open error\n");
			exit(EXIT_FAILURE);
		}
		fs->fd_disk = fd;
		fs->first_block = (FirstDiskBlock*) mmap(0, DISK_DIM, PROT_READ | PROT_WRITE, MAP_SHARED, fs->fd_disk, 0);
		fs->fat = (int32_t*) (fs->first_block + fs->first_block->fat.first_fatBlock);
		root_handle = (DirHandle*) malloc(sizeof(DirHandle));
		root_handle->first_block = (FirstDirBlock*) malloc(sizeof(FirstDirBlock));
		ret = driver_readBlock(fs->first_block, 0, root_handle->first_block);
		if (ret == -1) {
			printf("init read root block\n");	
			exit(EXIT_FAILURE);
		}
		root_handle->num_parent_block = -1;
		root_handle->parentDir_num_entry = -1;
		root_handle->fs = fs;
	}
	
	else {
		printf("DISKFILE NOT ALREADY EXISTS\n");
		int ret = ftruncate(fd, disk_dim);
		if (ret == -1) {
			printf("ftruncate error\n");
			exit(EXIT_FAILURE);
		}
		fs->fd_disk = fd;
		fs->first_block = (FirstDiskBlock*) mmap(0, DISK_DIM, PROT_READ | PROT_WRITE, MAP_SHARED, fs->fd_disk, 0);
		disk_init(fs->first_block, disk_dim, block_dim);
		fs->fat = (int32_t*) (fs->first_block + fs->first_block->fat.first_fatBlock);
		root_handle = (DirHandle*) malloc(sizeof(DirHandle));
		root_handle->first_block = (FirstDirBlock*) malloc(sizeof(FirstDirBlock));
		root_handle->num_parent_block = -1;
		root_handle->parentDir_num_entry = -1;
		root_handle->fs = fs;
		const char* name = "rootdir";
		strcpy(root_handle->first_block->header.name, name);
		root_handle->first_block->header.flag = ROOTDIR;
		root_handle->first_block->fcb.dim = 0;
		root_handle->first_block->fcb.first_idx = 0;
		root_handle->first_block->fcb.parent_block = -1;
		root_handle->first_block->first_free_entry = 0;
		ret = driver_writeBlock(fs->first_block, 0, root_handle->first_block);
		if (ret == -1) {
			printf("init write root block\n");	
			exit(EXIT_FAILURE);
		}
	}
	
	List_init(root_handle->open_files);

	return root_handle;
}


FileHandle* FS_createFile(DirHandle* dir, const char* filename) {

	int32_t ret;
	FileHandle* file_handle = NULL;
	printf("avaible blocks: %d\n", dir->fs->first_block->fat.free_blocks);
	int32_t first_idx = dir->first_block->fcb.first_idx;
	fs* fs = dir->fs;
	int32_t last_idx = first_idx;
	int32_t first_file_block_idx;
	int32_t num_entry_file;

	while(fs->fat[last_idx] != -1) 
		last_idx = fs->fat[last_idx];

	if (last_idx == first_idx) {  // we are in the first dir block
		if (dir->first_block->first_free_entry == -1) {
			int32_t free_idx = driver_getfreeBlock(dir->fs->first_block);
			if (free_idx == -1) {
				printf("there is not free blocks\n");
				return NULL;
			}
			dir->fs->fat[last_idx] = free_idx;
			last_idx = free_idx;
			DirBlock* block = (DirBlock*) malloc(sizeof(DirBlock));
			memset(block, 0, sizeof(DirBlock));
			int32_t first_file_block_idx = driver_getfreeBlock(dir->fs->first_block);
			if (first_file_block_idx == -1) {
				printf("there is not free blocks\n");
				return NULL;
			}

		}
		else {
			first_file_block_idx = driver_getfreeBlock(dir->fs->first_block);
			if (first_file_block_idx == -1) {
				printf("there is not free blocks\n");
				return NULL;
			}
			int32_t aux = dir->first_block->first_free_entry;
			dir->first_block->first_free_entry = first_file_block_idx;
			int i;
			for (i = aux+1; i < sizeof(dir->first_block->first_blocks)/sizeof(int32_t); i++) {
				if (dir->first_block->first_blocks[i] == 0) {
					dir->first_block->first_free_entry = i;
					break;
				}
			}
			if (i == sizeof(dir->first_block->first_blocks)/sizeof(int32_t))
				dir->first_block->first_free_entry = -1;
		}

	}
	

	
	dir->first_block->fcb.dim++;
	printf("avaible blocks: %d\n", dir->fs->first_block->fat.free_blocks);

	return NULL;	
}
