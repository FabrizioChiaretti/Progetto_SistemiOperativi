
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

	printf("Hi!\n");
	printf("Initializzation...\n");

	fs* fs_struct = (fs*) malloc(sizeof(fs));
	DirHandle* root_dir_handle = FS_init(fs_struct, DISK_PATHNAME, DISK_DIM, BLOCK_DIM);
	DirHandle* current_dir = (DirHandle*) malloc(sizeof(DirHandle));
	current_dir->fs = root_dir_handle->fs;
	current_dir->open_files = (ListHead*) malloc(sizeof(ListHead));
	List_init(current_dir->open_files);
	current_dir->first_block = (FirstDirBlock*) malloc(sizeof(FirstDirBlock));
	memcpy(current_dir->first_block, root_dir_handle->first_block, sizeof(FirstDirBlock));

	printf("Available blocks: %d\n", root_dir_handle->fs->first_block->fat.free_blocks);

	int path_size = MAX_LENGHT_NAME+1;
	char* path = (char*) calloc(path_size, sizeof(char));
	strcat(path, "/rootdir");
	int path_len = strlen(path);

	size_t nmemb = 20;
	size_t size = sizeof(char);
	size_t token_dim = nmemb*size;
	size_t token_len = 0;
	char* token = (char*) calloc(nmemb, size);

	size_t read = 0;
	while (1) {

		printf("%s$ ", path);

		read = getline(&token, &token_dim, stdin);

		token[strlen(token)-1] = '\0';
		token_len = strlen(token);
		printf("Read command: %s\n", token);

		if (!strcmp(token, "quit")) {

			if (current_dir->open_files->size != 0) {

				printf("Closing all files...\n");

				while (current_dir->open_files->size != 0) {
					ret = FS_close(current_dir, (FileHandle*) current_dir->open_files->first);
				}
			}

			break;

		}

		else if (!strcmp(token, "openedfiles")) {
			
			if (current_dir->open_files->size == 0) {
				printf("There are not opened files\n");
			}

			else {
				List_print(current_dir->open_files);
			}

		}
	
		else if (!strcmp(token, "createfile")) {

			printf("File name: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';
			token_len = strlen(token);

			while (token_len > MAX_LENGHT_NAME) {

				printf("The name is too long, try again\n");
				read = getline(&token, &token_dim, stdin);
				token[strlen(token)-1] = '\0';
				token_len = strlen(token);
			}

			FileHandle* file = FS_createFile(current_dir, token);
			if (file == NULL) {
				printf("File did not create\n");
			}
			
		}

		else if (!strcmp(token, "erasefile")) {

			printf("File name: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			ret = FS_eraseFile(current_dir, token);

		}

		else if (!strcmp(token, "mkdir")) {

			printf("Directory name: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';
			token_len = strlen(token);

			while (token_len > MAX_LENGHT_NAME) {

				printf("The name is too long, try again\n");
				read = getline(&token, &token_dim, stdin);
				token[strlen(token)-1] = '\0';
				token_len = strlen(token);
			}

			ret = FS_mkdir(current_dir, token);		

		}

		else if (!strcmp(token, "erasedir")) {
			
			printf("Directory name: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			ret = FS_eraseDir(current_dir, token);

		}

		else if (!strcmp(token, "open")) {

			
			printf("File name: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';
			char name[MAX_LENGHT_NAME+1];
			strcpy(name, token);
	
			printf("Opening modality: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';
			int mode = atoi(token);

			if (mode != 0 && mode != 1 && mode != 2) {
				printf("Invalid modality\n");
				continue;
			}

			FileHandle* file = FS_openFile(current_dir, name, mode);
			if (file != NULL)
				printf("Position: %d, size: %d\n", file->pos, file->first_block->fcb.dim);
	
		}

		else if (!strcmp(token, "close")) {

			printf("File name: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			FileHandle* file = (FileHandle*) List_find(current_dir->open_files, token);
			if (file == NULL) {
				printf("The file '%s' did not open\n", token);
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

			printf("Directory name: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			int32_t parent_idx = current_dir->first_block->fcb.parent_block;
			char current_name[MAX_LENGHT_NAME+1];
			strcpy(current_name, current_dir->first_block->header.name);
			ret = FS_changeDir(current_dir, token);

			if (ret == -1) {
				continue;
			}

			if (current_dir->first_block == NULL) {
				current_dir->first_block = (FirstDirBlock*) malloc(sizeof(FirstDirBlock));
				memcpy(current_dir->first_block, root_dir_handle->first_block, sizeof(FirstDirBlock));
			}

			if (parent_idx == current_dir->first_block->fcb.first_idx) { 
				
				size_t len = path_len;
				char* aux = path + len;
				aux--;
				size_t current_name_len = strlen(current_name);
				for (int i = 0; i < current_name_len+1; i++) {
					*aux = '\0';
					aux--;
				}

			}

			else {

				token_len = strlen(token);
				
				if (path_len + token_len +1 > path_size -1) {
					path_size *= 2;
					path = (char*) realloc(path, path_size);
				}

				strcat(path, "/");
				strcat(path, token);

			}

			path_len = strlen(path);

		}

		else if (!strcmp(token, "seek")) {

			printf("File name: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			FileHandle* file = (FileHandle*) List_find(current_dir->open_files, token);
			if (file == NULL) {
				printf("The file '%s' did not open\n", token);
				continue;
			}

			printf("offset: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			int offset = atoi(token);

			printf("whence: ");
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
				printf("Command not found\n");
				continue;
			}

			ret = FS_seek(file, pos);

			printf("Position: %d, size: %d\n", file->pos, file->first_block->fcb.dim);

		}

		else if (!strcmp(token, "write")) {
			
			printf("File name: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			FileHandle* file = (FileHandle*) List_find(current_dir->open_files, token);
			if (file == NULL) {
				printf("The '%s' did not open\n", token);
				continue;
			}

			if (file->mode != WR && file->mode != RDWR) {
				printf("The file did not open with write modality\n");
				continue;
			}

			printf("To write: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			int written = FS_write(file, (void*) token, strlen(token));
			
			printf("Written: %d\n", written);

			printf("Position: %d, size: %d\n", file->pos, file->first_block->fcb.dim);

		}

		else if (!strcmp(token, "read")) {

			printf("File name: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			FileHandle* file = (FileHandle*) List_find(current_dir->open_files, token);
			if (file == NULL) {
				printf("The file '%s' did not open\n", token);
				continue;
			}

			if (file->mode != RD && file->mode != RDWR) {
				printf("The file did not open with read modality\n");
				continue;
			}

			printf("Size to read: ");
			read = getline(&token, &token_dim, stdin);
			token[strlen(token)-1] = '\0';

			int to_read = atoi(token);
			size_t size_data = sizeof(char)*(to_read+1);

			char* data = (char*) calloc(to_read, size_data);
			read = FS_read(file, (void*) data, size_data-1);

			if (read != strlen(data)) {
				printf("KO\n");
			}

			printf("%s\n", data);
			free(data);

			printf("Position: %d, size: %d\n", file->pos, file->first_block->fcb.dim);

		}

		else {
			printf("Command not found\n");
		}

		if (!strcmp(current_dir->first_block->header.name, "rootdir")) {
			memcpy(root_dir_handle->first_block, current_dir->first_block, sizeof(FirstDirBlock));
		}
		else {
			ret = driver_writeBlock(current_dir->fs->first_block, current_dir->first_block->fcb.first_idx, current_dir->first_block);
			if (ret == -1) {
				printf("write block error\n");
				exit(EXIT_FAILURE);
			}
		}

	}

	printf("Available blocks: %d\n", root_dir_handle->fs->first_block->fat.free_blocks);

	printf("Exiting...\n");

	free(token);
	free(path);

	if (current_dir != NULL) {

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

	printf("Bye!\n");

	return EXIT_SUCCESS;

}
