#include "page_entry.h"


/* ===============================  Setters  =============================== */

void
page_entry_set_base(page_entry_t *entry, ppage_t base)
{
    entry->base = ((base) >> 12);
}

void
page_entry_set_flag(page_entry_t *entry, u8 flag)
{
    entry->flags |= flag;
}

void
page_entry_unset_flag(page_entry_t *entry, u8 flag)
{
    entry->flags &= ~flag;
}



/* ===============================  Getters  =============================== */

ppage_t
page_entry_get_base(const page_entry_t *entry)
{
    return (entry->base << 12);
}

bool
page_entry_is_present(const page_entry_t *entry)
{
    return (entry->flags & PAGE_ENTRY_PRESENT) == 1;
}

bool
page_entry_is_rw(const page_entry_t *entry)
{
    return (entry->flags & PAGE_ENTRY_RW) == 1;
}

bool
page_entry_is_user(const page_entry_t *entry)
{
    return (entry->flags & PAGE_ENTRY_USER) == 1;
}


/* =============================  Constructor  ============================= */

page_entry_t
page_entry_new(u32 base, u8 flags)
{
    page_entry_t e;
    
    page_entry_set_base(&e, base);
    page_entry_set_flag(&e, PAGE_ENTRY_PRESENT | flags);
    
    return e;
}