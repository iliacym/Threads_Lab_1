#include <stdio.h>
#include <stdlib.h>
#include "pth_ll_rwl.h"


list_node_s* create() {
    return NULL;
}

/*-----------------------------------------------------------------*/
/* Insert value in correct numerical location into list */
/* If value is not in list, return 1, else return 0 */
int Insert(list_node_s **head, int const value) {
    list_node_s *curr = *head;
    list_node_s *pred = NULL;
    int rv = 1;

    while (curr != NULL && curr->data < value) {
        pred = curr;
        curr = curr->next;
    }

    if (curr == NULL || curr->data > value) {
        list_node_s *temp = malloc(sizeof(struct list_node_s));
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
int Member(list_node_s **head, int const value) {
    list_node_s *temp = *head;
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
int Delete(list_node_s **head, int const value) {
    list_node_s *curr = *head;
    list_node_s *pred = NULL;
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
    if (*head == NULL)
        return;
    list_node_s *current = *head;
    list_node_s *following = current->next;
    while (following != NULL) {
        free(current);
        current = following;
        following = current->next;
    }
    free(current);
} /* Free_list */

