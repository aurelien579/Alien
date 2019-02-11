#include <kernel/memory/heap.h>
#include <kernel/memory/paging.h>

#include <stdio.h>

struct header
{
    uint8_t used;
    uint32_t size;
} __attribute__((packed));

struct heap
{
    uint32_t size;
    struct header *head;
};

struct heap heap = {
    .size = 0,
    .head = (struct header *) 0
};

static inline uint32_t
first_byte_after_heap()
{
    return ((uint32_t) heap.head) + heap.size;
}

static inline uint8_t
has_next(struct header *h)
{
    uint32_t byte_after = ((uint32_t) h) + h->size;
    if (byte_after >= first_byte_after_heap()) {
        return 0;
    }

    if (first_byte_after_heap() - byte_after <= sizeof(struct header)) {
        return 0;
    }

    return 1;
}

static inline struct header *
next(struct header *h)
{
    return (struct header *) (((uint8_t *) h) + h->size);
}

static inline void *
data(struct header *h)
{
    return (void *) (((uint32_t) h) + sizeof(struct header));
}

static inline struct header *
header_from_data(void *data)
{
    if ((uint32_t) data <= sizeof(struct header)) {
        return (struct header *) 0;
    }

    return (struct header *) (((uint32_t) data) - sizeof(struct header));
}

static struct header *
find_first_free(uint32_t size)
{
    struct header *header = heap.head;

    while (header->used || header->size < size) {
        if (!has_next(header)) {
            return (struct header *) 0;
        }

        header = next(header);
    }

    return header;
}

static inline void
resize(struct header *h, uint32_t size)
{
    if (h->size <= size) {
        return;
    }

    uint32_t old_size = h->size;
    h->size = size;

    if (has_next(h)) {
        struct header *new = next(h);
        new->size = old_size - size - sizeof(struct header);
        new->used = 0;

        printf("New header size: 0x%x\n", new->size);
    } else {
        h->size = old_size;
    }
}

void heap_install()
{
    uint32_t page = alloc_kpage();
    if (page == 0) {
        printf("[HEAP] [ERROR] Can't allocate page\n");
        return;
    }

    heap.size = PAGE_SIZE;
    heap.head = (struct header *) page;
    
    heap.head->used = 0;
    heap.head->size = PAGE_SIZE;

    printf("0x%x\n", find_first_free(PAGE_SIZE));
}

void *kmalloc(uint32_t size)
{
    if (size == 0) {
        return (void *) 0;
    }

    struct header *h = find_first_free(size);
    if (h->size > size) {
        resize(h, size);
    }

    h->used = 1;
    return data(h);
}

void kfree(void *ptr)
{
    if ((uint32_t) ptr == 0) {
        return;
    }

    struct header *h = header_from_data(ptr);
    if (h == (struct header *) 0) {
        return;
    }

    h->used = 0;
}