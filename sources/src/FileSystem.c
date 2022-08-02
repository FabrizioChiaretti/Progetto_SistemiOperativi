
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
		root_handle->first_block->fcb.last_idx = 0;
		root_handle->first_block->fcb.parent_block = -1;
		root_handle->first_block->first_free_entry = 0;
		ret = driver_writeBlock(fs->first_block, 0, root_handle->first_block);
		if (ret == -1) {
			printf("init write root block\n");	
			exit(EXIT_FAILURE);
		}
	}

	return root_handle;
}


FileHandle* FS_createFile(DirHandle* dir, const char* filename) {

	int32_t ret;
	FileHandle* file_handle = NULL;
	printf("avaible blocks: %d\n", dir->fs->first_block->fat.free_blocks);
	int32_t first_idx = dir->first_block->fcb.first_idx;
	fs* fs = dir->fs;
	int32_t last_idx = dir->first_block->fcb.last_idx;
	int32_t first_file_block_idx;

	if (last_idx == first_idx) {  // we are in the first dir block
		if (dir->first_block->first_free_entry == -1) {
			int32_t free_idx = driver_getfreeBlock(dir->fs->first_block);
			if (free_idx == -1) {
				printf("there is not free blocks\n");
				return NULL;
			}
			dir->fs->fat[last_idx] = free_idx;
			dir->first_block->fcb.last_idx = free_idx;
			DirBlock* block = (DirBlock*) malloc(sizeof(DirBlock));
			memset(block, 0, sizeof(DirBlock));
			first_file_block_idx = driver_getfreeBlock(dir->fs->first_block);
			if (first_file_block_idx == -1) {
				free(block);
				printf("there is not free blocks\n");
				return NULL;
			}
			
			block->first_blocks[0] = first_file_block_idx;
			block->first_free_entry = 1;
			ret = driver_writeBlock(dir->fs->first_block, free_idx, block);
			if (ret == -1) {
				free(block);
				printf("write block error\n");
				return NULL;
			}
			free(block);
			file_handle = (FileHandle*) malloc(sizeof(FileHandle));
			file_handle->num_parent_block = free_idx;
			file_handle->parentDir_num_entry = 0;
		}
		else {
			first_file_block_idx = driver_getfreeBlock(dir->fs->first_block);
			if (first_file_block_idx == -1) {
				printf("there is not free blocks\n");
				return NULL;
			}

			int32_t aux = dir->first_block->first_free_entry;
			dir->first_block->first_blocks[aux] = first_file_block_idx;
			int i;
			for (i = aux+1; i < sizeof(dir->first_block->first_blocks)/sizeof(int32_t); i++) {
				if (dir->first_block->first_blocks[i] == 0) {
					dir->first_block->first_free_entry = i;
					break;
				}
			}
			if (i == sizeof(dir->first_block->first_blocks)/sizeof(int32_t) -1)
				dir->first_block->first_free_entry = -1;
			
			file_handle = (FileHandle*) malloc(sizeof(FileHandle));
			file_handle->num_parent_block = dir->first_block->fcb.first_idx;
			file_handle->parentDir_num_entry = aux;
		}

	}

	else {
		DirBlock* block = malloc(sizeof(DirBlock));
		ret = driver_readBlock(dir->fs->first_block, last_idx, block);
		if (ret == -1) {
			free(block);
			printf("read block error\n");
			return NULL;
		}
		if (block->first_free_entry != -1) {
			first_file_block_idx = driver_getfreeBlock(dir->fs->first_block);
			if (first_file_block_idx == -1) {
				printf("there is not free blocks\n");
				free(block);
				return NULL;
			}
			int32_t aux = block->first_free_entry;
			block->first_blocks[aux] = first_file_block_idx;
			int i;
			for (i = aux+1; i < sizeof(block->first_blocks)/sizeof(int32_t); i++) {
				if (block->first_blocks[i] == 0) {
					block->first_free_entry = i;
					break;
				}
			}
			if (i == sizeof(block->first_blocks)/sizeof(int32_t) -1)
				block->first_free_entry = -1;
			
			ret = driver_writeBlock(dir->fs->first_block, last_idx, block);
			if (ret == -1) {
				printf("write block error\n");
				free(block);
				return NULL;
			}

			file_handle = (FileHandle*) malloc(sizeof(FileHandle));
			file_handle->num_parent_block = last_idx;
			file_handle->parentDir_num_entry = aux;
		}
		else {
			int32_t free_idx = driver_getfreeBlock(dir->fs->first_block);
			if (free_idx == -1) {
				printf("there is not free blocks\n");
				return NULL;
			}
			memset(block, 0, sizeof(DirBlock));
			dir->fs->fat[last_idx] = free_idx;
			dir->first_block->fcb.last_idx = free_idx;
			first_file_block_idx = driver_getfreeBlock(dir->fs->first_block);
			if (first_file_block_idx == -1) {
				printf("there is not free blocks\n");
				return NULL;
			}
			block->first_blocks[0] = first_file_block_idx;
			block->first_free_entry = 1;
			ret = driver_writeBlock(dir->fs->first_block, free_idx, block);
			if (ret == -1) {
				printf("write block error\n");
				return NULL;
			}
			file_handle = (FileHandle*) malloc(sizeof(FileHandle));
			file_handle->num_parent_block = dir->first_block->fcb.last_idx;
			file_handle->parentDir_num_entry = 0;
		}
		free(block);
	}

	dir->first_block->fcb.dim++;
	file_handle->pos = 0;
	file_handle->mode = WR;
	file_handle->first_block = (FirstFileBlock*) malloc(sizeof(FirstFileBlock));
	file_handle->first_block->occupied = 0;
	file_handle->first_block->header.flag = FL;
	strcpy(file_handle->first_block->header.name, filename);
	file_handle->first_block->fcb.dim = 0;
	file_handle->first_block->fcb.parent_block = file_handle->num_parent_block;
	file_handle->first_block->fcb.first_idx = dir->first_block->first_blocks[file_handle->parentDir_num_entry];
	file_handle->first_block->fcb.last_idx = file_handle->first_block->fcb.first_idx; 
	file_handle->current_block = NULL;

	//List_insert(dir->open_files, (ListItem*) file_handle);

	printf("avaible blocks: %d\n", dir->fs->first_block->fat.free_blocks);
	return file_handle;	
}



