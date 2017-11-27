#include <alien/pci.h>
#include <types.h>
#include <alien/kernel.h>
#include <alien/device.h>
#include <list.h>
#include <alien/ata.h>
#include <alien/string.h>

#define PCI_HEADER_IS_NORMAL(h)     (((h) & 0x0F) == 0x00)
#define PCI_HEADER_IS_PCI_PCI(h)    (((h) & 0x0F) == 0x01)
#define PCI_HEADER_IS_CARDBUS(h)    (((h) & 0x0F) == 0x02)
#define PCI_HEADER_HAS_MULTI(h)     ((h) & (0x80))


static void pci_probe(struct device *dev);

static struct list_head *drivers;
static struct driver pci_driver = { "pci", pci_probe };
static struct device pci_bus;


static struct driver *
load_driver(const char *name)
{    
    if (strcmp(name, "ata") == 0) {
        struct driver *driver = (struct driver *) kmalloc(sizeof(struct driver));

        driver->name = (char *) kmalloc(sizeof(char) * (strlen(name) + 1));
        strcpy(driver->name, name);
        driver->probe = ata_probe;
        list_add(&drivers, driver);
        
        return driver;
    }

    return (struct driver *) 0;
}

struct driver *
find_driver(const char *name)
{
    struct list_head *current = drivers;
    struct driver *driver;
    
    while (current != 0) {
        driver = (struct driver *) current->data;
        
        if (strcmp(driver->name, name) == 0) {
            return driver;
        }
        
        current = current->next;
    }
    
    return load_driver(name);
}

static u32
pci_read32(u8 bus, u8 slot, u8 function, u8 offset)
{
    u32 address;
    u32 lbus  = (u32) bus;
    u32 lslot = (u32) slot;
    u32 lfunc = (u32) function;
 
    address = (u32)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((u32)0x80000000));
    
    outl(0xCF8, address);
    
    return inl(0xCFC);
}

static u16
pci_read(u8 bus, u8 slot, u8 func, u8 offset)
{
    u32 address;
    u32 lbus  = (u32) bus;
    u32 lslot = (u32) slot;
    u32 lfunc = (u32) func;
    u32 tmp = 0;
 
    address = (u32)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset) | ((u32)0x80000000));
 
    outl(0xCF8, address);

    tmp = (u16) (inl(0xCFC) >> ((offset & 2) * 8));
    return tmp;
}

static inline u8
pci_read_header_type(u8 bus, u8 slot, u8 function)
{
    return pci_read(bus, slot, function, 0x0E) & 0xFF;
}

static void
probe_function(struct device *bus, u8 device, u8 function)
{
    u8 base_class;
    u8 sub_class;
    struct pci_device_data *bus_data = (struct pci_device_data *) bus->driver_data;

    if (pci_read(bus_data->bus, device, function, 0) == 0xFFFF)
        return;
    
    base_class = (u8) (pci_read(bus_data->bus, device, function, 10) >> 8);
    sub_class = (u8) pci_read(bus_data->bus, device, function, 10);
    
    struct device *dev = (struct device *) kmalloc(sizeof(struct device));
    dev->parent = bus;
    dev->children = 0;
    dev->read = 0;
    dev->driver = find_driver("pci");
    
    struct pci_device_data *data =
        (struct pci_device_data *) kmalloc(sizeof(struct pci_device_data));
    
    data->vendor_id = pci_read(bus_data->bus, device, function, 0);
    data->device_id = pci_read(bus_data->bus, device, function, 2);
    data->bus = bus_data->bus;
    data->device = device;
    data->function = function;
    data->base_class = base_class;
    data->sub_class = sub_class;
    
    list_add(&bus->children, dev);
    
    if (base_class == 0x01 && sub_class == 0x01) {
        struct driver *driver = find_driver("ata");
        if (!driver) {
            kprintf("No ata driver found  !\n");
        }
        driver->probe(dev);
    }
}

static void
pci_probe(struct device *dev)
{
    struct pci_device_data *data = (struct pci_device_data *) dev->driver_data;
    
    for (u8 device = 0; device < 32; device++) {
        if (PCI_HEADER_HAS_MULTI(pci_read(data->bus, device, 0, 0))) {
            for (u8 function = 0; function < 8; function++) {
                probe_function(dev, device, function);
            }
        } else {
            probe_function(dev, device, 0);        
        }
    }
}

void
pci_init()
{
    list_add(&drivers, &pci_driver);
    
    pci_bus.parent = 0;
    pci_bus.children = 0;
    pci_bus.read = 0;
    pci_bus.driver = &pci_driver;
    
    struct pci_device_data *data = (struct pci_device_data *)
        kmalloc(sizeof(struct pci_device_data));
    
    data->vendor_id = pci_read(0, 0, 0, 0);
    data->device_id = pci_read(0, 0, 0, 2);
    data->bus = 0;
    data->device = 0;
    data->function = 0;
    
    pci_bus.driver_data = data;
    
    kprintf("probing...\n");
    pci_driver.probe(&pci_bus);
}
