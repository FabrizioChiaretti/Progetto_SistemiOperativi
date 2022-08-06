#include "../header/linked_list.h"
#include "../header/FileSystem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void List_init(ListHead* head) {

    head->first = NULL;
  	head->last = NULL;
  	head->size = 0;
}


ListItem* List_find(ListHead* head, const char* filename){
    
    ListItem* aux = head->first;
    while(aux){
		  if (!strcmp(((FileHandle*)aux)->first_block->header.name, filename)) 
			  return aux;
		  aux = aux->next;
	  }

	  return NULL;
}


ListItem* List_insert(ListHead* head, ListItem* item) {

    if (head->first == NULL && head->last == NULL) {
        head->first = head->last = item;
        item->next = item->prev = NULL;
    }  
    else {
        head->last->next = item;
        item->prev = head->last;
        head->last = item;
        item->next = NULL;
    }  

    head->size++;
    return item;
}


ListItem* List_detach(ListHead* head, ListItem* item) {

    ListItem* prev=item->prev;
    ListItem* next=item->next;

    if (prev)
        prev->next = next;

    if (next)
        next->prev = prev;

    if (item == head->first)
        head->first = next;

    if (item == head->last)
        head->last = prev;

    head->size--;
    item->next = item->prev = 0;
  
    return item;
}


void List_destroy(ListHead* head) {
  
    if (head != NULL) {
        if(head->first == NULL) {
          free(head);
          return;
        }

        FileHandle* file;

        while(head->size != 0) {
            ListItem* item = List_detach(head, head->first);
            file = (FileHandle*) item;

            if(file->first_block != NULL)
                free(file->first_block);
        
            free(file);
        }
        free(head);
    }


    return;
}


void List_print(ListHead* head) {

    ListItem* aux = head->first;
    while(aux) {
        printf("%s  ",((FileHandle*)aux)->first_block->header.name);
        aux = aux->next;
    }
    printf("\n");
    return;
}
