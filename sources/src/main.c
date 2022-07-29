

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../header/FileSystem.h"
#include "../header/driver.h"

#define DISK_PATHNAME "disk" 





int main (int argc, char** argv) {
	fs* fs_struct = (fs*) malloc(sizeof(fs));
	DirHandle* root_dir_handle = FS_init(fs_struct, DISK_PATHNAME, DISK_DIM, BLOCK_DIM);
	printf("ok\n");
	return EXIT_SUCCESS;
}
