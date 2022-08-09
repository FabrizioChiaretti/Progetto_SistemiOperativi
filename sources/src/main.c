
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
#define QUIT "quit\n"


int main (int argc, char** argv) {

	int ret;

	printf("Hi!\n");
	printf("Fat Filesystem init...\n");

	fs* fs_struct = (fs*) malloc(sizeof(fs));
	DirHandle* root_dir_handle = FS_init(fs_struct, DISK_PATHNAME, DISK_DIM, BLOCK_DIM);
	DirHandle* current_dir = root_dir_handle;

	printf("avaible blocks: %d\n", current_dir->fs->first_block->fat.free_blocks);

	size_t nmemb = 20;
	size_t size = sizeof(char);
	size_t token_dim = nmemb*size;
	size_t token_len = 0;
	char* token = (char*) calloc(nmemb, size);

	/*
	for (int i = 0; i < ARGC; i++) {
		switch (i) {

			case 0:
				nmemb = 11;

			case 1:
				nmemb = MAX_LENGHT_NAME +1;

			case 2:
				nmemb = MAX_TEXT_LENGHT +1;

			case 3:
				nmemb = 8;
		}

		tokens[i] = (char*) calloc(nmemb, size);
	}
	

	nmemb = 11 + MAX_LENGHT_NAME + MAX_TEXT_LENGHT + 8 +1;
	size_t line_dim = nmemb;
	char* line = (char*) calloc(nmemb, sizeof(char));
	*/

	size_t read = 0;
	while (1) {

		read = getline(&token, &token_dim, stdin);

		if (!strcmp(token, QUIT))
			break;

		token[strlen(token)-1] = '\0';
		token_len = strlen(token);
		printf("token len: %ld, command read: %s\n", token_len, token);
	
		

	}

	printf("avaible blocks: %d\n", current_dir->fs->first_block->fat.free_blocks);

	printf("Exiting...\n");

	/*
	for (int i = 0; i < ARGC; i++) {
		free(tokens[i]);
	}
	*/

	free(token);

	if (current_dir != NULL && current_dir != root_dir_handle) {

		if (current_dir->open_files != NULL)
			free(current_dir->open_files);

		if (current_dir->first_block != NULL)
			free(current_dir->first_block);

		free(current_dir);
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

	printf("Bye\n");
	return EXIT_SUCCESS;
}
