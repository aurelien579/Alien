/*******************************************************************************
 * SOURCE NAME  : tests/paging.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Tests for the paging module
 ******************************************************************************/

#include <kernel/memory/paging.h>

int test_paging()
{
    uint32_t pages[8000];

    for (int i = 0; i < 10; i++) {
        pages[i] = alloc_kpage();
    }

    uint32_t oldpage = pages[5];
    free_page(oldpage);
    uint32_t newpage = alloc_kpage();

    if (oldpage != newpage) {
        return 0;
    }

    oldpage = pages[0];
    for (int i = 0; i < 10; i++) {
        uint8_t *ptr = pages[i];
        memset(ptr, 0, 4096);
        free_page(pages[i]);
    }

    newpage = alloc_kpage();
    if (oldpage != newpage) {
        return 0;
    }

    free_page(newpage);

    for (int i = 0; i < 7500; i++) {
        pages[i] = alloc_kpage();
    }

    for (int i = 0; i < 7500; i++) {
        free_page(pages[i]);
    }

    return 1;
}