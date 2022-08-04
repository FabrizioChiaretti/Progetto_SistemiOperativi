
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "../header/FileSystem.h"
#include "../header/driver.h"

#define DISK_PATHNAME "disk" 





int main (int argc, char** argv) {
	int ret;
	fs* fs_struct = (fs*) malloc(sizeof(fs));
	DirHandle* root_dir_handle = FS_init(fs_struct, DISK_PATHNAME, DISK_DIM, BLOCK_DIM);
	root_dir_handle->open_files = (ListHead*) malloc(sizeof(ListHead));
	List_init(root_dir_handle->open_files);
	DirHandle* current_dir = root_dir_handle;

	printf("avaible blocks: %d\n", current_dir->fs->first_block->fat.free_blocks);
	printf("root first_free_entry: %d\n",  current_dir->first_block->first_free_entry);
	printf("avaible blocks: %d\n", current_dir->fs->first_block->fat.free_blocks);
	printf("open files: %d\n", root_dir_handle->open_files->size);
	FileHandle* file;

	char* name;
	for (int i = 0; i < 10; i++) {
		if (i % 2) { 
			file = FS_createFile(current_dir, "pippo");
			if(file == NULL) {
				printf("creat file error\n");
				exit(EXIT_FAILURE);
			}
		}
		else {
			if (i == 0)
				name = "bruno";
			if (i == 2)
				name = "seneca";
			if (i == 4)
				name = "pitagora";
			if (i == 6)
				name = "cicerone";
			if (i == 8)
				name = "aristotele";
				
			ret = FS_mkdir(current_dir, name);
			if(ret == -1) {
				printf("mkdir error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
	printf("////////////////////\n");
	ret = FS_changeDir(current_dir, "aristotele");
	if (ret == -1) {
		printf("cd error\n");
		exit(EXIT_FAILURE);	
	}

	printf("name: %s\n", current_dir->first_block->header.name);
	printf("flag: %d\n", current_dir->first_block->header.flag);
	printf("parent block: %d\n", current_dir->first_block->fcb.parent_block);


	file = FS_createFile(current_dir, "pippo");
	if(file == NULL) {
		printf("creat file error\n");
		exit(EXIT_FAILURE);
	}

	printf("dim: %d\n", current_dir->first_block->fcb.dim);
	printf("open files size: %d\n", current_dir->open_files->size);
	printf("free entry: %d\n", current_dir->first_block->first_free_entry);
	printf("////////////////////\n");



	printf("avaible blocks: %d\n", current_dir->fs->first_block->fat.free_blocks);
	printf("root first_free_entry: %d\n",  root_dir_handle->first_block->first_free_entry);
	printf("avaible blocks: %d\n", root_dir_handle->fs->first_block->fat.free_blocks);
	//printf("open files: %d\n", root_dir_handle->open_files->size);

	ret = driver_writeBlock(current_dir->fs->first_block, 0, current_dir->first_block);
	if(ret == -1) {
		printf("write rootblock error\n");
		exit(EXIT_FAILURE);
	}
	ret = FS_flush(fs_struct->first_block);
	if(ret == -1) {
		printf("flush error\n");
		exit(EXIT_FAILURE);
	}
	ret = munmap(fs_struct->first_block, DISK_DIM);
	if(ret == -1) {
		printf("munmap first_block error\n");
		exit(EXIT_FAILURE);
	}
	ret = close(fs_struct->fd_disk);
	if(ret == -1) {
		printf("close disk error\n");
		exit(EXIT_FAILURE);
	}
	printf("bye\n");
	return EXIT_SUCCESS;
}
