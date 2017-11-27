#ifndef LIST_H
#define LIST_H

struct list_head {
    void *data;
    struct list_head *next;
};

struct list_head *list_new();
void list_add(struct list_head **l, void *data);

#endif