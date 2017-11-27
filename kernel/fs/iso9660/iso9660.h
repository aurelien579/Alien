#ifndef ISO_9660_H
#define ISO_9660_H

#include <types.h>

struct volume_descriptor {
    i8      type;
    char    id[5];
    i8      version;
};

struct boot_record {
    i8      type;
    char    id[5];
    i8      version;
    char    boot_system_id[32];
    char    boot_id[32];
};

struct el_torito_boot_record {
    i8      type;
    char    id[5];
    i8      version;
    char    boot_system_id[32];
    char    boot_id[32];
    u32     boot_catalog_lba;    
};

struct primary_descriptor {
    i8      type;
    char    id[5];
    i8      version;
    u8      _unused0;
    char    system_id[32];
    char    volume_id[32];
    u8      _unused1[8];
    i32     block_count;
    u8      _unused2[36];
    i16     set_size;
    u8      _unused3[2];
    i16     set_index;
    u8      _unused4[2];
    i16     block_size;
    u8      _unused5[2];
    i32     path_table_size;
    u8      _unused6[4];
    i32     path_table_lba;
    u8      _unused7[4];
    i32     path_table_lba2;
    u8      _unused8[12];
    u8      dir_entry[34];
    char    set_id[128];
    char    publisher_id[128];
    char    data_prepare_id[128];
};

struct path_table_entry {
    u8      dir_id_length;
    u8      attr_record_length;
    u32     lba;
    u16     parent_index;
    char    dir_id[];
};

#endif
