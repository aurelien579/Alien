#include <kernel/tests.h>
#include <stdio.h>

void test_all()
{
    int total = 1;
    int success = 0;

    if (!test_ata_read()) {
        printf("[TEST] ATA read test failed\n");
    } else {
        success++;
    }

    printf("[TEST] %d/%d success\n", success, total);
}