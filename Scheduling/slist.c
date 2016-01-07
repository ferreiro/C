#include "slist.h"
#ifndef NULL
#define NULL	0
#endif

void init_slist (slist_t* slist, size_t node_offset)
{
    INIT_LIST_HEAD(&slist->list);
    slist->node_offset=node_offset;
    slist->size=0;
}

void insert_slist ( slist_t* slist,void* elem)
{
    struct list_head* link=((struct list_head*)(((char*)elem) +  slist->node_offset));
    list_add_tail(link,&slist->list);
    slist->size++;
}

void insert_slist_head ( slist_t* slist,void* elem)
{
    struct list_head* link=((struct list_head*)(((char*)elem) +  slist->node_offset));
    list_add(link,&slist->list);
    slist->size++;
}

void remove_slist ( slist_t* slist,void* elem)
{
    struct list_head* link=((struct list_head*)(((char*)elem) +  slist->node_offset));
    list_del(link);
    slist->size--;
}

void* head_slist ( slist_t* slist)
{
    struct list_head *first=NULL;
    void* first_item=NULL;

    if (!list_empty(&slist->list)) {
        first = slist->list.next;
        first_item= ((char*)first) - slist->node_offset;
    }

    return first_item;
}


void* tail_slist ( slist_t* slist)
{
    struct list_head *last=NULL;
    void* last_item=NULL;

    if (!list_empty(&slist->list)) {
        last = slist->list.prev;
        last_item= ((char*)last) - slist->node_offset;
    }

    return last_item;
}

void* next_slist ( slist_t* slist,void* elem)
{
    struct list_head *cur=NULL;

    if (!elem)
        return NULL;

    cur=((struct list_head*)(((char*)elem) +  slist->node_offset));

    /* Check if we reached the end of the list */
    if (cur->next==&slist->list)
        return NULL;

    /* Otherwise return item */
    return ((char*)cur->next) - slist->node_offset;
}

void* prev_slist ( slist_t* slist,void* elem)
{
    struct list_head *cur=NULL;

    if (!elem)
        return NULL;

    cur=((struct list_head*)(((char*)elem) +  slist->node_offset));

    /* Check if we reached the end of the list */
    if (cur->prev==&slist->list)
        return NULL;

    /* Otherwise return item */
    return ((char*)cur->prev) - slist->node_offset;
}

void insert_after_slist(slist_t *slist, void *object, void *nobject)
{
    struct list_head* prev_node=((struct list_head*)(((char*)object) +  slist->node_offset));
    struct list_head* new_node=((struct list_head*)(((char*)nobject) +  slist->node_offset));

    if (!object) {
        insert_slist_head(slist,nobject);
    } else {
        list_add(new_node,prev_node);
        slist->size++;
    }
}

void insert_before_slist(slist_t *slist, void *object, void *nobject)
{
    struct list_head* prev_node=((struct list_head*)(((char*)object) +  slist->node_offset));
    struct list_head* new_node=((struct list_head*)(((char*)nobject) +  slist->node_offset));

    if (!object) {
        insert_slist(slist,nobject);
    } else {
        list_add_tail(new_node,prev_node);
        slist->size++;
    }
}

void sorted_insert_slist(slist_t* slist, void* object, int ascending, int (*compare)(void*,void*))
{
    void *cur=NULL;

    cur=head_slist(slist);
    /* Search */
    if (ascending) {
        // Find
        while(cur!=NULL && compare(cur,object)<=0) {
            cur=next_slist(slist,cur);
        }
    } else {
        // Find
        while(cur!=NULL && compare(cur,object)>=0) {
            cur=next_slist(slist,cur);
        }
    }

    insert_before_slist(slist,cur,object);
}


void sorted_insert_slist_front(slist_t* slist, void* object, int ascending, int (*compare)(void*,void*))
{
    void *cur=NULL;

    cur=head_slist(slist);
    /* Search */
    if (ascending) {
        // Find
        while(cur!=NULL && compare(cur,object)<0) {
            cur=next_slist(slist,cur);
        }
    } else {
        // Find
        while(cur!=NULL && compare(cur,object)>0) {
            cur=next_slist(slist,cur);
        }
    }

    insert_before_slist(slist,cur,object);
}

void sort_slist(slist_t* slist, int ascending, int (*compare)(void*,void*))
{
    void *cur=NULL,*selected_node=NULL,*prev_selected=NULL;
    int i=0;

    /* Check if the list is already trivially sorted */
    if (slist->size<=1)
        return;

    cur=head_slist(slist);

    /* Insertion sort */
    for (i=0; i<slist->size-1 && cur!=NULL; i++) {

        /* Search */
        selected_node=cur;

        if (ascending) {
            // Search for min
            while(cur!=NULL) {
                if (compare(cur,selected_node)<0)
                    selected_node=cur;
                cur=next_slist(slist,cur);
            }
        } else {
            // Search for max
            while(cur!=NULL) {
                if (compare(cur,selected_node)>0)
                    selected_node=cur;
                cur=next_slist(slist,cur);
            }
        }

        remove_slist(slist,selected_node);
        insert_after_slist(slist,prev_selected,selected_node);
        prev_selected=selected_node;
        cur=next_slist(slist,selected_node);
    }
}