#include <kernel/core/mm.h>
#include <kernel/io.h>
#include <kernel/paging.h>

#define MM_BLOCK_USED 0x01
#define MM_BLOCK_LAST 0x80

#define MM_BLOCK_SIZE 0x1000 /* 4096 bytes */

struct mm_block
{
    u32 base; /* Physical base */
    u8 flags;
} __attribute__((packed));

#define align(a, b) \
    if (a % b != 0) \
        a += b - a % b;

#define mm_next_block(b) \
    ((struct mm_block*) (((u32)b) + sizeof (struct mm_block)))

#define mm_block_is_last(b) ((b->flags & MM_BLOCK_LAST) != 0)
#define mm_block_is_used(b) ((b->flags & MM_BLOCK_USED) != 0)

static struct mm_block *first_block;

/**
 * Initialize memory management from the given @mem_start of the given @len.
 * @param mem_len length of the total RAM
 * @return the beginning of the available memory
 */
u32 init_mm(u32 mem_len)
{
    extern u32 _kernel_end;

    /* Calculate the beginnig of free memory and it's length */
    u32 free_mem_start = paddr((u32) &_kernel_end);
    u32 n_blocks = mem_len / MM_BLOCK_SIZE;

    first_block = (struct mm_block*) vaddr(free_mem_start);

    /* Reserve memory for the blocks */
    free_mem_start += n_blocks * sizeof (struct mm_block);
    align (free_mem_start, 4096);
    printf("free_mem_start : %x\n", free_mem_start);

    /* Initialize the blocks */
    struct mm_block* block = first_block;

    for (u32 i = 0; i < n_blocks - 1; i++)
    {
        block->base = i * MM_BLOCK_SIZE;
        block->flags = 0;

        /* Mark the block as used if before kernel_end */
        if (block->base < free_mem_start)
            block->flags |= MM_BLOCK_USED;
        block = mm_next_block(block);
    }

    block->base = (n_blocks - 1) * MM_BLOCK_SIZE;
    block->flags = MM_BLOCK_LAST;

    return free_mem_start;
}

/**
 * Find a free block after @b To get the first free block,
 * use find_free(first_block);
 * Return 0 if their is no free block after @b
 */
static inline struct mm_block* find_free(struct mm_block* b)
{
    while(mm_block_is_used(b))
    {
        if (mm_block_is_last(b))
            return (struct mm_block*) 0;
        b = mm_next_block(b);
    }

    return b;
}

/**
 * Check whether or not the @n blocks after @b are free
 */
static inline u32 are_free(struct mm_block* b, u32 n)
{
    for (u32 i = 0; i < n; i++)
    {
        if (mm_block_is_used(b))
            return 0;
        b = mm_next_block(b);
    }

    return 1;
}

u32 used_memory()
{
    u32 ret = 0;
    struct mm_block* block = first_block;

    while (!mm_block_is_last(block))
    {
        if (mm_block_is_used(block))
            ret += MM_BLOCK_SIZE;
        block = mm_next_block(block);
    }

    return ret;
}

/**
 * Allocate @n consecutives memory blocks and return the PHYSICAL base of the
 * first block.
 */
paddr_t alloc_blocks(u32 n)
{
    struct mm_block* block = first_block;
    struct mm_block* temp;

    while (!are_free(block, n))
    {
        while (!mm_block_is_used(block))
            block = mm_next_block(block);

        block = find_free(block);
        if (block == 0)
            return 0;
    }

    temp = block;
    for (u32 i = 0; i < n; i++)
    {
        temp->flags |= MM_BLOCK_USED;
        temp = mm_next_block(temp);
    }

    return block->base;
}

/**
 * Free the @n blocks at pointing at the @base physical adress
 */
void free_blocks(u32 base, u32 n)
{
    struct mm_block* b = first_block;

    while (b->base != base)
    {
        b = mm_next_block(b);
        if (b == 0)
            return;
    }

    while (n > 0)
    {
        b->flags &= 0xFE;
        b = mm_next_block(b);
        n--;
    }
}
