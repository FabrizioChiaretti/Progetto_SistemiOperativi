
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
		memset(root_handle->first_block, 0, sizeof(FirstDirBlock));
		root_handle->fs = fs;
		const char* name = "rootdir";
		strcpy(root_handle->first_block->header.name, name);
		root_handle->first_block->header.flag = ROOTDIR;
		root_handle->first_block->fcb.dim = 0;
		root_handle->first_block->fcb.first_idx = 0;
		root_handle->first_block->fcb.last_idx = 0;
		root_handle->first_block->fcb.parent_block = -1;
		root_handle->first_block->first_free_entry = 0;
		root_handle->first_block->occupied = 0;
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
	int32_t first_idx = dir->first_block->fcb.first_idx;
	int32_t last_idx = dir->first_block->fcb.last_idx;
	int32_t first_file_block_idx;
	int32_t parent_block;

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
			block->occupied = 1;
			ret = driver_writeBlock(dir->fs->first_block, free_idx, block);
			if (ret == -1) {
				free(block);
				printf("write block error\n");
				return NULL;
			}
			free(block);
			parent_block = dir->first_block->fcb.last_idx;
		}
		else {
			first_file_block_idx = driver_getfreeBlock(dir->fs->first_block);
			if (first_file_block_idx == -1) {
				printf("there is not free blocks\n");
				return NULL;
			}

			int32_t aux = dir->first_block->first_free_entry;
			dir->first_block->first_blocks[aux] = first_file_block_idx;
			dir->first_block->occupied++;
			int i;
			for (i = aux+1; i < sizeof(dir->first_block->first_blocks)/sizeof(int32_t); i++) {
				if (dir->first_block->first_blocks[i] == 0) {
					dir->first_block->first_free_entry = i;
					break;
				}
			}
			if (i == sizeof(dir->first_block->first_blocks)/sizeof(int32_t) -1)
				dir->first_block->first_free_entry = -1;
			parent_block = dir->first_block->fcb.first_idx;
		}

	}

	else {
		DirBlock* block = (DirBlock*) malloc(sizeof(DirBlock));
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
			block->occupied++;
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
			parent_block = dir->first_block->fcb.last_idx;
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
			block->occupied++;
			ret = driver_writeBlock(dir->fs->first_block, free_idx, block);
			if (ret == -1) {
				printf("write block error\n");
				return NULL;
			}
		}
		free(block);
		parent_block = dir->first_block->fcb.last_idx;
	}
	dir->first_block->fcb.dim++;
	FileHandle* file_handle = (FileHandle*) calloc(1, sizeof(FileHandle));
	file_handle->pos = 0;
	file_handle->mode = WR;
	file_handle->fs = dir->fs;
	file_handle->first_block = (FirstFileBlock*) malloc(sizeof(FirstFileBlock));
	memset(file_handle->first_block, 0, sizeof(FirstFileBlock));
	file_handle->first_block->occupied = 0;
	file_handle->first_block->header.flag = FL;
	strcpy(file_handle->first_block->header.name, filename);
	file_handle->first_block->fcb.dim = 0;
	file_handle->first_block->fcb.parent_block = parent_block;
	file_handle->first_block->fcb.first_idx = first_file_block_idx;
	file_handle->first_block->fcb.last_idx = file_handle->first_block->fcb.first_idx; 
	file_handle->current_block = 0;

	List_insert(dir->open_files, (ListItem*) file_handle);

	ret = driver_writeBlock(dir->fs->first_block, file_handle->first_block->fcb.first_idx, file_handle->first_block);
	if (ret == -1) {
		printf("write block error\n");
		return NULL;
	}

	return file_handle;	
}



