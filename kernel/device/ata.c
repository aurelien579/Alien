/*******************************************************************************
 * SOURCE NAME  : ata.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : A basic ATA driver only capable of identifying a device.
 ******************************************************************************/

#include <kernel/device/ata.h>
#include <kernel/device/device.h>
#include <kernel/io.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define ATA_DEV(dev)            ((struct ata_device *) dev->p)



/*******************************************************************************
 *                          PRIVATE CONSTANTS
 ******************************************************************************/

#define ATA_DATA_PORT			0
#define ATA_FEAT_PORT			1
#define ATA_SECTOR_COUNT_PORT	2
#define ATA_LBALO_PORT			3
#define ATA_LBAMID_PORT			4
#define ATA_LBAHI_PORT			5
#define ATA_DRIVE_PORT			6
#define ATA_COMMAND_PORT	    7

#define ATA_ISBSY(status)       (status & (1 << 7))
#define ATA_ISRDY(status)       (status & (1 << 6))
#define ATA_ISDRQ(status)       (status & (1 << 3))
#define ATA_ISERR(status)       (status & 1)

#define ATA_CMD_IDENTIFY        0xEC
#define ATA_CMD_PACKET_IDENTIFY 0xA1

//#define ATA_LOG 1


/*******************************************************************************
 *                          PRIVATE STRUCTURES
 ******************************************************************************/

enum ata_device_type
{
    ATA_TYPE_UNKNOWN,
    ATA_TYPE_PATAPI,
    ATA_TYPE_SATAPI,
    ATA_TYPE_PATA,
    ATA_TYPE_SATA
};

struct ata_device
{
    enum ata_device_type type;
    uint16_t base_port;
    uint16_t control_port;
    uint8_t slave_bit;

    uint8_t lba48_support;
    uint64_t lba48_total;
    uint32_t lba28_total;

    uint32_t pos;
};

static struct ata_device ata_devices[4];



/*******************************************************************************
 *                          PRIVATE FUNCTIONS
 ******************************************************************************/


/*
 *  Simple inline functions
 */

static inline uint8_t
ata_device_is_packet(struct ata_device *d)
{
    return d->type == ATA_TYPE_PATAPI || d->type == ATA_TYPE_SATAPI;
}

/* Wait n*100 nano seconds */
static inline void
ata_wait(int n)
{
    for (int i = 0; i < n; i++) {
        inb(0x3F6);
    }
}

static inline uint8_t
ata_read_status(struct ata_device *d)
{
    return inb(d->base_port + ATA_COMMAND_PORT);
}

static inline uint8_t
ata_wait_bsy_clear(struct ata_device *d)
{
    uint8_t status;

    do {
        status = ata_read_status(d);
    } while (ATA_ISBSY(status));

    return status;
}

static inline uint8_t
ata_wait_drq_set(struct ata_device *d)
{
    uint8_t status;

    do {
        status = ata_read_status(d);
    } while (!ATA_ISDRQ(status) && !ATA_ISERR(status));

    return status;
}

static inline void
ata_software_reset(struct ata_device *d)
{
    outb(d->control_port, 4);
    outb(d->control_port, 0);
}

static inline void
ata_select_drive(struct ata_device *d)
{
    outb(d->base_port + ATA_DRIVE_PORT, 0xA0 | d->slave_bit);
    ata_wait(5);
}

static inline uint16_t
ata_pio_read16(struct ata_device *d)
{
    return inw(d->base_port + ATA_DATA_PORT);
}

static void
ata_log_device(struct ata_device *d)
{
    printf("{ base: 0x%x, control: 0x%x, slave: %d }\n",
           d->base_port, d->control_port, d->slave_bit);
}



/*
 * Low level functions
 */
static result_t atapi_read_sector(struct ata_device *dev,
                                 uint32_t lba,
                                 uint8_t *buffer)
{
    uint8_t status;
    int32_t size = 0;

    //ata_log_device(dev);

//    ata_select_drive(dev);
    //printf("Status: 0x%x\n", inb(dev->base_port + ATA_COMMAND_PORT));

    //outb(dev->base_port + ATA_DRIVE_PORT, dev->slave_bit);
    //ata_wait(4);
    outb(dev->base_port + ATA_DRIVE_PORT, 0xE0 | (dev->slave_bit) | ((lba >> 24) & 0x0F));
    outb(dev->base_port + ATA_FEAT_PORT, 0);
    outb(dev->base_port + ATA_SECTOR_COUNT_PORT, 1);

    outb(dev->base_port + ATA_LBALO_PORT, lba);
    outb(dev->base_port + ATA_LBAMID_PORT, lba >> 8);
    outb(dev->base_port + ATA_LBAHI_PORT, lba >> 16);

    outb(dev->base_port + ATA_COMMAND_PORT, 0x20);

    /* Poll */
    //printf("Status: 0x%x\n", inb(dev->base_port + ATA_COMMAND_PORT));

    /* Wait for BSY bit to clear */
    while ((status = inb(dev->base_port + ATA_COMMAND_PORT)) & 0x80)
        ;

    //printf("[ATA] BSY clear 0x%x\n", status);

    while (!((status = inb(dev->base_port + ATA_COMMAND_PORT)) & 0x8)
        && !(status & 0x1))
        ;

    if (status & 0x1) {
        printf("[ATA] ERROR 0x%x\n0", status);
        return -1;
    }

    insw(dev->base_port, (uint16_t *) buffer, 256);

    while ((status = inb (dev->base_port + ATA_COMMAND_PORT)) & 0x88)
        ;

    return size;
}

