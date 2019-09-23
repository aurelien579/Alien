/*******************************************************************************
 * SOURCE NAME  : heap.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provides kmalloc/kfree
 ******************************************************************************/

#include <kernel/memory/heap.h>
#include <kernel/memory/paging.h>

#include <math.h>
#include <stdio.h>


typedef struct heap_block
{
    uint8_t             used;
    uint32_t            size;
    struct heap_block  *next;
} __attribute__((packed)) heap_block_t;

typedef struct heap
{
    heap_block_t   *head;
    uint32_t        size;
} heap_t;

static heap_t heap;

static inline void *
block_get_base(heap_block_t *block)
{
    return (void *) (((uint32_t) block) + sizeof(heap_block_t));
}

static inline heap_block_t *
block_from_base(void *base)
{
    return (heap_block_t *) (((uint32_t) base) - sizeof(heap_block_t));
}

static inline heap_block_t *
heap_get_last()
{
    heap_block_t *current = heap.head;

    while (current->next) {
        current = current->next;
    }

    return current;
}

/**
 * DESCRIPTION: Find a free block from the heap
 * IN: 
 *      - size: size of the memory block to allocate
 *      - heap (global): coherent heap structure
 * OUT:
 *      - ret: free block found
 */
static heap_block_t *
heap_find_free(uint32_t size)
{
    heap_block_t *current = heap.head;

    while (current) {
        if (!current->used && current->size >= size) {
            return current;
        }

        current = current->next;
    }

    return (heap_block_t *) 0;
}

/**
 * DESCRIPTION: Mark a new block of memory as used and make the heap coherent
 * IN: 
 *      - size: size of the memory block to allocate
 *      - block: a free block of memory from the heap
 *      - heap (global): coherent heap structure
 * OUT:
 *      - ret: memory address of the block's base
 * MODIFY:
 *      - heap (global): stays coherent
 */
static void *
heap_allocate_block(heap_block_t *block, uint32_t size)
{
    block->used = 1;

    /* If avaiable memory inside block is bigger than the size of a new block.
       If there is enough space to put another block after this one. */
    if (block->size > size + sizeof(heap_block_t)) {
        uint32_t free_size = block->size - size - sizeof(heap_block_t);

        block->size = size;

        /* Insert new block */
        heap_block_t *new_block = (heap_block_t *) (((uint32_t) block_get_base(block)) + size);
        new_block->used = 0;
        new_block->size = free_size;
        new_block->next = block->next;
        block->next = new_block;
    }

    return block_get_base(block);
}

/**
 * DESCRIPTION: Expand the heap so that it can contains 'size' available memory
 * IN: 
 *      - size: size of the memory block to allocate
 *      - heap (global): coherent heap structure with 'free_size < size'
 * OUT:
 *      - ret: memory address of the block or null if an error occured
 * MODIFY:
 *      - heap (global): coherent with 'free_size >= size'
 */
static void
heap_expand(uint32_t size)
{
    heap_block_t *last = heap_get_last();
    uint32_t required_size = 0;

    /* Calculate the number of bytes to add. */
    if (last->used) {
        required_size = size;
    } else {
        if (last->size >= size) {
            /* No need to expand. */
            return;
        }

        required_size = size - last->size;
    }

    /* We add sizeof(heap_block_t) in case we need to add a block */
    uint32_t required_pages = ((required_size + sizeof(heap_block_t)) / PAGE_SIZE) + 1;

    uint32_t byte_after_last = ((uint32_t) block_get_base(last)) + last->size;    
    uint32_t page = alloc_continuous_pages(required_pages);

    if (page == byte_after_last) {
        /* Continous area */
        last->size += required_pages * PAGE_SIZE;
    } else {
        heap_block_t *new_block = (heap_block_t *) page;
        new_block->used = 0;
        new_block->next = 0;
        new_block->size = (required_pages * PAGE_SIZE) - sizeof(heap_block_t);
        last->next = new_block;
        last = new_block;
    }
}

/**
 * DESCRIPTION: Allocate a new block of memory of size 'size'
 * IN: 
 *      - size: size of the memory block to allocate
 *      - heap (global): coherent heap structure
 * OUT:
 *      - ret: memory address of the block or null if an error occured
 * MODIFY:
 *      - heap (global): stays coherent
 */
void *
kmalloc(uint32_t size)
{
    //printf("[HEAP] kmalloc(%d)\n", size);
    heap_block_t *block = heap_find_free(size);


    if (block == 0) {
        heap_expand(size);

        printf("[HEAP] Expanded\n");

        block = heap_find_free(size);

        printf("[HEAP] New free: 0x%x, size: %d, next: 0x%x, used: %d\n", block, block->size, block->next, block->used);
        if (block == 0) {
            /* Expansion didn't worked */
            return (void *) 0;
        }
    }

    return heap_allocate_block(block, size);
}

void
kfree(void *ptr)
{
    if (!ptr) {
        return;
    }

    heap_block_t *block = block_from_base(ptr);
    
    block->used = 0;

    /* Merge with next block if present and not used */
    heap_block_t *next = block->next;
    if (next) {
        if (!next->used) {
            block->size += next->size + sizeof(heap_block_t);
            block->next = next->next;
        }
    }
}

void
heap_install()
{
    uint32_t page = alloc_kpage();
    if (page == 0) {
        printf("[HEAP] [ERROR] Can't allocate page\n");
        return;
    }

    heap.size = PAGE_SIZE;
    heap.head = (heap_block_t *) page;
    
    heap.head->used = 0;
    heap.head->next = 0;
    heap.head->size = PAGE_SIZE - sizeof(heap_block_t);

    printf("0x%x\n", heap_find_free(PAGE_SIZE - sizeof(heap_block_t)));
}