FileHandle* FS_openFile(DirHandle* dir, const char* filename, int mode) {
	
	// checking that the file isn't already open
	if (List_find(dir->open_files, filename)) {
		printf("file %s is already open\n", filename);
		return NULL;
	}

	int ret;
	int32_t num_entries = dir->first_block->fcb.dim;
	int32_t current_idx = dir->first_block->fcb.first_idx;
	int32_t last_idx = dir->first_block->fcb.last_idx;
	int32_t file_idx;
	int find = 0;
	void* first_file_block = (void*) malloc(sizeof(FirstFileBlock));
	DirBlock* dir_block = (DirBlock*) malloc(sizeof(DirBlock));
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
					FileBlockHeader* header = (FileBlockHeader*) first_file_block;
					if (header->flag == FL) {
						FirstFileBlock* block = (FirstFileBlock*) first_file_block;
						if (strcmp(filename, block->header.name) == 0) {
							find = 1;
							parent_block = dir->first_block->fcb.first_idx;
							break;
						}
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
					FileBlockHeader* header = (FileBlockHeader*) first_file_block;
					if (header->flag == FL) {
						FirstFileBlock* block = (FirstFileBlock*) first_file_block;
						if (strcmp(filename, block->header.name) == 0) {
							find = 1;
							parent_block = dir->first_block->fcb.first_idx;
							break;
						}
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
	handle->first_block = (FirstFileBlock*) first_file_block;
	
	handle->current_block = 0;
	handle->mode = mode;
	handle->pos = 0;
	handle->fs = dir->fs;
	List_insert(dir->open_files, (ListItem*) handle);

	if (dir_block != NULL)
		free(dir_block);
	
	return handle;
	
}



int FS_close(DirHandle* dir, FileHandle* file) {
	
	FileHandle* detached = (FileHandle*) List_detach(dir->open_files, (ListItem*) file);

	free(detached->first_block);
	free(detached);

	return 0;
}



int FS_mkdir(DirHandle* dir, const char* dirname) {
	
	int32_t first_dir_idx;
	int32_t parent_idx;
	int32_t num_entries = dir->first_block->fcb.dim; 

	if (num_entries <= sizeof(dir->first_block->first_blocks) / sizeof(int32_t)) { // first dir block

		if (num_entries == sizeof(dir->first_block->first_blocks) / sizeof(int32_t)) {
			int32_t free_idx = driver_getfreeBlock(dir->fs->first_block);
			if (free_idx == -1) {
				printf("there is not free blocks\n");
				return -1;
			}
			dir->fs->fat[dir->first_block->fcb.first_idx] = free_idx;	
			dir->first_block->fcb.last_idx = free_idx;
			DirBlock* dir_block = (DirBlock*) malloc(sizeof(DirBlock));
			memset(dir_block, 0, sizeof(DirBlock));
			first_dir_idx = driver_getfreeBlock(dir->fs->first_block);
			if (first_dir_idx == -1) {
				printf("there is not free blocks\n");
				free(dir_block);
				return -1;
			}
			dir_block->first_blocks[0] = first_dir_idx; 
			dir_block->first_free_entry = 1;
			dir_block->occupied = 1;
			if (driver_writeBlock(dir->fs->first_block, free_idx, dir_block) == -1) {
				printf("write dir block error\n");
				free(dir_block);
				return -1;
			}
			parent_idx = free_idx;
			free(dir_block);
		}

		else {
			first_dir_idx = driver_getfreeBlock(dir->fs->first_block);
			if (first_dir_idx == -1) {
				printf("there is not free blocks\n");
				return -1;
			}
			parent_idx = dir->first_block->fcb.first_idx;
			dir->first_block->first_blocks[dir->first_block->first_free_entry] = first_dir_idx;
			dir->first_block->occupied++;
			int i = dir->first_block->first_free_entry +1;
			for (i; i < sizeof(dir->first_block->first_blocks) / sizeof(int32_t); i++) {
				if (dir->first_block->first_blocks[i] == 0) {
					dir->first_block->first_free_entry = i;
					break;
				}
			}
		}
	}

	else {
		int32_t last_idx = dir->first_block->fcb.last_idx;
		DirBlock* dir_block = (DirBlock*) malloc(sizeof(DirBlock));
		memset(dir_block, 0, sizeof(DirBlock));
		int ret = driver_readBlock(dir->fs->first_block, last_idx, dir_block);
		if (ret == -1) {
			printf("read block error\n");
			free(dir_block);
			return -1;
		}
		int32_t free_idx;
		if (dir_block->first_free_entry == -1) {
			free_idx = driver_getfreeBlock(dir->fs->first_block);
			if (free_idx == -1) {
				printf("there is not free blocks\n");
				free(dir_block);
				return -1;
			}
			dir->fs->fat[dir->first_block->fcb.last_idx] = free_idx;
			dir->first_block->fcb.last_idx = free_idx;
			first_dir_idx = driver_getfreeBlock(dir->fs->first_block);
			if (first_dir_idx == -1) {
				printf("there is not free blocks\n");
				free(dir_block);
				return -1;
			}
			dir_block->first_blocks[0] = first_dir_idx;
			dir_block->first_free_entry = 1;	
			dir_block->occupied = 1;
			parent_idx = free_idx;

			if (driver_writeBlock(dir->fs->first_block, free_idx, dir_block) == -1) {
				printf("write dir block error\n");
				free(dir_block);
				return -1;
			}
		}

		else {
			first_dir_idx = driver_getfreeBlock(dir->fs->first_block);
			if (first_dir_idx == -1) {
				printf("there is not free blocks\n");
				free(dir_block);
				return -1;
			}
			parent_idx = last_idx;
			dir_block->first_blocks[dir_block->first_free_entry] = first_dir_idx;
			dir_block->occupied++;
			int i = dir_block->first_free_entry +1;
			for (i; i < sizeof(dir_block->first_blocks) / sizeof(int32_t); i++) {
				if (dir_block->first_blocks[i] == 0) {
					dir_block->first_free_entry = i;
					break;
				}
			}			
		}

		free(dir_block);
	}

	dir->first_block->fcb.dim++;

	FirstDirBlock* dir_block = (FirstDirBlock*) malloc(sizeof(FirstDirBlock));
	memset(dir_block, 0, sizeof(DirBlock));
	dir_block->header.flag = DIR;
	strcpy(dir_block->header.name, dirname);
	dir_block->first_free_entry = 0;
	dir_block->fcb.dim = 0;
	dir_block->fcb.first_idx = dir_block->fcb.last_idx = first_dir_idx;
	dir_block->fcb.parent_block = parent_idx;

	if (driver_writeBlock(dir->fs->first_block, dir_block->fcb.first_idx, dir_block) == -1) {
		printf("write dir block error\n");
		free(dir_block);
		return -1;
	}

	free(dir_block);

	return 0;
}



int FS_listing(DirHandle* dir, char** file_names) {

	int result = 0;
	int32_t num_entries = dir->first_block->fcb.dim;
	for (int i = 0; i < num_entries; i++) 
		file_names[i] = (char*) malloc(sizeof(char)*(MAX_LENGHT_NAME+1+6)); // 1 for '\0' and 6 for '(file)' or '(dir)'
	FileBlockHeader* header;
	void* block = (void*) malloc(BLOCK_DIM);
	DirBlock* dir_block = (DirBlock*) malloc(sizeof(DirBlock));
	memset(dir_block, 0, BLOCK_DIM);
	memcpy(dir_block->first_blocks, dir->first_block->first_blocks, sizeof(dir->first_block->first_blocks));
	int32_t current_idx = dir->first_block->fcb.first_idx;
	int32_t occupied = dir->first_block->occupied;
	int32_t entries_block = sizeof(dir_block->first_blocks)/sizeof(int32_t);
	int ret;
	int j = 0;
	char* flag;

	while (1) {
		for (int i = 0; i < entries_block && occupied != 0; i++) {
			if (dir_block->first_blocks[i] != 0) {
				occupied--;
				ret = driver_readBlock(dir->fs->first_block, dir_block->first_blocks[i], block);
				if (ret == -1) {
					printf("read file_block error\n");
					free(dir_block);
					return 0;
				}
				header = (FileBlockHeader*) block;
				if (header->flag == FL) {
					flag = "(file)";
				}
				else { 
					flag = "(dir)";
				}
				strcpy(file_names[j], header->name);
				strcat(file_names[j], flag);
				j++;
				result++;
			}
		}

		if (current_idx == dir->first_block->fcb.last_idx) 
			break;
		
		else {
			current_idx = dir->fs->fat[current_idx];
			ret = driver_readBlock(dir->fs->first_block, current_idx, dir_block);
			if (ret == -1) {
				printf("read dir_block error\n");
				free(dir_block);
				return 0;
			}
			occupied = dir_block->occupied; 
		} 
	}

	free(dir_block);

	return result;
}



int FS_changeDir(DirHandle* dir, char* dirname) {

	if (dir->open_files->size != 0) {
		printf("close all files and try again\n");
		return -1;
	}

	int ret = driver_writeBlock(dir->fs->first_block, dir->first_block->fcb.first_idx, dir->first_block);
	if (ret == -1) {
		printf("write current first_dir block error\n");
		return -1;
	}

	int32_t num_entries = dir->first_block->fcb.dim;
	FileBlockHeader* header;
	void* block = (void*) malloc(BLOCK_DIM);
	DirBlock* dir_block = (DirBlock*) malloc(sizeof(DirBlock));
	memset(dir_block, 0, BLOCK_DIM);
	memcpy(dir_block->first_blocks, dir->first_block->first_blocks, sizeof(dir->first_block->first_blocks));
	int32_t current_idx = dir->first_block->fcb.first_idx;
	int32_t occupied = dir->first_block->occupied;
	int32_t entries_block = sizeof(dir_block->first_blocks)/sizeof(int32_t);
	int find = 0;
	int32_t first_dir_idx;

	while (1) {
		for (int i = 0; i < entries_block && occupied != 0 && !find; i++) {
			if (dir_block->first_blocks[i] != 0) {
				occupied--;
				ret = driver_readBlock(dir->fs->first_block, dir_block->first_blocks[i], block);
				if (ret == -1) {
					printf("read file_block error\n");
					free(dir_block);
					free(block);
					return 0;
				}
				header = (FileBlockHeader*) block;
				if (header->flag == DIR) {
					if (strcmp(header->name, dirname) == 0) {
						first_dir_idx = dir_block->first_blocks[i];
						find = 1;
						break;
					}
				}
			}
		}

		if (find || !find && current_idx == dir->first_block->fcb.last_idx)
			break;
		
		else {
			current_idx = dir->fs->fat[current_idx];
			ret = driver_readBlock(dir->fs->first_block, current_idx, dir_block);
			if (ret == -1) {
				printf("read dir_block error\n");
				free(dir_block);
				free(block);
				return 0;
			}
			occupied = dir_block->occupied; 
		} 
	}

	if (!find) {
		printf("dir %s does not exists\n", dirname);
		free(dir_block);
		free(block);
		return -1;
	}

	memset(dir->first_block, 0, sizeof(FirstDirBlock));	
	dir->first_block = (FirstDirBlock*) block;
	free(dir_block);

	return 0;
}



int FS_write(FileHandle* file, void* data, int size) {

	int ret;
	int written = 0;
	int pos = file->pos;
	int to_write;
	char* data_pos = (char*) data;
	int free_bytes;
	int dim = file->first_block->fcb.dim;
	FileBlock* file_block = (FileBlock*) malloc(sizeof(FileBlock));
	int32_t offset = sizeof(file->first_block->block);
	int32_t block_offset = 0;
	int32_t current_idx = file->first_block->fcb.first_idx;
	int32_t size_block = size;
	for (int i = 0; i < file->current_block; i++) {
		current_idx = file->fs->fat[current_idx];
	}

	while (written != size) {

		memset(file_block, 0, sizeof(FileBlock));
		if (pos < sizeof(file->first_block->block) -1) {
			free_bytes = sizeof(file->first_block->block) - pos -1;
			if (size_block - free_bytes > 0) 
				to_write = free_bytes;
			else
				to_write = size_block;

			memcpy(file->first_block->block+pos, data_pos, to_write);
			if (pos == dim)
				dim += to_write;
			else if (pos + to_write > dim) 
				dim += pos + to_write - dim;

			ret = driver_writeBlock(file->fs->first_block, file->first_block->fcb.first_idx, file->first_block);
			if (ret == -1) {
				printf("write block error\n");
				free(file_block);
				return written;
			}
			written += to_write;
			pos += to_write;
			data_pos += to_write;
			size_block -= to_write;
		}
		else if (pos == sizeof(file->first_block->block)-1) {
			int flag = 0;
			file->current_block++;
			if (file->first_block->fcb.first_idx != file->first_block->fcb.last_idx) {
				current_idx = file->fs->fat[current_idx];
				ret = driver_readBlock(file->fs->first_block, current_idx, file_block);
				if (ret == -1) {
					printf("read block error\n");
					free(file_block);
					return written;
				}
			}
			else {
				flag = 1;
				current_idx = driver_getfreeBlock(file->fs->first_block);
				if (ret == -1) {
					printf("there are not free blocks\n");
					free(file_block);
					return written;
				}
				file->fs->fat[file->first_block->fcb.first_idx] = current_idx;
				file->first_block->fcb.last_idx = current_idx;
			}
			
			free_bytes = sizeof(file_block->block)-1;
			if (size_block - free_bytes > 0) 
				to_write = free_bytes;
			else 
				to_write = size_block;

			if (flag) {
				dim += to_write + 1;
			}
			else if (pos + to_write > dim) 	
				dim += pos + to_write - dim;
				
	
			memcpy(file_block->block, data_pos, to_write);
			ret = driver_writeBlock(file->fs->first_block, current_idx, file_block);
			if (ret == -1) {
				printf("write block error\n");
				free(file_block);
				return written;
			}
			written += to_write;
			pos += to_write +1;
			data_pos += to_write;		
			size_block -= to_write;	
		}
		else {
			if ((pos - sizeof(file->first_block->block) +1) % (sizeof(file_block->block)) == 0) {
				file->current_block++;
				if (file->fs->fat[current_idx] == LAST_BLOCK) {
					ret = driver_getfreeBlock(file->fs->first_block);
					if (ret == -1) {
						printf("there are not free blocks\n");
						free(file_block);
						return written;
					}
					file->fs->fat[current_idx] = ret;
					file->first_block->fcb.last_idx = ret;
					current_idx = ret;
					free_bytes = sizeof(file_block->block)-1;

					if (size_block - free_bytes > 0) 
						to_write = free_bytes;
					else 
						to_write = size_block;
					
					dim += to_write + 1;

				}
				else {
					current_idx = file->fs->fat[current_idx];
					ret = driver_readBlock(file->fs->first_block, current_idx, file_block);
					if (ret == -1) {
						printf("read block error\n");
						free(file_block);
						return written;
					}

					free_bytes = sizeof(file_block->block)-1;
					if (size_block - free_bytes > 0) 
						to_write = free_bytes;
					else 
						to_write = size_block;
					
					if (pos + to_write > dim) 	
						dim += pos + to_write - dim;
					
				}

				memcpy(file_block->block, data_pos, to_write);
				ret = driver_writeBlock(file->fs->first_block, current_idx, file_block);
				if (ret == -1) {
					printf("write block error\n");
					free(file_block);
					return written;
				}
				written += to_write;
				pos += to_write +1;
				data_pos += to_write;	
				size_block -= to_write;	
			}
			else {
				offset += (file->current_block)*sizeof(file_block->block);
				ret = driver_readBlock(file->fs->first_block, current_idx, file_block);
				if (ret == -1) {
					printf("read block error\n");
					free(file_block);
					return written;
				}
				block_offset = offset - pos; 
				free_bytes = block_offset-1;

				if (size_block - free_bytes > 0) 
					to_write = free_bytes;
				else 
					to_write = size_block;

				dim += to_write;
				
				printf("to write: %d\n", to_write);

				int32_t pos_block = sizeof(file_block->block) - block_offset; 
				printf("block_pos: %d\n", pos_block);
				memcpy(file_block->block + pos_block, data_pos, to_write);

				ret = driver_writeBlock(file->fs->first_block, current_idx, file_block);
				if (ret == -1) {
					printf("write block error\n");
					free(file_block);
					return written;
				}
				written += to_write;
				pos += to_write;
				data_pos += to_write;	
				size_block -= to_write;			
			}
		}
		printf("pos: %d\n", pos);
		printf("written: %d\n", written);
	}

	file->pos = pos;
	file->first_block->fcb.dim = dim;
	free(file_block);
	printf("esco, written: %d, pos: %d, dim: %d\n", written, pos, dim);

	return written;
}



int FS_read(FileHandle* file, void* data, int size) {

	if (file->first_block->fcb.dim < file->pos + size) {
		printf("invalid size read\n");
		return -1;
	}

	int ret;
	int result = 0;
	char* block = (char*) data;
	int pos = file->pos;
	int size_block = size;
	int32_t to_read;
	int32_t offset = sizeof(file->first_block->block);
	int32_t block_offset = 0;
	int32_t current_idx = file->first_block->fcb.first_idx;
	for (int i = 0; i < file->current_block; i++) {
		current_idx = file->fs->fat[current_idx];
	}

	//FileBlock* aux = &file_block;
 	FileBlock* aux = (FileBlock*)malloc(sizeof(FileBlock));
	while (size_block != 0) {


		if (pos < sizeof(file->first_block->block)-1) {

			if (size_block - strlen(file->first_block->block) +pos >= 0) 
				to_read = strlen(file->first_block->block) -pos;
			else 
				to_read = size_block;

			strncat(block, file->first_block->block + pos, to_read);
			pos += to_read;
			size_block -= to_read;
			result += to_read;
		}
		else if (pos == sizeof(file->first_block->block)-1 || 
				((pos - sizeof(file->first_block->block) +1) % (sizeof(aux->block)) == 0)) {

			current_idx = file->fs->fat[current_idx];
			pos++;
			file->current_block++;
			ret = driver_readBlock(file->fs->first_block, current_idx, aux);
			if (ret == -1) {
				printf("read block error\n");
				free(aux);
				return result;
			}
	
			if (size_block >  strlen(aux->block)) {
				to_read = strlen(aux->block);
			}
			else {
				to_read = size_block;
			}
			
			strncat(block, aux->block, to_read);
			pos += to_read;
			size_block -= to_read;
			result += to_read;
		}
		else {
			offset += (file->current_block)*sizeof(aux->block);
			printf("offset: %d\n", offset);
			ret = driver_readBlock(file->fs->first_block, current_idx, aux);
			if (ret == -1) {
				printf("read block error\n");
				free(aux);
				return result;
			}
			int avaibles = offset - pos -1;
			int block_offset = sizeof(aux->block) - avaibles-1;

			printf("avaibles: %d, block_offset: %d\n", avaibles, block_offset);

			if (size_block > strlen(aux->block)) {
				to_read = strlen(aux->block);
			}
			else 
				to_read = size_block;

			printf("to read: %d\n", to_read);
			strncat(block, aux->block + block_offset, to_read);
			pos += to_read;
			size_block -= to_read;
			result += to_read;
		}
		printf("pos: %d, result: %d\n", pos, result);
		printf("size: %d\n", size_block);
	}

	file->pos = pos;

	return result;
}



int FS_seek(FileHandle* file, int pos) {

	if (pos > file->first_block->fcb.dim) {
		printf("invalide seek\n");
		return -1;
	}

	file->pos = pos;
	file->current_block = 0;
	int32_t offset = sizeof(file->first_block->block);
	int32_t block_dim = BLOCK_DIM - sizeof(int16_t);

	while (1) {

		if (file->pos <= offset-1)
			break;
		
		offset += block_dim;
		file->current_block++;
	}

	printf("pos: %d, current_block: %d\n", file->pos, file->current_block);
	return file->pos;
}



int FS_eraseFile(DirHandle* dir, const char* filename) {
	
	if (List_find(dir->open_files, filename)) { 
		printf("close the file and try again to remove\n");
		return -1;
	}

	// thanks to change dir 
	int ret;
	int32_t num_entries = dir->first_block->fcb.dim;
	FileBlockHeader* header;
	void* block = (void*) malloc(BLOCK_DIM);
	DirBlock* dir_block = (DirBlock*) malloc(sizeof(DirBlock));
	memset(dir_block, 0, BLOCK_DIM);
	memcpy(dir_block->first_blocks, dir->first_block->first_blocks, sizeof(dir->first_block->first_blocks));
	dir_block->occupied = dir->first_block->occupied;
	int32_t current_idx = dir->first_block->fcb.first_idx;
	int32_t occupied = dir->first_block->occupied;
	int32_t entries_block = sizeof(dir_block->first_blocks)/sizeof(int32_t);
	int find = 0;
	int32_t first_file_idx;
	int i;
	int32_t to_free;

	while (1) {
		for (i = 0; i < entries_block && occupied != 0 && !find; i++) {
			if (dir_block->first_blocks[i] != 0) {
				occupied--;
				ret = driver_readBlock(dir->fs->first_block, dir_block->first_blocks[i], block);
				if (ret == -1) {
					printf("read file_block error\n");
					free(dir_block);
					free(block);
					return 0;
				}
				header = (FileBlockHeader*) block;
				if (header->flag == FL) {
					if (strcmp(header->name, filename) == 0) {
						first_file_idx = dir_block->first_blocks[i];
						find = 1;
						break;
					}
				}
			}
		}

		if (find || !find && current_idx == dir->first_block->fcb.last_idx)
			break;
		
		else {
			current_idx = dir->fs->fat[current_idx];
			ret = driver_readBlock(dir->fs->first_block, current_idx, dir_block);
			if (ret == -1) {
				printf("read dir_block error\n");
				free(dir_block);
				free(block);
				return 0;
			}
			occupied = dir_block->occupied; 
		} 
	}

	if (!find) {
		printf("file '%s' does not exist\n", filename);
		return -1;
	}

	dir->first_block->fcb.dim--;
	dir_block->first_blocks[i] = 0;

	dir_block->occupied--;
	if (i < dir_block->first_free_entry)
		dir_block->first_free_entry = i;

	if (current_idx != dir->first_block->fcb.first_idx) {
		to_free = current_idx;
		if (dir_block->occupied == 0) {
			if (current_idx == dir->first_block->fcb.last_idx) {
				current_idx = dir->first_block->fcb.first_idx;
				while (dir->fs->fat[current_idx] != dir->first_block->fcb.last_idx) {
					current_idx = dir->fs->fat[current_idx];
				}
				dir->fs->fat[dir->first_block->fcb.last_idx] = FREE_BLOCK;
				dir->fs->fat[current_idx] = LAST_BLOCK;
				dir->first_block->fcb.last_idx = current_idx;
			}
			else {
				int32_t aux = dir->first_block->fcb.first_idx;
				while (dir->fs->fat[aux] != current_idx) {
					aux = dir->fs->fat[aux];
				}
				dir->fs->fat[aux] = dir->fs->fat[current_idx];
				dir->fs->fat[current_idx] = FREE_BLOCK;
			}
			ret = driver_freeBlock(dir->fs->first_block, to_free);
			if (ret == -1) {
				printf("free_block error\n");
				return -1;
			}
		}	
		else {
			ret = driver_writeBlock(dir->fs->first_block, current_idx, dir_block);
			if (ret == -1) {
				printf("write block error\n");
				free(dir_block);
				return -1;
			}
		}
	}
	else {
		dir->first_block->occupied--;
		dir->first_block->first_free_entry = dir_block->first_free_entry;
		dir->first_block->first_blocks[i] = 0;
	}

	FirstFileBlock* file_block = (FirstFileBlock*) malloc(sizeof(FirstFileBlock));
	ret = driver_readBlock(dir->fs->first_block, first_file_idx, file_block);
	if (ret == -1) {
		printf("read block error\n");
		return -1;
	}
	int32_t idx = file_block->fcb.first_idx;
	while (idx != LAST_BLOCK) {
		to_free = idx;
		printf("to free: %d\n", to_free);
		idx = dir->fs->fat[idx];
		ret = driver_freeBlock(dir->fs->first_block, to_free);
		if (ret == -1) {
			printf("free_block error\n");
			return -1;
		}
		dir->fs->fat[to_free] = FREE_BLOCK;
	}

	free(dir_block);
	free(block);
	free(file_block);

	return 0;
}



int FS_eraseDir_aux(DirHandle* dir) {
	
	int ret;
	int32_t entries = dir->first_block->fcb.dim;
	FileBlockHeader* header = NULL;
	DirBlock* dir_block = (DirBlock*) malloc(sizeof(DirBlock));
	void* block = (void*) malloc(sizeof(FileBlock));
	memcpy(dir_block->first_blocks, dir->first_block->first_blocks, sizeof(dir->first_block->first_blocks));
	dir_block->occupied = dir->first_block->occupied;
	int32_t current_idx = dir->first_block->fcb.first_idx;
	int32_t occupied = dir->first_block->occupied;
	int32_t entries_block = sizeof(dir_block->first_blocks)/sizeof(int32_t);
	DirHandle* dir_handle = (DirHandle*) malloc(sizeof(DirHandle));
	dir_handle->fs = dir->fs;
	int32_t to_free;

	printf("entries: %d, dir: %s\n", entries, dir->first_block->header.name);

	while (entries != 0) {

		for (int i = 0; i < entries_block && occupied != 0; i++) {
			if (dir_block->first_blocks[i] != 0) {
				occupied--;
				entries--;
				ret = driver_readBlock(dir->fs->first_block, dir_block->first_blocks[i], block);
				if (ret == -1) {
					printf("read file_block error\n");
					free(dir_block);
					free(block);
					return 0;
				}
				header = (FileBlockHeader*) block;
				if (header->flag == DIR) {
					block = (FirstDirBlock*) header;
					dir_handle->first_block = block;
					ret = FS_eraseDir_aux(dir_handle);
					if (ret == -1) {
						free(dir_block);
						free(block);
						return -1;
					}
				}
				else { // header->flag == FL
					ret = FS_eraseFile(dir, header->name);
					if (ret == -1) {
						free(dir_block);
						free(block);
						return -1;
					}
				}
			}
		}
		
		to_free = current_idx;
		current_idx = dir->fs->fat[current_idx];
		ret = driver_freeBlock(dir->fs->first_block, to_free);
		if (ret == -1) {
			printf("free_block error\n");
			free(dir_block);
			free(block);
			free(dir_handle);
			return -1;
		}

		dir->fs->fat[to_free] = FREE_BLOCK;
		if (current_idx == -1)
			break;

		ret = driver_readBlock(dir->fs->first_block, current_idx, dir_block);
		if (ret == -1) {
			printf("read dir_block error\n");
			free(dir_block);
			free(block);
			return -1;
		}
		occupied = dir_block->occupied; 

	}
	
	free(dir_block);
	free(block);
	free(dir_handle);

	printf("num_entries: %d\n", entries);

	return 0;
}



int FS_eraseDir(DirHandle* dir, const char* dirname) {

	int ret;
	int32_t num_entries = dir->first_block->fcb.dim;
	FileBlockHeader* header;
	void* block = (void*) malloc(BLOCK_DIM);
	DirBlock* dir_block = (DirBlock*) malloc(sizeof(DirBlock));
	memset(dir_block, 0, BLOCK_DIM);
	memcpy(dir_block->first_blocks, dir->first_block->first_blocks, sizeof(dir->first_block->first_blocks));
	dir_block->occupied = dir->first_block->occupied;
	int32_t current_idx = dir->first_block->fcb.first_idx;
	int32_t occupied = dir->first_block->occupied;
	int32_t entries_block = sizeof(dir_block->first_blocks)/sizeof(int32_t);
	int find = 0;
	int32_t first_dir_idx;
	int i;

	while (1) {
		for (i = 0; i < entries_block && occupied != 0 && !find; i++) {
			if (dir_block->first_blocks[i] != 0) {
				occupied--;
				ret = driver_readBlock(dir->fs->first_block, dir_block->first_blocks[i], block);
				if (ret == -1) {
					printf("read file_block error\n");
					free(dir_block);
					free(block);
					return 0;
				}
				header = (FileBlockHeader*) block;
				if (header->flag == DIR) {
					if (strcmp(header->name, dirname) == 0) {
						first_dir_idx = dir_block->first_blocks[i];
						find = 1;
						break;
					}
				}
			}
		}

		printf("find: %d, current_idx: %d\n", find,current_idx);

		if (find || !find && current_idx == dir->first_block->fcb.last_idx)
			break;
		
		else {
			current_idx = dir->fs->fat[current_idx];
			ret = driver_readBlock(dir->fs->first_block, current_idx, dir_block);
			if (ret == -1) {
				printf("read dir_block error\n");
				free(dir_block);
				free(block);
				return 0;
			}
			occupied = dir_block->occupied; 
		} 
	}

	if (!find) {
		printf("dir '%s' does not exist\n", dirname);
		free(dir_block);
		free(block);
		return -1;
	}

	dir->first_block->fcb.dim--;

	dir_block->occupied--;
	if (i < dir_block->first_free_entry)
		dir_block->first_free_entry = i;

	if (current_idx != dir->first_block->fcb.first_idx) {
		if (dir_block->occupied == 0) {
			int32_t to_free = current_idx; 
			if (current_idx == dir->first_block->fcb.last_idx) {
				current_idx = dir->first_block->fcb.first_idx;
				while (dir->fs->fat[current_idx] != dir->first_block->fcb.last_idx) {
					current_idx = dir->fs->fat[current_idx];
				}
				dir->fs->fat[dir->first_block->fcb.last_idx] = FREE_BLOCK;
				dir->fs->fat[current_idx] = LAST_BLOCK;
				dir->first_block->fcb.last_idx = current_idx;
			}
			else {
				int32_t aux = dir->first_block->fcb.first_idx;
				while (dir->fs->fat[aux] != current_idx) {
					aux = dir->fs->fat[aux];
				}
				dir->fs->fat[aux] = dir->fs->fat[current_idx];
				dir->fs->fat[current_idx] = FREE_BLOCK;
			}
			ret = driver_freeBlock(dir->fs->first_block, to_free);
			if (ret == -1) {
				printf("free_block error\n");
				free(dir_block);
				free(block);
				return -1;
			}
		}
		else {
			ret = driver_writeBlock(dir->fs->first_block, current_idx, dir_block);
			if (ret == -1) {
				printf("write block error\n");
				free(dir_block);
				free(block);
				return -1;
			}
		}
	}
	else {
		dir->first_block->occupied = dir_block->occupied;
		dir->first_block->first_free_entry = dir_block->first_free_entry;
		dir->first_block->first_blocks[i] = 0;
	}

	FirstDirBlock* first_dir_block = (FirstDirBlock*) malloc(sizeof(FirstDirBlock));
	ret = driver_readBlock(dir->fs->first_block, first_dir_idx, first_dir_block);
	if (ret == -1) {
		printf("read block error\n");
		free(dir_block);
		free(block);
		free(first_dir_block);
		return -1;
	}

	DirHandle* handle_aux = (DirHandle*) malloc(sizeof(DirHandle));
	handle_aux->fs = dir->fs;
	handle_aux->first_block = first_dir_block;

	ret = FS_eraseDir_aux(handle_aux);
	
	free(handle_aux);
	free(dir_block);
	free(block);
	free(first_dir_block);

	return ret;
}



int FS_flush(FirstDiskBlock* disk) { 
    int ret = msync(disk, disk->header.disk_dim, MS_SYNC);
    if (ret == -1)
        printf("flush error\n");
    return ret;
}