#if 0
static result_t atapi_read_block_bis(struct ata_device *dev,
                                 uint32_t lba,
                                 uint8_t *buffer,
                                 uint32_t maxlen)
{
    uint8_t read_cmd[12] = { 0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t status;
    int32_t size;

    ata_log_device(dev);

    ata_select_drive(dev);

    outb(dev->base_port + ATA_DRIVE_PORT, dev->slave_bit);
    ata_wait(4);

    outb(dev->base_port + ATA_FEAT_PORT, 0);
    //outb(dev->base_port + ATA_LBAMID_PORT, maxlen & 0xFF);
    //outb(dev->base_port + ATA_LBAHI_PORT, maxlen >> 8);

    outb(dev->base_port + ATA_LBAMID_PORT, 0x00);
    outb(dev->base_port + ATA_LBAHI_PORT, 0x01);

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

    outsw(dev->base_port, (uint16_t *) read_cmd, 6);

    size = (((uint32_t) inb(dev->base_port + ATA_LBAHI_PORT)) << 8) |
           ((uint32_t) inb(dev->base_port + ATA_LBAMID_PORT));

    insw(dev->base_port, (uint16_t *) buffer, size / 2);

    while ((status = inb (dev->base_port + ATA_COMMAND_PORT)) & 0x88)
        ;

    return size;
}
#endif

/**
 * @brief Select the device provided then send the IDENTIFY command. If the
 *  device doesn't exist  or it's not ATA returns 0. If the device exists,
 *  reads 512 bytes from data block returned by the cmd.
 * @param d : the device to identify (base_port and control_port must be
 *  initialized)
 * @param buffer : output buffer of 256 words (512 bytes)
 * @return 0 if the device doesn't exist or it's not ATA, 1 else
 */
static uint8_t
ata_cmd_identify(struct ata_device *d, uint16_t *buffer)
{
    uint8_t status, lbahi, lbamid;

    //ata_select_drive(d);

    /* Send the IDENTIFY command */
    outb(d->base_port + ATA_SECTOR_COUNT_PORT, 0);
    outb(d->base_port + ATA_LBALO_PORT, 0);
    outb(d->base_port + ATA_LBAMID_PORT, 0);
    outb(d->base_port + ATA_LBAHI_PORT, 0);
    
    if (ata_device_is_packet(d)) {
        outb(d->base_port + ATA_COMMAND_PORT, ATA_CMD_PACKET_IDENTIFY);
    } else {
        outb(d->base_port + ATA_COMMAND_PORT, ATA_CMD_IDENTIFY);
    }

    status = ata_read_status(d);
    
    status = ata_wait_bsy_clear(d);
    if (status == 0) {
        return 0;
    }
    
    if (ATA_ISERR(status)) {
        printf("[ATA] Error after sending IDENTIFY\n", status);
        return 0;
    }

    /* Check that lbamid and lbahi are zero */
    lbamid = inb(d->base_port + ATA_LBAMID_PORT);
    lbahi = inb(d->base_port + ATA_LBAHI_PORT);

    if (lbamid != 0 || lbahi != 0) {
        printf("[ATA] No ATA device\n");
        return 0;
    }

    /* Wait for the data to be ready */
    status = ata_wait_drq_set(d);
    if (ATA_ISERR(status)) {
        printf("[ATA] Error\n");
        return 0;
    }

    /* Read the data */
    for (uint32_t i = 0; i < 256; i++) {
        buffer[i] = ata_pio_read16(d);
    }

    return 1;
}

static uint8_t
ata_detect(uint16_t base_port,
           uint16_t control_port,
           uint8_t  slave_bit,
           struct ata_device *device)
{
    uint8_t status, sector_mid, sector_hi;

    /* Fill the structure */
    device->base_port = base_port;
    device->control_port = control_port;
    device->slave_bit = slave_bit;

#ifdef ATA_LOG
    printf("[ATA] Identifying device ");
    ata_log_device(device);
#endif

    /* Reset the drive */
    ata_software_reset(device);
    status = ata_wait_bsy_clear(device);

    ata_select_drive(device);
    status = ata_wait_bsy_clear(device);

    if (!ATA_ISRDY(status)) {
#ifdef ATA_LOG
        printf("[ATA] No device ");
        ata_log_device(device);
#endif
        return 0;
    }

    /* Read informations to determine if the device supports packet interface */
    sector_mid = inb(base_port + ATA_LBAMID_PORT);
	sector_hi = inb(base_port + ATA_LBAHI_PORT);

	if (sector_mid == 0x14 && sector_hi == 0xEB) {
		device->type = ATA_TYPE_PATAPI;
	} else if (sector_mid == 0x69 && sector_hi == 0x96) {
		device->type = ATA_TYPE_SATAPI;
	} else if (sector_mid == 0 && sector_hi == 0) {
		device->type = ATA_TYPE_PATA;
	} else if (sector_mid == 0x3C && sector_hi == 0xC3) {
		device->type = ATA_TYPE_SATA;
    }

#ifdef ATA_LOG
    if (ata_device_is_packet(device)) {
        printf("[ATA] Packet device found.\n");
    }
#endif

    uint16_t buffer[256];
    if (!ata_cmd_identify(device, (uint16_t *) buffer)) {
        printf("[ATA] Error while identifying device\n");
        return 0;
    }


    /* Verify that the device has been correctly identified */
    if (ata_device_is_packet(device)) {
        if (!(buffer[0] & (1 << 15)) && !(buffer[0] & (1 << 14))) {
            printf("[ATA] Invalid ATAPI device.\n");
            return 0;
        }
    } else {
        if (buffer[0] & (1 << 15)) {
            printf("[ATA] Invalid ATA device.\n");
            return 0;
        }
    }

#ifdef ATA_LOG
    printf("[ATA] Identification successful\n");
#endif

    return 1;
}



/*
 * High level functions
 */

static result_t ata_read(struct device *device,
                         uint32_t *size,
                         uint8_t *out)
{
    uint32_t size_read = 0;
    uint8_t buffer[512];
    struct ata_device *ata_dev = ATA_DEV(device);

#ifdef ATA_LOG
    printf("[ATA] ata_read size: 0x%x\n", *size);
#endif

    while (size_read < *size) {
        /* Next sector to read */
        uint32_t sector = ata_dev->pos / 512;
        int res = atapi_read_sector(ata_dev, sector, buffer);
        if (res < 0) {
            *size = size_read;
            return -1;
        }

        /* Where should we start copying bytes from the sector to the output */
        uint32_t offset = ata_dev->pos - (sector * 512);

        /* Remaining size to read. Only copy remaining bytes if there is less
           than one sector to read */
        uint32_t remaining = (*size) - size_read;

#ifdef ATA_LOG
        printf("[ATA] Reading sector %d from 0x%x (remains 0x%x)\n", sector, offset, remaining);
#endif

        if (remaining >= 512) {
            memcpy(out + size_read, buffer + offset, 512);
            size_read += (512 - offset);
            ata_dev->pos += (512 - offset);
        } else {
            memcpy(out + size_read, buffer + offset, remaining);
            size_read += (remaining - offset);
            ata_dev->pos += (remaining - offset);
        }
    }

    *size = size_read;
    return OK;
}

static result_t ata_write(struct device *device,
                          uint32_t *size,
                          uint8_t *out)
{
    return ERROR;
}

static result_t ata_seek(struct device *device,
                         uint32_t pos)
{
    struct ata_device *ata_dev = (struct ata_device *) device->p;
    ata_dev->pos = pos;
    return OK;
}

static void
ata_register_device(const char *name,
                    struct ata_device *ata_dev)
{
    struct device dev;
    dev.type = DEVICE_RANDOM;
    strncpy(dev.name, name, DEVICE_NAME_MAX);
    dev.p = ata_dev;

    dev.read = ata_read;
    dev.write = ata_write;
    dev.seek = ata_seek;

    device_register(&dev);
}


/*******************************************************************************
 *                          PUBLIC FUNCTIONS
 ******************************************************************************/

void ata_install()
{
    if (ata_detect(0x1F0, 0x3F6, 0, &ata_devices[0])) {
        ata_register_device("ATA-0", &ata_devices[0]);
    }

    if (ata_detect(0x1F0, 0x3F6, 1 << 4, &ata_devices[1])) {
        ata_register_device("ATA-1", &ata_devices[1]);
    }

    if (ata_detect(0x170, 0x376, 0, &ata_devices[2])) {
        ata_register_device("ATA-2", &ata_devices[2]);
    }

    if (ata_detect(0x170, 0x376, 1 << 4, &ata_devices[3])) {
        ata_register_device("ATA-3", &ata_devices[3]);
    }
}
