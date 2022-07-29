#include "../header/FileSystem.h"
#include "../header/driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main () {
	int ret;
	int fd = open("prova.txt", O_CREAT | O_EXCL | O_RDWR, 0666);
	if (fd == -1) {
		//printf("THE DISKFILE ALREADY EXISTS\n");
		
	}
	fd = open("prova.txt", O_RDWR, 0666);
	printf("%d\n", fd);
	/*printf("%d\n", fd);
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
	return EXIT_SUCCESS;*/
	
}
