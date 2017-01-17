#include <kernel/heap.h>
#include <types.h>

struct heap_entry
{
    u32 size;
    u32 flags;
    struct heap_entry* next;
} __attribute__((packed));

static heap_entry *heap;
