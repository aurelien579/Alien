/*******************************************************************************
 * SOURCE NAME  : device.h
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Public interface of the device system
 ******************************************************************************/

#ifndef DEVICE_DEVICE_H
#define DEVIce_DEVICE_H

#include <stdint.h>

#define DEVICES_MAX         10
#define DEVICE_NAME_MAX     20

#define OK      1
#define ERROR   2

struct device;

typedef uint32_t result_t;

enum device_type
{
    DEVICE_STREAM,
    DEVICE_RANDOM
};

struct device
{
    enum device_type type;
    char name[DEVICE_NAME_MAX];
    void *p;
    
    result_t (*read)    (struct device *, uint32_t *, uint8_t *);
    result_t (*write)   (struct device *, uint32_t *, uint8_t *);
    result_t (*seek)    (struct device *, uint32_t);
};

result_t device_register(const struct device *dev);


struct device *device_find(const char *name);


result_t device_stream_read(struct device *dev,
                            uint32_t *size,
                            uint8_t *out);


result_t device_stream_write(struct device *dev,
                             uint32_t *size,
                             uint8_t *in);


result_t device_random_read(struct device *dev,
                            uint32_t addr,
                            uint32_t *size,
                            uint8_t *out);


result_t device_random_write(struct device *dev,
                             uint32_t addr,
                             uint32_t *size,
                             uint8_t *in);

#endif
