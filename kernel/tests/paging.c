#include <kernel/memory/paging.h>

void test_paging()
{
    uint32_t pages[8000];

    printf("===== test_paging() =====\n");

    for (int i = 0; i < 10; i++) {
        pages[i] = alloc_kpage();
    }

    uint32_t oldpage = pages[5];
    free_page(oldpage);
    uint32_t newpage = alloc_kpage();

    if (oldpage != newpage) {
        printf("Fail\n");
        return;
    }

    oldpage = pages[0];
    for (int i = 0; i < 10; i++) {
        uint8_t *ptr = pages[i];
        memset(ptr, 0, 4096);
        free_page(pages[i]);
    }

    newpage = alloc_kpage();
    if (oldpage != newpage) {
        printf("Fail\n");
        return;
    }

    free_page(newpage);

    for (int i = 0; i < 7500; i++) {
        pages[i] = alloc_kpage();
        if (pages[i] == 0) {
            printf("null page %d\n", i);
        }
    }

    printf("Success\n");
}