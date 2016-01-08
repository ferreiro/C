#ifndef SLIST_H
#define SLIST_H
#ifdef __KERNEL__
#include <linux/list.h>
#else
#include <string.h>
#include <stdlib.h>
#include "list.h"
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

typedef struct {
    struct list_head list;
    size_t size;
    size_t node_offset;
} slist_t;

typedef struct list_head list_node_t;

/* Operations on slist_t */
void init_slist (slist_t* slist, size_t node_offset);
void insert_slist ( slist_t* slist,void* elem);
void insert_slist_head ( slist_t* slist,void* elem);
void remove_slist ( slist_t* slist,void* elem);
void* head_slist ( slist_t* slist);
void* tail_slist ( slist_t* slist);
void* next_slist ( slist_t* slist,void* elem);
void* prev_slist ( slist_t* slist,void* elem);
void insert_after_slist(slist_t* slist, void *object, void *nobject);
void insert_before_slist(slist_t* slist, void *object, void *nobject);
static inline int is_empty_slist(slist_t* slist)
{
    return slist->size==0;
}
static inline int size_slist(slist_t* slist)
{
    return slist->size;
}
void sorted_insert_slist(slist_t* slist, void* object, int ascending, int (*compare)(void*,void*));
void sorted_insert_slist_front(slist_t* slist, void* object, int ascending, int (*compare)(void*,void*));
void sort_slist(slist_t* slist, int ascending, int (*compare)(void*,void*));

#endif	/* _SLIST_H*/
