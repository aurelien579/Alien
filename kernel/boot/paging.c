#include <kernel/paging.h>
#include <kernel/kernel.h>
#include <kernel/io.h>

struct pt_entry
{

} __attribute__((packed));

struct pd_entry
{

} __attribute__((packed));

struct pd
{
    struct pd_entry entries[1024];
} __attribute__((packed));

struct pt
{
    struct pt_entry entries[1024];
} __attribute__((packed));

static struct pd kpd;

void init_paging()
{
    int phys_free_start = ((kernel_end & 0xFFFFF000) + 0x1000) - kernel_virtual_base();
    printf("paging_start: 0x%x\n", phys_free_start);
}
