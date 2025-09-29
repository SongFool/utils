#include "list.h"
#include "stdio.h"

/*
    A <-> B <-> C <-> A
    _new = D
    prev = C
    next = A
    A前一个指向新的
    C下一个指向新的
*/
void _list_add(struct list_head * _new, struct list_head * prev, struct list_head * next)
{
    next->prev = _new;   // A前一个指向新的
    _new->next = next;
    _new->prev = prev;
    prev->next = _new;
}
/*

插入node1：head <---> node1 <---> head

*/
void list_add_tail(struct list_head *_new, struct list_head *head)
{
    _list_add(_new,head->prev,head);
}

int list_empty(struct list_head *head)
{
    return head->next == head;
}

void list_add(struct list_head *_new, struct list_head *head)
{

    _list_add(_new,head,head->next);

}

void _list_del(struct list_head * prev, struct list_head * next)
{
    prev->next = next;
    next->prev = prev;
}

void list_del(struct list_head *entry)
{
    _list_del(entry->prev,entry->next);
    entry->prev = NULL;
    entry->next = NULL;
}
