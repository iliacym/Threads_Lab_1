#ifndef PTH_LL_RWL_H
#define PTH_LL_RWL_H

typedef struct list_node_s {
    int data;
    struct list_node_s *next;
} list_node_s;

list_node_s* create();
int Insert(list_node_s **head, int value);
int Member(list_node_s **head, int value);
int Delete(list_node_s **head, int value);
void Free_list(list_node_s **head);
int Is_empty(list_node_s **head);

#endif
