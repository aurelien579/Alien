#ifndef PAGE_ENTRY_H
#define PAGE_ENTRY_H

#include <types.h>
#include <alien/memory/paging.h>

#define PAGE_ENTRY_PRESENT  (1)
#define PAGE_ENTRY_RW       (2)
#define PAGE_ENTRY_USER     (4)


/* ===============================  Setters  =============================== */
void            page_entry_unset_flag   (page_entry_t *entry, u8 flag);
void            page_entry_set_flag     (page_entry_t *entry, u8 flag);                                         
void            page_entry_set_base     (page_entry_t *entry, ppage_t base);


/* ===============================  Getters  =============================== */
ppage_t         page_entry_get_base     (const page_entry_t *entry);
bool            page_entry_is_present   (const page_entry_t *entry);
bool            page_entry_is_rw        (const page_entry_t *entry);
bool            page_entry_is_user      (const page_entry_t *entry);


/* =============================  Constructor  ============================= */
page_entry_t    page_entry_new          (u32 base, u8 flags);

#endif