
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

	file = (FileHandle*) current_dir->open_files->first;
	char* block1 = "Ti sta scomparendo il tanga Tanga in mezzo alle chiappe La mia tipa chiama pensa che ci siano altre Con me nella stessa stanza sono in ciabatte sto fumando ganja Ganja vado su marte Hermano Kaleb che passa dall’Argentina all’Italia c’ho la tua tipa che chiama (hace calor) Tu sei una Hakuna matata ma senza alcuna patata tamo ganando un milion Hace calor Hace calor Hace calor Hace calor ";
	char * block2 = "Ho visto l'Amazzonia tra i fumi della città Nera come la macchina, dai, portami mia da qua Mi dai fastidio se mi fai video Tramonto chimico, balliamo in bilico Sulla schiena, fiume in piena Notte fonda, Macarena Brucia lenta l'atmosfera diventa magica E maledetta l'estate Col suono delle sirene, delle cicale Tropicana, danza dolceamara Ballo anche se arriva il temporale E mi piace Notta fonda, luna piena Tropicana, Macarena Ballo anche se arriva il temporale (baila) La senti l'aria? La senti l'aria? La gente brava sbaglia la strada Gira il mondo quando ti muovi tu Suonano i tamburi della tribù Bedda guarda che questa giungla è scura Fa paura solo se guardi giù Ancora questo è il nostro momento Un giorno come adesso non ritorna più Quando mi sveglio in mezzo al cemento Con te il cielo sembra più blu E maledetta l'estate Col suono delle sirene delle cicale Tropicana, danza dolceamara Ballo anche se arriva il temporale E mi piace";
	char* block3 = " mario rossi";
	int to_write1 = strlen(block1);
	int to_write2 = strlen(block2);
	int to_write3 = strlen(block3);
	
	printf("to_write1: %d, to_write2: %d\n", to_write1, to_write2);
	ret = FS_write(file, (void*) block1, to_write1);
	printf("write1 ok\n");
	ret = FS_write(file, (void*) block2, to_write2);
	printf("write2 ok\n");
	ret = FS_write(file, (void*) block3, to_write3);
	printf("write3 ok\n");


	
	FileBlock* file_block = (FileBlock*) malloc(sizeof(FileBlock));
	int32_t current_idx = file->first_block->fcb.first_idx;
	/*while (1) {
		if (current_idx == file->first_block->fcb.first_idx) {
			printf("%s ", file->first_block->block);
		}
		else {
			ret = driver_readBlock(file->fs->first_block, current_idx, file_block);
			if (ret == -1) {
				printf("read error block\n");
				exit(EXIT_FAILURE);
			}
			printf("%s ", file_block->block);
		}
		current_idx = file->fs->fat[current_idx];
		if (current_idx == -1)
			break;
	}
	printf("\n");
	printf("//////////////////////\n");*/

	char* block = (char*) malloc(1001);
	*block = '\0';
	file->pos = 0;
	file->current_block = 0;
	ret = FS_read(file, (void*) block, 1000);
	printf("result: %d\n", ret);
	printf("%s\n", block);

	printf("avaible blocks: %d\n", current_dir->fs->first_block->fat.free_blocks);
	printf("root first_free_entry: %d\n",  root_dir_handle->first_block->first_free_entry);
	printf("avaible blocks: %d\n", root_dir_handle->fs->first_block->fat.free_blocks);
	printf("open files: %d\n", root_dir_handle->open_files->size);
	printf("%d\n",file->current_block);
	
	char* prova = (char*) malloc(353);
	ret = FS_read(file, (void*) prova, 353);
	printf("result: %d\n", ret);
	printf("%s\n", prova);

	file->pos = 532;
	file->current_block = 1;
	char* aux = (char*) malloc(200);
	ret = FS_read(file, (void*) aux, 200);
	printf("result: %d\n", ret);
	printf("%s\n", aux);
	printf("%ld\n", strlen(aux));
	block2 += 137;
	for (int i = 0; i < 200; i++) {
		printf("%c", *block2);
		block2++;
	}
	printf("\n");

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