FileHandle* FS_openFile(DirHandle* dir, const char* filename, int mode) {
	
	// checking that the file isn't already open
	if (List_find(dir->open_files, filename)) {
		printf("file %s is already open", filename);
		return NULL;
	}

	int ret;
	int32_t num_entries = dir->first_block->fcb.dim;
	int32_t current_idx = dir->first_block->fcb.first_idx;
	int32_t last_idx = dir->first_block->fcb.last_idx;
	int32_t file_idx;
	int find = 0;
	FirstFileBlock* first_file_block = (FirstFileBlock*) malloc(sizeof(FirstFileBlock));
	DirBlock* dir_block = (DirBlock*) malloc(sizeof(DirBlock));
	int num_entry;
	int parent_block;

	int i;
	while (num_entries != 0 && !find) {
		if (current_idx == dir->first_block->fcb.first_idx) {
			for (i = 0; i < sizeof(dir->first_block->first_blocks)/sizeof(int32_t); i++) {  
				if (dir->first_block->first_blocks[i] != 0) {
					num_entries--;
					file_idx = dir->first_block->first_blocks[i];
					memset(first_file_block, 0, sizeof(FirstFileBlock));
					ret = driver_readBlock(dir->fs->first_block, file_idx, first_file_block);
					if (ret == -1) {
						printf("read block error\n");
						free(first_file_block);
						free(dir_block);
						return NULL;
					}
					if (strcmp(filename, first_file_block->header.name) == 0) {
						find = 1;
						num_entry = i;
						parent_block = dir->first_block->fcb.first_idx;
						break;
					}
				}
			}
		}

		else {
			memset(dir_block, 0, sizeof(DirBlock));
			ret = driver_readBlock(dir->fs->first_block, current_idx, dir_block);
				if (ret == -1) {
					printf("read block error\n");
					free(first_file_block);
					free(dir_block);
					return NULL;
				}
			for (i = 0; i < (BLOCK_DIM - sizeof(int32_t))/sizeof(int32_t); i++) {  
				if (dir_block->first_blocks[i] != 0) {
					num_entries--;
					file_idx = dir_block->first_blocks[i];
					memset(first_file_block, 0, sizeof(FirstFileBlock));
					ret = driver_readBlock(dir->fs->first_block, file_idx, first_file_block);
					if (ret == -1) {
						printf("read block error\n");
						free(first_file_block);
						free(dir_block);
						return NULL;
					}
					if (strcmp(filename, first_file_block->header.name) == 0) {
						find = 1;
						num_entry = i;
						parent_block = current_idx;
						break;
					}
				}
			}
		}

		if(!find && current_idx == last_idx) {
			printf("file %s does not exists\n", filename);
			free(dir_block);
			free(first_file_block);
			return NULL;
		}

		else if (!find && current_idx != last_idx) 
			current_idx = dir->fs->fat[current_idx];

		// else find !!!
	}

	FileHandle* handle = (FileHandle*) malloc(sizeof(FileHandle));
	handle->first_block = first_file_block;
	
	handle->current_block = NULL;
	handle->mode = mode;
	handle->pos = 0;
	handle->num_parent_block = parent_block;
	handle->parentDir_num_entry = num_entry;
	List_insert(dir->open_files, (ListItem*) handle);

	if (dir_block != NULL)
		free(dir_block);
	
	return handle;
	
}



int driver_flush(FirstDiskBlock* disk) { 
    int ret = msync(disk, disk->header.disk_dim, MS_SYNC);
    if (ret == -1)
        printf("flush error\n");
    return ret;
}