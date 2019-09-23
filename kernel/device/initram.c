/*******************************************************************************
 * SOURCE NAME  : initram.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : A basic initram driver only capable of identifying a device.
 ******************************************************************************/

#include <kernel/device/device.h>
#include <math.h>
#include <string.h>

struct initram_dev
{
    uint8_t *ptr;
    uint32_t length;
    uint32_t pos;
};



/*******************************************************************************
 *                          PRIVATE VARIABLES
 ******************************************************************************/

static const char *dev_name = "initram";
static struct initram_dev dev_storage;



/*******************************************************************************
 *                          PRIVATE FUNCTIONS
 ******************************************************************************/

static result_t
initram_write(struct device *device, uint32_t *size, uint8_t *out)
{
    struct initram_dev *initram_dev = (struct initram_dev *) device->p;

    if (initram_dev == 0 || device == 0 || size == 0 || out == 0) {
        return ERROR;
    }

    /* Compute writable size */
    uint32_t write_size = min(*size, initram_dev->length - initram_dev->pos - 1);
    
    /* Write the data and output read size */
    memcpy(out, &initram_dev->ptr[initram_dev->pos], write_size);
    *size = write_size;

    /* Update position on ramdisk */
    initram_dev->pos += write_size;

    return OK;
}

static result_t
initram_read(struct device *device, uint32_t *size, uint8_t *in)
{
    struct initram_dev *initram_dev = (struct initram_dev *) device->p;

    if (initram_dev == 0 || device == 0 || size == 0 || in == 0) {
        return ERROR;
    }
    
    /* Compute readable size */
    uint32_t read_size = min(*size, initram_dev->length - initram_dev->pos - 1);

    /* Write the data and output read size */
    memcpy(&initram_dev->ptr[initram_dev->pos], in, read_size);
    *size = read_size;

    /* Update position on ramdisk */
    initram_dev->pos += read_size;

    printf("OK %d\n\n", *size);
    return OK;
}

static result_t
initram_seek(struct device *device, uint32_t pos)
{
    struct initram_dev *initram_dev = (struct initram_dev *) device->p;
    
    if (initram_dev == 0) {
        return ERROR;
    }

    if (pos >= initram_dev->length) {
        return ERROR;
    }

    initram_dev->pos = pos;

    return OK;
}



/*******************************************************************************
 *                          PUBLIC FUNCTIONS
 ******************************************************************************/

void
initram_install(uint32_t base, uint32_t length)
{
    struct device dev;

    dev.type = DEVICE_RANDOM;
    strncpy(dev.name, dev_name, strlen(dev_name));
    dev.read = initram_read;
    dev.write = initram_write;
    dev.seek = initram_seek;
    dev.p = &dev_storage;

    dev_storage.ptr = (uint8_t *) base;
    dev_storage.pos = 0;
    dev_storage.length = length;

    device_register(&dev);
}
