
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
