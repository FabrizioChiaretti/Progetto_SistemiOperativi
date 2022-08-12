CC=gcc
CCOPTS=--std=gnu99 -Wall 
AR=ar


HEADERS=../header/driver.h\
	../header/linked_list.h\
	../header/FileSystem.h


OBJS= driver.o\
	linked_list.o\
	FileSystem.o


BINS= FS_main


.phony: clean all


all:	$(BINS) 


%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

FS_main: FS_main.c $(OBJS) 
	$(CC) $(CCOPTS)  -o $@ $^ -lm


clean:
	rm -rf *.o *~  $(OBJS) 
