
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
	
	DirHandle* root_handle = (DirHandle*) malloc(sizeof(DirHandle));
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
		FirstDirBlock* root_dir = (FirstDirBlock*) malloc(sizeof(FirstDirBlock));
		ret = driver_readBlock(fs->first_block, 0, root_dir);
		if (ret == -1) {
			printf("init readrootblock\n");	
			exit(EXIT_FAILURE);
		}
		// inizializza i campi di root_handle
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
		FirstDirBlock* root_dir = (FirstDirBlock*) malloc(sizeof(FirstDirBlock));
		const char* name = "rootdir";
		strcpy(root_dir->header.name, name);
		root_dir->header.flag = ROOTDIR;
		root_dir->fcb.dim = 0;
		root_dir->fcb.first_idx = 0;
		root_dir->fcb.parent_block = -1;
		for (int i = 0; i < BLOCK_DIM/sizeof(int32_t); i++) {
			root_dir->first_blocks[i] = -1;
		}
		ret = driver_writeBlock(fs->first_block, 0, root_dir);
		if (ret == -1) {
			printf("init writerootblock\n");	
			exit(EXIT_FAILURE);
		}
		// inizializza i campi di root_handle
	}
	
	return root_handle;
}

