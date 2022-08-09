
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
	/*DirHandle* current_dir = (DirHandle*) malloc(sizeof(DirHandle));
	current_dir->fs = root_dir_handle->fs;
	current_dir->open_files = (ListHead*) malloc(sizeof(ListHead));
	current_dir->first_block = (FirstDirBlock*) malloc(sizeof(FirstDirBlock));
	List_init(current_dir->open_files);
	memcpy(current_dir->first_block, root_dir_handle->first_block, sizeof(FirstDirBlock));*/

	printf("avaible blocks: %d\n", current_dir->fs->first_block->fat.free_blocks);
	printf("root first_free_entry: %d\n",  current_dir->first_block->first_free_entry);
	printf("avaible blocks: %d\n", current_dir->fs->first_block->fat.free_blocks);
	printf("open files: %d\n", root_dir_handle->open_files->size);
	FileHandle* file;

	char* name = "giulio";
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
	
	ret = FS_write(file, (void*) block1, to_write1);
	ret = FS_write(file, (void*) block2, to_write2);
	ret = FS_write(file, (void*) block3, to_write3);
	printf("write3 ok\n");

	ret = driver_writeBlock(current_dir->fs->first_block, file->first_block->fcb.first_idx, file->first_block);
	if(ret == -1) {
		printf("write rootblock error\n");
		exit(EXIT_FAILURE);
	}

	ret = driver_writeBlock(current_dir->fs->first_block, 0, current_dir->first_block);
	if(ret == -1) {
		printf("write rootblock error\n");
		exit(EXIT_FAILURE);
	}

	printf("avaible blocks: %d\n", current_dir->fs->first_block->fat.free_blocks);
	printf("root first_free_entry: %d\n",  root_dir_handle->first_block->first_free_entry);
	printf("open files: %d\n", root_dir_handle->open_files->size);

	char filename[30];
	strcpy(filename, file->first_block->header.name);
	ret = FS_close(current_dir, file);

	char** ls_dir = (char**) malloc(sizeof(char*)*current_dir->first_block->fcb.dim);
	ret = FS_listing(current_dir, ls_dir);
	printf("result: %d, entries: %d\n", ret, current_dir->first_block->fcb.dim);
	for (int i = 0; i < current_dir->first_block->fcb.dim; i++) {
		printf("%s\n", ls_dir[i]);
	}
	printf("\n");
	printf("eliminando: %s\n", filename);
	ret = FS_eraseFile(current_dir, filename);
	ls_dir = (char**) malloc(sizeof(char*)*current_dir->first_block->fcb.dim);
	ret = FS_listing(current_dir, ls_dir);
	printf("result: %d, entries: %d\n", ret, current_dir->first_block->fcb.dim);
	for (int i = 0; i < current_dir->first_block->fcb.dim; i++) {
		printf("%s\n", ls_dir[i]);
	}
	printf("\n");
	printf("avaible blocks: %d\n", current_dir->fs->first_block->fat.free_blocks);
	printf("root first_free_entry: %d\n",  root_dir_handle->first_block->first_free_entry);
	printf("open files: %d\n", root_dir_handle->open_files->size);

	printf("eliminando: %s\n", "pitagora");
	ret = FS_eraseDir(current_dir, "pitagora");
	ls_dir = (char**) malloc(sizeof(char*)*current_dir->first_block->fcb.dim);
	ret = FS_listing(current_dir, ls_dir);
	printf("result: %d, entries: %d\n", ret, current_dir->first_block->fcb.dim);
	for (int i = 0; i < current_dir->first_block->fcb.dim; i++) {
		printf("%s\n", ls_dir[i]);
	}
	printf("\n");
	printf("avaible blocks: %d\n", current_dir->fs->first_block->fat.free_blocks);
	printf("root first_free_entry: %d\n",  root_dir_handle->first_block->first_free_entry);
	printf("open files: %d\n", root_dir_handle->open_files->size);

	printf("change cicerone\n");
	current_dir = (DirHandle*) malloc(sizeof(DirHandle));
	current_dir->fs = root_dir_handle->fs;
	current_dir->open_files = (ListHead*) malloc(sizeof(ListHead));
	current_dir->first_block = (FirstDirBlock*) malloc(sizeof(FirstDirBlock));
	List_init(current_dir->open_files);
	memcpy(current_dir->first_block, root_dir_handle->first_block, sizeof(FirstDirBlock));

	ret = FS_changeDir(root_dir_handle, current_dir, "cicerone");
	if (current_dir->first_block == NULL) {
		free(current_dir);
		current_dir = root_dir_handle;
	}

	printf("///////// create 4 file\n");
	for (int i = 0; i < 2; i++) {
		if (i == 0)
			name = "bruto";
		else 
			name = "cassio";
		file = FS_createFile(current_dir, name);
	}

	for (int i = 0; i < 2; i++) {
		if (i == 0)
			name = "Manzoni";
		else 
			name = "Alighieri";
			ret = FS_mkdir(current_dir, name);
	}

	ls_dir = (char**) malloc(sizeof(char*)*current_dir->first_block->fcb.dim);
	ret = FS_listing(current_dir, ls_dir);
	printf("result: %d, entries: %d\n", ret, current_dir->first_block->fcb.dim);
	for (int i = 0; i < current_dir->first_block->fcb.dim; i++) {
		printf("%s\n", ls_dir[i]);
	}

	while (current_dir->open_files->size != 0) {
		ret = FS_close(current_dir, (FileHandle*) current_dir->open_files->first);
		if(ret == -1) {
			printf("close error\n");
			exit(EXIT_FAILURE);
		} 
	}

	printf("///////////// cicerone erase\n");

	ret = driver_writeBlock(current_dir->fs->first_block, current_dir->first_block->fcb.first_idx, current_dir->first_block);
	if(ret == -1) {
		printf("write rootblock error\n");
		exit(EXIT_FAILURE);
	}


	ret = FS_changeDir(root_dir_handle, current_dir, "..");
	if (current_dir->first_block == NULL) {
		free(current_dir);
		current_dir = root_dir_handle;
	}

	ret = FS_eraseDir(current_dir, "cicerone");

	ls_dir = (char**) malloc(sizeof(char*)*current_dir->first_block->fcb.dim);
	ret = FS_listing(current_dir, ls_dir);
	printf("result: %d, entries: %d\n", ret, current_dir->first_block->fcb.dim);
	for (int i = 0; i < current_dir->first_block->fcb.dim; i++) {
		printf("%s\n", ls_dir[i]);
	}
	printf("\n");

	printf("avaible blocks: %d\n", root_dir_handle->fs->first_block->fat.free_blocks);
	printf("root first_free_entry: %d\n",  root_dir_handle->first_block->first_free_entry);
	printf("open files: %d\n", root_dir_handle->open_files->size);

	ret = FS_flush(root_dir_handle->fs->first_block);
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
