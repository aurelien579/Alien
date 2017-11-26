#include <alien/kernel.h>

#define ATA_DEVICE_COUNT 		4

#define ATA_DATA_PORT			0
#define ATA_FEAT_PORT			1
#define ATA_SECTOR_COUNT_PORT	2
#define ATA_LBALO_PORT			3
#define ATA_LBAMID_PORT			4
#define ATA_LBAHI_PORT			5
#define ATA_DRIVE_PORT			6
#define ATA_COMMAND_PORT		7

#define ATA_MASTER				0xA0

/* Bit 1 set for PI devices */
#define ATA_TYPE_UNKNOWN    0
#define ATA_TYPE_PATAPI     1
#define ATA_TYPE_SATAPI     3
#define ATA_TYPE_PATA       4
#define ATA_TYPE_SATA       6

struct ata_device
{
	u8 type;
	u16 base_port;
	u16 control_port;
	u8 slave_bit;
    u8 lba48_support;
    u64 lba48_total;
    u32 lba28_total;
} __attribute__((packed));

struct ata_device devices[ATA_DEVICE_COUNT] =
{
	{ ATA_TYPE_UNKNOWN, 0x1F0, 0x3F6, 0, 0, 0, 0 },
	{ ATA_TYPE_UNKNOWN, 0x1F0, 0x3F6, 1 << 4, 0, 0, 0 },
	{ ATA_TYPE_UNKNOWN, 0x170, 0x376, 0, 0, 0, 0 },
	{ ATA_TYPE_UNKNOWN, 0x170, 0x376, 1 << 4, 0, 0, 0 }
};

extern void iowait(void);
extern int ata_send_identify(struct ata_device *dev, u16 *buffer);

static void
ata_detect(struct ata_device *device)
{
	u8 status, sector_mid, sector_hi;
	
	outb(device->control_port, 4);
    outb(device->control_port, 0);
    outb(device->base_port + ATA_DRIVE_PORT,
		 ATA_MASTER | device->slave_bit);
    
    for (int i = 0; i < 5; i++)
		status = inb(device->base_port + ATA_COMMAND_PORT);
	
	if (status & 1 || !(status & (1 << 6))) {
		return;
	}
	
	sector_mid = inb(device->base_port + ATA_LBAMID_PORT);
	sector_hi = inb(device->base_port + ATA_LBAHI_PORT);
	
	if (sector_mid == 0x14 && sector_hi == 0xEB) {
		device->type = ATA_TYPE_PATAPI;
	} else if (sector_mid == 0x69 && sector_hi == 0x96) {
		device->type = ATA_TYPE_SATAPI;
	} else if (sector_mid == 0 && sector_hi == 0) {
		device->type = ATA_TYPE_PATA;
	} else if (sector_mid == 0x3C && sector_hi == 0xC3) {
		device->type = ATA_TYPE_SATA;
	}
}


static void
ata_identify(struct ata_device *dev)
{
    if (dev->type == ATA_TYPE_UNKNOWN)
        return;
    
    u16 buffer[256];
    if (ata_send_identify(dev, buffer) < 0) {
        kprintf("[ERROR] ata_send_identify\n");
        return;
    }
    
    dev->lba48_support = (buffer[83] & (1 << 10)) >> 10;
    dev->lba48_total = *((u64*)(&buffer[100]));
    
    dev->lba28_total = *((u32*)(&buffer[60]));
}

static i32
atapi_read(struct ata_device *dev, u32 lba, u8 *buffer, u16 maxlen)
{
    u8 read_cmd[12] = { 0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    u8 status;
    i32 size;
    
    outb(dev->base_port + ATA_DRIVE_PORT, dev->slave_bit);
    iowait();
    iowait();
    iowait();
    iowait();
        
    outb(dev->base_port + ATA_FEAT_PORT, 0);
    outb(dev->base_port + ATA_LBAMID_PORT, maxlen & 0xFF);
    outb(dev->base_port + ATA_LBAHI_PORT, maxlen >> 8);
    outb(dev->base_port + ATA_COMMAND_PORT, 0xA0);
    
    while ((status = inb(dev->base_port + ATA_COMMAND_PORT)) & 0x80)
        ;
    
    while (!((status = inb(dev->base_port + ATA_COMMAND_PORT)) & 0x8)
        && !(status & 0x1))
        ;
    
    if (status & 0x1) {
        return -1;
    }
    
    read_cmd[9] = 1;
    read_cmd[2] = (lba >> 0x18) & 0xFF;
    read_cmd[3] = (lba >> 0x10) & 0xFF;
    read_cmd[4] = (lba >> 0x08) & 0xFF;
    read_cmd[5] = (lba >> 0x00) & 0xFF;
    
    outsw(dev->base_port, (u16 *) read_cmd, 6);
    
    size = (((u32) inb(dev->base_port + ATA_LBAHI_PORT)) << 8) |
           ((u32) inb(dev->base_port + ATA_LBAMID_PORT));
    
    insw(dev->base_port, (u16 *) buffer, size / 2);
    
    while ((status = inb (dev->base_port + ATA_COMMAND_PORT)) & 0x88)
        ;
    
    return size;
}

void
ata_init()
{
    for (int i = 1; i < ATA_DEVICE_COUNT; i++) {
        ata_detect(&devices[i]);
        ata_identify(&devices[i]);
    }

    struct ata_device *dev = &devices[0];
    
    u8 buffer[2048];
    atapi_read(dev, 0x2f, buffer, 2048);
    kprintf("buffer : 0x%x\n", buffer);
    while(1);
}
