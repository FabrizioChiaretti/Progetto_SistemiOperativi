
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

	printf("Ciao!\n");
	printf("Inizializzazione...\n");

	fs* fs_struct = (fs*) malloc(sizeof(fs));
	DirHandle* root_dir_handle = FS_init(fs_struct, DISK_PATHNAME, DISK_DIM, BLOCK_DIM);
	DirHandle* current_dir = (DirHandle*) malloc(sizeof(DirHandle));
	current_dir->fs = root_dir_handle->fs;
	current_dir->open_files = (ListHead*) malloc(sizeof(ListHead));
	List_init(current_dir->open_files);
	current_dir->first_block = (FirstDirBlock*) malloc(sizeof(FirstDirBlock));
	memcpy(current_dir->first_block, root_dir_handle->first_block, sizeof(current_dir->first_block));

	printf("Blocchi disponibili: %d\n", root_dir_handle->fs->first_block->fat.free_blocks);

	size_t nmemb = 20;
	size_t size = sizeof(char);
	size_t token_dim = nmemb*size;
	size_t token_len = 0;
	char* token = (char*) calloc(nmemb, size);

	size_t read = 0;
	while (1) {

		printf("FS> ");

		read = getline(&token, &token_dim, stdin);

		token[strlen(token)-1] = '\0';
		token_len = strlen(token);
		printf("comando da eseguire: %s\n", token);

		if (!strcmp(token, "quit")) {

			if (current_dir->open_files->size != 0) {

				printf("Chiudo tutti i file aperti\n");

				while (current_dir->open_files->size != 0) {
					FileHandle* file = (FileHandle*) List_detach(current_dir->open_files, current_dir->open_files->first);
					ret = FS_close(current_dir, file);
				}
			}

			break;

		}
	
		else if (!strcmp(token, "createfile")) {

			printf("Nome del file: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';
			token_len = strlen(token);

			while (token_len > MAX_LENGHT_NAME) {

				printf("Il nome è troppo lungo, riprova\n");
				read = getline(&token, &token_dim, stdin);
				token[strlen(token)-1] = '\0';
				token_len = strlen(token);
			}

			FileHandle* file = FS_createFile(current_dir, token);
			
		}

		else if (!strcmp(token, "erasefile")) {

			printf("Nome del file: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			ret = FS_eraseFile(current_dir, token);

		}

		else if (!strcmp(token, "mkdir")) {

			printf("Nome della directory: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';
			token_len = strlen(token);

			while (token_len > MAX_LENGHT_NAME) {

				printf("Il nome è troppo lungo\n");
				read = getline(&token, &token_dim, stdin);
				token[strlen(token)-1] = '\0';
				token_len = strlen(token);
			}

			ret = FS_mkdir(current_dir, token);		

		}

		else if (!strcmp(token, "erasedir")) {
			
			printf("Nome della directory: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			ret = FS_eraseDir(current_dir, token);

		}

		else if (!strcmp(token, "open")) {

			printf("Nome del file: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';
	
			printf("Modalità di apertura: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';
			int mode = atoi(token);

			FileHandle* file = FS_openFile(current_dir, token, mode);

		}

		else if (!strcmp(token, "close")) {

			printf("Nome del file: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			FileHandle* file = (FileHandle*) List_find(current_dir->open_files, token);
			if (file == NULL) {
				printf("il file '%s' non è stato aperto\n", token);
				continue;
			}

			ret = FS_close(current_dir, file);

		}

		else if (!strcmp(token, "ls")) {

			size_t nmemb = current_dir->first_block->fcb.dim;
			size_t size = sizeof(char*);
			char** names = (char**) calloc(nmemb, size);

			ret = FS_listing(current_dir, names);

			if (ret != -1) {
				for (int i = 0; i < nmemb; i++) {
					printf("%s\n", names[i]);
				}
			}

			for (int i = 0; i < nmemb; i++) {
				if (names[i] != NULL)
					free(names[i]);
			}

			free(names);

		}

		else if (!strcmp(token, "cd")) {

			printf("Nome della directory: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			ret = FS_changeDir(current_dir, token);

			if (current_dir->first_block == NULL) {
				current_dir->first_block = (FirstDirBlock*) malloc(sizeof(FirstDirBlock));
				memcpy(current_dir->first_block, root_dir_handle->first_block, sizeof(FirstDirBlock));
			}

		}

		else if (!strcmp(token, "seek")) {

			printf("Nome del file: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			FileHandle* file = (FileHandle*) List_find(current_dir->open_files, token);
			if (file == NULL) {
				printf("il file '%s' non è stato aperto\n", token);
				continue;
			}

			printf("offset: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			int offset = atoi(token);

			printf("da dove: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			int pos;

			if (!strcmp(token, "start")) {
				pos = offset;
			}
			
			else if (!strcmp(token, "current")) {
				pos = file->pos + offset; 
			}

			else if (!strcmp(token, "end")) {
				pos = file->first_block->fcb.dim - offset;
			}

			else {
				printf("comando non valido\n");
				continue;
			}

			ret = FS_seek(file, pos);

		}

		else if (!strcmp(token, "write")) {
			
			printf("Nome del file: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			FileHandle* file = (FileHandle*) List_find(current_dir->open_files, token);
			if (file == NULL) {
				printf("Il file '%s' non è stato aperto\n", token);
				continue;
			}

			printf("Bytes da scrivere dalla posizione corrente: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			int written = FS_write(file, (void*) token, strlen(token));

			if (written != strlen(token)) {
				printf("KO\n");
			}

		}

		else if (!strcmp(token, "read")) {

			printf("Nome del file: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			FileHandle* file = (FileHandle*) List_find(current_dir->open_files, token);
			if (file == NULL) {
				printf("Il file '%s' non è stato aperto\n", token);
				continue;
			}

			printf("Bytes da leggere dalla posizione corrente: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			int to_read = atoi(token);

			char* data = (char*) calloc(to_read, sizeof(char));
			read = FS_read(file, (void*) data, size);

			if (read != strlen(data)) {
				printf("KO\n");
			}

		}

		else {
			printf("comando non disponibile\n");
		}

		if (!strcmp(current_dir->first_block->header.name, "rootdir")) {
			memcpy(root_dir_handle->first_block, current_dir->first_block, sizeof(current_dir->first_block));
		}

	}

	printf("Blocchi disponibili: %d\n", current_dir->fs->first_block->fat.free_blocks);

	printf("Esco...\n");

	free(token);

	if (current_dir != NULL && current_dir != root_dir_handle) {

		if (current_dir->open_files != NULL)
			free(current_dir->open_files);

		if (current_dir->first_block != NULL)
			free(current_dir->first_block);

		free(current_dir);
	}

	ret = driver_writeBlock(root_dir_handle->fs->first_block, root_dir_handle->first_block->fcb.first_idx, root_dir_handle->first_block);
	if (ret == -1) {
		printf("write rootdir block error\n");
		exit(EXIT_FAILURE);
	}

	if (root_dir_handle != NULL) {

		if (root_dir_handle->open_files != NULL)
			free(root_dir_handle->open_files);
		
		if (root_dir_handle->first_block != NULL)
			free(root_dir_handle->first_block);
		
		free(root_dir_handle);
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

	free(fs_struct);

	printf("Arrivederci!\n");

	return EXIT_SUCCESS;

}
