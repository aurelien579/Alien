/*******************************************************************************
 * SOURCE NAME  : tests/tests.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Common tests functions
 ******************************************************************************/

#include <kernel/tests.h>
#include <stdio.h>

typedef int (*test_func_t) ();

struct test_context
{
    int total;
    int success;
};

static void run_test(struct test_context *ctx, test_func_t test, const char *name)
{
    ctx->total++;

    printf("[TEST] Running %s...\n", name);
    if (!test()) {
        printf("[TEST] %s failed\n", name);
    } else {
        ctx->success++;
    }
}

void test_all()
{
    struct test_context ctx = {
        .total = 0,
        .success = 0
    };

    run_test(&ctx, test_ata_read, "test_ata_read");
    run_test(&ctx, test_heap, "test_heap");
    run_test(&ctx, test_string, "test_string");
    run_test(&ctx, test_paging, "test_paging");

    printf("[TEST] %d/%d success\n", ctx.success, ctx.total);
}