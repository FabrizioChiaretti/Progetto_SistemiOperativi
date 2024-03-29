#pragma once

typedef struct ListItem {
  struct ListItem* prev;
  struct ListItem* next;
} ListItem;

typedef struct ListHead {
  ListItem* first;
  ListItem* last;
  int size;
} ListHead;

void List_init(ListHead* head);
ListItem* List_find(ListHead* head, const char* filename);
ListItem* List_insert(ListHead* head, ListItem* item);
ListItem* List_detach(ListHead* head, ListItem* item);
void List_destroy(ListHead* head);
void List_print(ListHead* head);
