
#include "../header/driver.h"
#include "../header/FileSystem.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

void disk_init(FirstDiskBlock* disk, int disk_dim, int block_dim) {

	(disk->header).disk_dim = disk_dim;
    (disk->header).block_dim = block_dim;
    (disk->header).total_blocks = disk_dim / block_dim; // 16 384, 512/4 = 128, 16 384/128 = 128
    //printf("total blocks %d\n", (disk->header).total_blocks);
    (disk->header).fat_blocks = ((disk->header).total_blocks / (block_dim / sizeof(int32_t))) -1; // 127, the last idx is invalid
    //printf("fat blocks %d\n", (disk->header).fat_blocks);
    (disk->header).num_blocks = (disk->header).total_blocks - 2 - (disk->header).fat_blocks; //-2 because the last idx of the last fat block is invalid
    //printf("user blocks %d\n", (disk->header).num_blocks);
    disk->rootDir_idx = (disk->header).fat_blocks +1;
    //printf("root_idx %d\n", disk->rootDir_idx);
    disk->fat.dim = (disk->header).fat_blocks*((disk->header).block_dim/sizeof(int32_t)) -1;
    //printf("fat dim %d\n", disk->fat.dim);
    disk->fat.first_free = 1; // 0 is used for the first block of the directory block
    disk->fat.free_blocks = disk->fat.dim -1;
    //printf("fat free blocks %d\n", disk->fat.free_blocks);
    disk->fat.first_fatBlock = 1;
    int32_t* fat = (int32_t*) (disk+1);
    fat[0] = LAST_BLOCK;

    for (int i = 1; i < disk->fat.dim-1; i++) {
        fat[i] = FREE_BLOCK;
    }

    return;
}

int driver_readBlock(FirstDiskBlock* disk, int32_t block_num, void* dest) {

    int32_t* fat = (int32_t*) (disk->fat.first_fatBlock + disk);
    if (block_num < 0 || block_num >= disk->fat.dim || fat[block_num] < -1) {
        printf("readBlockError, block_num not valid\n");
        return -1;
    }
    void* disk_block = (void*) (disk + disk->rootDir_idx + block_num);
    memcpy(dest, disk_block, BLOCK_DIM);

    return 0;
}

int driver_writeBlock(FirstDiskBlock* disk, int32_t block_num, void* source) {

    int32_t* fat = (int32_t*) (disk->fat.first_fatBlock + disk);
    if (block_num < 0 || block_num >= disk->fat.dim || fat[block_num] < -1) {
        printf("writeBlockError, block_num not valid\n");
        return -1;
    }
    void* disk_block = (void*) (disk + disk->rootDir_idx + block_num);
    memcpy(disk_block, source, BLOCK_DIM);

    return 0;
}


int driver_freeBlock(FirstDiskBlock* disk, int32_t block_num) {
    
    int32_t* fat = (int32_t*) (disk->fat.first_fatBlock + disk);
    if (block_num >= disk->fat.dim || fat[block_num] == FREE_BLOCK) {
        printf("freeBlockerror, block_num not valid\n");
        return -1;
    }
    if (disk->fat.first_free > block_num) {
        disk->fat.first_free = block_num;
    }
    fat[block_num] = FREE_BLOCK;
    disk->fat.free_blocks++;
    void* disk_block = (void*) (disk + disk->rootDir_idx + block_num);
    memset(disk_block, 0, BLOCK_DIM); // clean block

    return 0;
}


int driver_getfreeBlock(FirstDiskBlock* disk) {
    
    if (disk->fat.free_blocks == 0) {
        printf("there is not fat free blocks\n");
        return -1;
    }
    disk->fat.free_blocks--;
    int32_t* fat = (int32_t*) (disk->fat.first_fatBlock + disk);
    int result = disk->fat.first_free;
    fat[result] = LAST_BLOCK;
    int i;
    for (i = result+1; i < disk->fat.dim; i++) {
        if (fat[i] == FREE_BLOCK) {
            disk->fat.first_free = i;
            break;
        }
    }
    if (disk->fat.free_blocks == 0) {
        disk->fat.first_free = -1;
    }
    printf("avaible blocks: %d\n", disk->fat.free_blocks);
    return result;
}

// non so se serve
int driver_flush(FirstDiskBlock* disk, int32_t block_num, int8_t flush_fat) { 

    int ret;
    /*if (flush_fat) {
        void* fat = (void*) (disk + disk->fat.first_fatBlock);
        ret = msync(fat, (size_t) disk->fat.dim*sizeof(int32_t), MS_SYNC);
        if (ret == -1) {
            printf("msync fat error\n");
            return ret;
        }
    }
    //void* block = (void*) (disk + disk->rootDir_idx + block_num);
    ret = msync(block, BLOCK_DIM, MS_SYNC);
    if (ret == -1) {
        printf("msync block error\n");
        return ret;
    }*/
    ret = msync(disk, disk->header.disk_dim, MS_SYNC);
    if (ret == -1)
        printf("ensomma\n");
    return ret;
}



