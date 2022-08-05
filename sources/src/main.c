
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
			if (i == 1)
				name = "pippo";
			if (i == 3)
				name = "pluto";
			if (i == 5)
				name = "paperino";
			if (i == 7)
				name = "topolino";
			if (i == 9)
				name = "topolina";

			file = FS_createFile(current_dir, name);
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
	printf("size file block: %ld\n", sizeof(file->first_block->block));
	char* block1 = "Ti sta scomparendo il tanga Tanga in mezzo alle chiappe La mia tipa chiama pensa che ci siano altre Con me nella stessa stanza sono in ciabatte sto fumando ganja Ganja vado su marte Hermano Kaleb che passa dall’Argentina all’Italia c’ho la tua tipa che chiama (hace calor) Tu sei una Hakuna matata ma senza alcuna patata tamo ganando un milion Hace calor Hace calor Hace calor Hace calor ";
	char * block2 = "Ho visto l'Amazzonia tra i fumi della città Nera come la macchina, dai, portami mia da qua Mi dai fastidio se mi fai video Tramonto chimico, balliamo in bilico Sulla schiena, fiume in piena Notte fonda, Macarena Brucia lenta l'atmosfera diventa magica E maledetta l'estate Col suono delle sirene, delle cicale Tropicana, danza dolceamara Ballo anche se arriva il temporale E mi piace Notta fonda, luna piena Tropicana, Macarena Ballo anche se arriva il temporale (baila) La senti l'aria? La senti l'aria? La gente brava sbaglia la strada Gira il mondo quando ti muovi tu Suonano i tamburi della tribù Bedda guarda che questa giungla è scura Fa paura solo se guardi giù Ancora questo è il nostro momento Un giorno come adesso non ritorna più Quando mi sveglio in mezzo al cemento Con te il cielo sembra più blu E maledetta l'estate Col suono delle sirene delle cicale Tropicana, danza dolceamara Ballo anche se arriva il temporale E mi piace";
	int to_write1 = strlen(block1)+1;
	int to_write2 = strlen(block2)+1;
	printf("to_write1: %d, to_write2: %d\n", to_write1, to_write2);
	file = (FileHandle*) current_dir->open_files->first;
	ret = FS_write(file, (void*) block1, to_write1);
	printf("ret: %d\n", ret);
	printf("pos: %d\n", file->pos);
	printf("write1 ok\n");
	ret = FS_write(file, (void*) block2, to_write2);
	printf("write2 ok\n");
	if (ret == to_write1 + to_write2) {
		printf("ok\n");
	}
	
	printf("%s\n", file->first_block->block);
	FileBlock* file_block = (FileBlock*) malloc(sizeof(FileBlock));
	ret = driver_readBlock(current_dir->fs->first_block, file->first_block->fcb.last_idx, file_block);
	printf("%s\n", file_block->block);
	printf("////////////////////\n");



	printf("avaible blocks: %d\n", current_dir->fs->first_block->fat.free_blocks);
	printf("root first_free_entry: %d\n",  root_dir_handle->first_block->first_free_entry);
	printf("avaible blocks: %d\n", root_dir_handle->fs->first_block->fat.free_blocks);
	printf("open files: %d\n", root_dir_handle->open_files->size);

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
