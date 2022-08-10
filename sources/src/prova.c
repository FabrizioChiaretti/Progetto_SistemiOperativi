#include "../header/FileSystem.h"
#include "../header/driver.h"
#include "../header/linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


int main () {
	/*int ret;
	int fd = open("prova.txt", O_CREAT | O_EXCL | O_RDWR, 0666);
	if (fd == -1) {
		//printf("THE DISKFILE ALREADY EXISTS\n");
		
	}
	fd = open("prova.txt", O_RDWR, 0666);
	printf("%d\n", fd);
	printf("%d\n", fd);
	int size = 100;
	ret = ftruncate(fd, size);
	if (ret == -1) {
		printf("truncate\n");
		exit(EXIT_FAILURE);
	}
	char* ptr = (char*) mmap(0, 11, PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == NULL) {
		printf("ptr\n");
		exit(EXIT_FAILURE);
	}
	char* msg = "HELLO WORLD";
	printf("%ld\n", strlen(msg));
	memcpy(ptr, msg, strlen(msg));
	ret = msync(ptr, strlen(msg), MS_SYNC);
	if (ret == -1) {
		printf("msync\n");
		exit(EXIT_FAILURE);
	}
	ret = munmap((void*) ptr, 100);
	if (ret == -1) {
		printf("munmap\n");
		exit(EXIT_FAILURE);
	}
	ret = close(fd);
	if (ret == -1) {
		printf("close\n");
		exit(EXIT_FAILURE);
	}
	printf("ok\n");
	return EXIT_SUCCESS;
	/ListHead* head = (ListHead*) malloc(sizeof(ListHead));
	List_init(head);
	char* arr;
	for (int i = 0; i < 5; i++) {
		FileHandle* file = (FileHandle*) malloc(sizeof(FileHandle));
		file->first_block = (FirstFileBlock*) malloc(sizeof(FirstFileBlock));
		if (i == 0) 
			arr = "mario";
		if (i == 1)
			arr = "pietro";
		if (i == 2)
			arr = "marco";
		if (i == 3)
			arr = "matteo";
		if (i == 4)
			arr = "paolo";
		strcpy(file->first_block->header.name, arr);
		List_insert(head, (ListItem*) file);
	}
	List_print(head);
	ListItem* item;
	for (int i = 0; i < 5; i++) {
		if (i == 0) {
			item = List_find(head, "matteo");
			List_detach(head, item);
		}
		if (i == 1) {
			item = List_find(head, "paolo");
			List_detach(head, item);
		}
		if (i == 2) {
			item = List_find(head, "mario");
			List_detach(head, item);
		}
		if (i == 3) {
			item = List_find(head, "pietro");
			List_detach(head, item);
		}
		if (i == 4) {
			item = List_find(head, "marco");
			List_detach(head, item);
		}
		List_print(head);
	}*/
	size_t token_dim = 20;
	char* token = (char*) malloc(sizeof(token_dim));
	int result;

	while(token != "q") {
		getline(&token, &token_dim, stdin);
		token[strlen(token)-1] = '\0';
		result = atoi(token);
		printf("%d\n", result);
	}
}





