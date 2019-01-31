#include <stdint.h>
#include <kernel/device/device.h>

static uint8_t test_read_device(struct device *dev)
{
    uint32_t size = 512;
    uint8_t out[4096];
    memset(out, 0, sizeof(out));
    out[512] = 'c';

    strcpy(out, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab");

    device_random_read(dev, 3, &size, out);

    //printf("out: %x\n", out);
    //printf("DATA: %s\n", out);

    if (strcmp(out, "JOUR") != 0) {
        return 0;
    }

    if (out[512] != 'c') {
        return 0;
    }

    //printf("DATA 2: %s\n", &out[512]);

    return 1;
}

int test_ata_read()
{
    if (!test_read_device(device_find("ATA-3"))) {
        return 0;
    }

    if (!test_read_device(device_find("ATA-3"))) {
        return 0;
    }

    if (!test_read_device(device_find("ATA-3"))) {
        return 0;
    }

    return 1;
}