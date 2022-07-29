
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
	
	int fd = open(filename, O_CREAT | O_EXCL | O_RDWR, 0666);
	
	if (fd == -1) {
		printf("DISKFILE ALREADY EXISTS\n");
		fd = open(filename, O_RDWR, 0666);
		if (fd == -1) {
			printf("open error\n");
			exit(EXIT_FAILURE);
		}
		fs->fd_disk = fd;
		fs->first_block = (FirstDiskBlock*) mmap(0, DISK_DIM, PROT_READ | PROT_WRITE, MAP_SHARED, fs->fd_disk, 0);
		uint8_t* aux = (uint8_t*) fs->first_block;
		int block_Dim = fs->first_block->header.block_dim;
		int root_idx = fs->first_block->rootDir_idx;
		aux += block_Dim*root_idx;
		fs->root_dir = (RootDir*) aux;
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
		uint8_t* aux = (uint8_t*) fs->first_block;
		int block_Dim = (fs->first_block->header).block_dim;
		int root_idx = fs->first_block->rootDir_idx;
		aux += block_Dim*root_idx;
		fs->root_dir = (RootDir*) aux;
		char *root_name = fs->root_dir->header.name;
		const char* name = "rootdir";
		strcpy(fs->root_dir->header.name, name); 
	}
	
	// create and return DirHandle
	return NULL;
}

