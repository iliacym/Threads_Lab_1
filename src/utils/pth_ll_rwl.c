#include <stdio.h>
#include <stdlib.h>
#include "pth_ll_rwl.h"


list_node_s* create() {
    return NULL;
}

/*-----------------------------------------------------------------*/
/* Insert value in correct numerical location into list */
/* If value is not in list, return 1, else return 0 */
int Insert(list_node_s **head, int value) {
    struct list_node_s *curr = *head;
    struct list_node_s *pred = NULL;
    struct list_node_s *temp;
    int rv = 1;

    while (curr != NULL && curr->data < value) {
        pred = curr;
        curr = curr->next;
    }

    if (curr == NULL || curr->data > value) {
        temp = malloc(sizeof(struct list_node_s));
        temp->data = value;
        temp->next = curr;
        if (pred == NULL)
            *head = temp;
        else
            pred->next = temp;
    } else { /* value in list */
        rv = 0;
    }

    return rv;
} /* Insert */


/*-----------------------------------------------------------------*/
int Member(list_node_s **head,int value) {
    struct list_node_s *temp;

    temp = *head;
    while (temp != NULL && temp->data < value)
        temp = temp->next;

    if (temp == NULL || temp->data > value) {
        return 0;
    } else {
        return 1;
    }
} /* Member */

/*-----------------------------------------------------------------*/
/* Deletes value from list */
/* If value is in list, return 1, else return 0 */
int Delete(list_node_s **head,int value) {
    struct list_node_s *curr = *head;
    struct list_node_s *pred = NULL;
    int rv = 1;

    /* Find value */
    while (curr != NULL && curr->data < value) {
        pred = curr;
        curr = curr->next;
    }

    if (curr != NULL && curr->data == value) {
        if (pred == NULL) { /* first element in list */
            *head = curr->next;
            free(curr);
        } else {
            pred->next = curr->next;
            free(curr);
        }
    } else { /* Not in list */
        rv = 0;
    }

    return rv;
} /* Delete */

/*-----------------------------------------------------------------*/
void Free_list(list_node_s **head) {
    struct list_node_s *current;
    struct list_node_s *following;

    if (Is_empty(*head))
        return;
    current = *head;
    following = current->next;
    while (following != NULL) {
        free(current);
        current = following;
        following = current->next;
    }
    free(current);
} /* Free_list */

/*-----------------------------------------------------------------*/
int Is_empty(list_node_s **head) {
    if (*head == NULL)
        return 1;
    else
        return 0;
} /* Is_empty */
