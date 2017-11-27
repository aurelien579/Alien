#include "iso9660.h"
#include <alien/ata.h>

void
iso9660_init(const struct ata_device *dev)
{
    u8 buffer[2048];
    atapi_read(dev, 0x2f, buffer, 2048);
    
    struct primary_descriptor *desc = buffer;
    
}
