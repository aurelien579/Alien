#include <list.h>

static struct list_head *
list_new_element(void *data)
{
    struct list_head *el = (struct list_head *) kmalloc(sizeof(struct list_head));
    el->data = data;
    el->next = 0;
    return el;
}

struct list_head *
list_new()
{
    return (struct list_head *) 0;
}

void
list_add(struct list_head **l, void *data)
{
    struct list_head *el = list_new_element(data);
    el->next = *l;
    *l = el;
}
