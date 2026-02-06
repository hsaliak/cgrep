#ifndef LIST_H
#define LIST_H

#include <stddef.h>

/**
 * @file list.h
 * @brief Generic intrusive singly-linked list infrastructure.
 */

typedef struct list_node {
    struct list_node *next;
} list_node_t;

/**
 * @brief container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 */
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#endif // LIST_H
