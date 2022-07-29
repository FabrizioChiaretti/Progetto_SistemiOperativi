
#include "../header/driver.h"
#include "../header/FileSystem.h"
#include <stdio.h>

void disk_init(FirstDiskBlock* disk, int disk_dim, int block_dim) {

	(disk->header).disk_dim = disk_dim;
    (disk->header).block_dim = block_dim;
    (disk->header).total_blocks = disk_dim / block_dim; // 16 384, 512/4 = 128, 16 384/128 = 128
    (disk->header).fat_blocks = ((disk->header).total_blocks / (block_dim / sizeof(int32_t))) -1; // 127, the last idx is invalid
    (disk->header).num_blocks = (disk->header).total_blocks - 2 - (disk->header).fat_blocks; //-2 because the last idx of the last fat block is invalid
    disk->rootDir_idx = (disk->header).fat_blocks +1;
    disk->fat.dim = (disk->header).fat_blocks*128 -1;
    disk->fat.first_free = 1; // 0 is used for the first block of the directory block
    disk->fat.free_blocks = disk->fat.dim -1;
    uint8_t* aux = (uint8_t*) disk; 
    aux += block_dim;
    disk->fat.fat = (int32_t*) aux;
    int32_t* fat = disk->fat.fat; 
    *fat = LAST_BLOCK;

    for (int i = 1; i < disk->fat.dim; i++) {
        fat[i] = FREE_BLOCK;
        if (i == disk->fat.dim-1);
    }

    aux = (uint8_t*) disk;
    aux += block_dim*(disk->rootDir_idx);
    RootDir* rootdir = (RootDir*) aux;
    (rootdir->header).flag = ROOTDIR;
    //(rootdir->header).name = "rootdir";
    (rootdir->fcb).dim = 0;
    (rootdir->fcb).first_idx = 0;
    (rootdir->fcb).parent_block = -1;

    return;
}
