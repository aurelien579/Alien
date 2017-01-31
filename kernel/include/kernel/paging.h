#ifndef PAGING_H
#define PAGING_H

#include <types.h>
#include <kernel/kernel.h>

#define PE_PRESENT  1
#define PE_RW       2
#define PE_USER     4

#define PD_INDEX(virt) ((virt & 0xFFC00000) >> 22)
#define PT_INDEX(virt) ((virt & 0x003FF000) >> 12)

struct pt_entry
{
    u8 flags;
    u8 zero : 4;
    u32 base : 20;
} __attribute__((packed));

struct pd_entry
{
    u8 flags;
    u8 zero : 4;
    u32 base : 20;
} __attribute__((packed));

struct pd
{
    struct pd_entry entries[1024];
} __attribute__((packed));

struct pt
{
    struct pt_entry entries[1024];
} __attribute__((packed));

struct pd* kpd;
struct pd* current_pd;

static inline vaddr_t vaddr(paddr_t phys)
{
    return phys + kernel_info.kernel_vbase;
}

static inline paddr_t paddr(vaddr_t virt)
{
    return virt - kernel_info.kernel_vbase;
}

void init_paging(u32 kernel_end);
void map_page(struct pd* dir, u32 page, u32 virt, u8 flags);
void switch_page_dir(struct pd* dir);

#endif
