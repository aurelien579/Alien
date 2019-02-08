/*******************************************************************************
 * SOURCE NAME  : device.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Implements the device management system
 ******************************************************************************/

#include <kernel/device/device.h>
#include <stdio.h>
#include <string.h>



/*******************************************************************************
 *                          PRIVATE VARIABLES
 ******************************************************************************/

static struct device devices[DEVICES_MAX];
static uint32_t devices_count = 0;



/*******************************************************************************
 *                          PUBLIC FUNCTIONS
 ******************************************************************************/

struct device *device_find(const char *name)
{
    for (uint32_t i = 0; i < devices_count; i++) {
        if (!strcmp(devices[i].name, name)) {
            return &devices[i];
        }
    }
    
    return (struct device *) 0;
}

result_t device_register(const struct device *dev)
{
    if (devices_count >= DEVICES_MAX || !dev) return ERROR;
    
    devices[devices_count++] = *dev;

    printf("[DEV] %s registered\n", dev->name);
    
    return OK;
}

result_t device_stream_read(struct device *d, uint32_t *size, uint8_t *out)
{
    if (!d) return ERROR;
    if (!d->read) return ERROR;
    
    return d->read(d, size, out);
}

result_t device_stream_write(struct device *d, uint32_t *size, uint8_t *in)
{
    if (!d) return ERROR;
    if (!d->write) return ERROR;
    
    return d->write(d, size, in);
}

result_t
device_random_read(struct device *d, uint32_t addr, uint32_t *sz, uint8_t *out)
{
    if (!d) return ERROR;
    if (!d->read || !d->seek) return ERROR;
    
    d->seek(d, addr);    
    return d->read(d, sz, out);
}

result_t
device_random_write(struct device *d, uint32_t addr, uint32_t *sz, uint8_t *in)
{
    if (!d) return ERROR;
    if (!d->read || !d->seek) return ERROR;
    
    d->seek(d, addr); 
    return d->write(d, sz, in);
}
