
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
	const char* filename = "pippo";
	FileHandle* file = FS_createFile(current_dir, filename);
	driver_writeBlock(fs_struct->first_block, 0, root_dir_handle->first_block);
	driver_writeBlock(fs_struct->first_block, file->first_block->fcb.first_idx, file->first_block);
	printf("////////////OPEN\n");
	FileHandle* filepippo = FS_openFile(current_dir, "pippo", WR);
	printf("parent_block: %d\n", filepippo->num_parent_block);
	printf("parent_numentry: %d\n", filepippo->parentDir_num_entry);
	printf("first_idx %d\n", filepippo->first_block->fcb.first_idx);
	printf("parent_block: %d\n", filepippo->first_block->fcb.parent_block);
	printf("name: %s\n", filepippo->first_block->header.name);
	driver_writeBlock(fs_struct->first_block, 0, root_dir_handle->first_block);
	ret = msync(fs_struct->first_block, DISK_DIM, MS_SYNC);
	if(ret == -1) {
		printf("msync error\n");
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
