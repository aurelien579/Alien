#include <types.h>
#include <alien/boot/paging.h>

#define KHEAP_SIZE      1 * 1024 * 1024         // 1MB
#define KHEAP_PAGE_C    KHEAP_SIZE / PAGE_SIZE

struct kheap_header {
    struct kheap_header* prev;
    u32                  size;
    u8                   used;
} __attribute__((packed));

static struct kheap_header *kheap;

void
init_kheap()
{
    /*if ((kheap = (struct kheap_header*) kalloc_pages(KHEAP_PAGE_C)) == 0)
        panic("Can't init kheap, kalloc_pages error\n");
    */
    kheap->prev = (struct kheap_header*) 0;
    kheap->used = 0;
    kheap->size = KHEAP_SIZE;
}

static inline u8
isvalid(struct kheap_header *head, u32 size)
{
    return (head->used == 0) && (head->size >= size);
}

static inline u32
getbase(const struct kheap_header* head)
{
    return ((u32)head) + sizeof(struct kheap_header);
}

static inline struct kheap_header*
getnext(const struct kheap_header* head)
{
    return (struct kheap_header*) (getbase(head) + head->size);
}

static inline struct kheap_header*
gethead(void* p)
{
    return (struct kheap_header*) (p - sizeof(struct kheap_header));
}

void*
kmalloc(u32 size)
{
    struct kheap_header *head = kheap;
    u32                  saved_size;

    while (isvalid(head, size) == 0) {
        head = getnext(head);
        if ((u32)head >= ((u32)kheap) + KHEAP_SIZE) {
            return 0;
        }
    }

    if (size < head->size) {
        saved_size = head->size;
        head->size = size;

        getnext(head)->prev = head;
        getnext(head)->used = 0;
        getnext(head)->size = saved_size - size - sizeof(struct kheap_header);
    }

    head->used = 1;

    return (void*) getbase(head);
}

void
kfree(void *p)
{
    if (p == 0)
        return;

    struct kheap_header* head = gethead(p);
    struct kheap_header* next = getnext(head);

    head->used = 0;

    if (next->used == 0) {
        head->size += sizeof(struct kheap_header) + next->size;
    }

    if (head->prev->used == 0) {
        head->prev->size += sizeof(struct kheap_header) + head->size;
    }
}
