
struct list_head {
    struct list_head *next;    //下一个
    struct list_head *prev;    //前一个
};

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define container_of(ptr, type, member) ({ \
    (type *)((char  *)ptr - offsetof(type,member)); })

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_get_entry(pos_type,pos, head, member) \
    pos = list_entry((head)->next, pos_type, member)

#define list_for_each_entry_safe(pos, n, head, type, member) \
    for (pos = list_entry((head)->next, type, member), \
              n = list_entry(pos->member.next, type, member); \
         &pos->member != (head); \
         pos = n, n = list_entry(n->member.next, type, member))

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)


#define INIT_LIST_HEAD(ptr) \
do { \
    (ptr)->next = (ptr);\
    (ptr)->prev = (ptr); \
} while (0)

void _list_add(struct list_head * _new, struct list_head * prev, struct list_head * next);
void list_add_tail(struct list_head *_new, struct list_head *head);
int list_empty(struct list_head *head);
void list_add(struct list_head *_new, struct list_head *head);
void _list_del(struct list_head * prev, struct list_head * next);
void list_del(struct list_head *entry);
