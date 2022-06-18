
#pragma once

#include <FileSystem.h>

typedef struct {
  struct ListItem* prev;
  struct ListItem* next;
} ListItem;

typedef struct {
  ListItem* first;
  ListItem* last;
  int size;
} ListHead;

void List_init(ListHead* head);
ListItem* List_find(ListHead* head, ListItem* item);
ListItem* List_insert(ListHead* head, ListItem* previous, ListItem* item);
ListItem* List_detach(ListHead* head, ListItem* item);


typedef struct {
	ListItem item;
	DirectoryHandle* handle;
} DirHandleItem;

typedef struct {
	ListItem item;
	FileHandle* handle;
} FileHandleItem;


typedef struct {
  ListHead head;
} DirListHead;


typedef struct {
  ListHead head;
} FileListHead;




