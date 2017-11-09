#include <alien/memory/kmalloc.h>
#include <alien/memory/paging.h>
#include <alien/io.h>

#define FLAGS_USED 1
#define FLAGS_LAST 2

typedef struct kheap_block
{
	u32 size;
	u8 flags;
} __attribute__((packed)) kheap_block_t;

static kheap_block_t *heap = 0;

void
kmalloc_init()
{
	heap  = (kheap_block_t *) alloc_kpage();
	heap->flags = FLAGS_LAST;
	heap->size = 4096 - sizeof(kheap_block_t);
}

static inline u32
_block_base(const kheap_block_t *b)
{
	return (((u32)b) + sizeof(kheap_block_t));
}

static inline kheap_block_t *
_next(const kheap_block_t *b)
{
	return (kheap_block_t *) (((u32)b) + sizeof(kheap_block_t) + b->size);
}

static inline u8
_is_last(const kheap_block_t *b)
{
	return b->flags & FLAGS_LAST;
}

static inline u8
_is_used(const kheap_block_t *b)
{
	return b->flags & FLAGS_USED;
}

void *
kmalloc(unsigned int s)
{
	kheap_block_t *cur = heap;
	
	while (cur) {
		if (!_is_used(cur)) {
			i64 size_diff = (i64)cur->size - s;
			if (size_diff > sizeof(kheap_block_t)) {
				cur->size = s;
				kheap_block_t *new = _next(cur);
				new->size = size_diff - sizeof(kheap_block_t);
				new->flags = cur->flags;
				cur->flags &= ~FLAGS_LAST;
			}
			
			if (cur->size >= s) {
				cur->flags |= FLAGS_USED;				
				return (void *) _block_base(cur);
			}
		}
		
		if (_is_last(cur)) {
			cur = 0;
		} else {
			cur = _next(cur);
		}
	}
	
	return 0;
}
