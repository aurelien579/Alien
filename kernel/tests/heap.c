#include <kernel/memory/heap.h>
#include <stdio.h>

int test_heap()
{
    void *p1, *p2, *p3;
    uint32_t a, b, c;

    p1 = kmalloc(4000);
    if (!p1) {
        printf("[TEST] [HEAP] kmalloc returned 0\n");
        return 0;
    }

    kfree(p1);
    p2 = kmalloc(4000);
    if (p2 != p1) {
        printf("[TEST] [HEAP] p1 != p2\n");
        return 0;
    }

    p3 = kmalloc(10);
    a = (uint32_t) p1;
    b = (uint32_t) p3;
    if (b - a != 4009) {
        printf("[TEST] [HEAP] b - c != 4009%d\n", b - a);
        return 0;
    }

    kfree(p3);
    kfree(p1);
    p1 = kmalloc(4010);
    if (p1 != p2) {
        printf("[TEST] [HEAP] Merge blocks didn't worked\n");
        return 0;
    }
    kfree(p1);

    return 1;
}
