#ifndef DEVICE_H
#define DEVICE_H

#include <types.h>
#include <list.h>

#define CLASS_IDE   0x0101

struct driver;
struct device;

struct driver {
    char    *name;
    
    void (*probe) (struct device *);
};

struct device {
    struct device    *parent;
    struct list_head *children;
    struct driver    *driver;
    
    void*           driver_data;
    
    int (*read) (struct device *, byte_t *, int);
};

struct pci_device_data {
    unsigned short  vendor_id;
    unsigned short  device_id;
    unsigned char   bus;
    unsigned char   device;
    unsigned char   function;
    unsigned char   base_class;
    unsigned char   sub_class;
};

struct driver *find_driver(const char *name);

#endif
