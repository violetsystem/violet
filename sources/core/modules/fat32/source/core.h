#ifndef _MODULE_FAT32_CORE_H
#define _MODULE_FAT32_CORE_H

#include <errno.h>

typedef struct{
    uint8_t jump[3];
    uint8_t oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fats;
    uint16_t root_directory_entries;
    uint16_t total_sectors16;
    uint8_t media_descriptor_type;
    uint16_t reserved;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors32;
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t version;
    uint32_t root_cluster_number;
    uint16_t sector_number_fs_info;
    uint16_t backup_boot_sector;
    uint8_t reserved2[12];
    uint8_t drive_number;
    uint8_t nt_flags;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t label[11];
    uint64_t identifier;
    uint8_t boot_code[420];
    uint16_t boot_signature;
}__attribute__((packed)) bpb_t;

typedef struct{
    uint8_t readOnly:1;
    uint8_t hidden:1;
    uint8_t system:1;
    uint8_t volumeID:1;
    uint8_t directory:1;
    uint8_t archive:1;
    uint8_t reserved:2;
}__attribute__((packed)) fat_attributes_t;

typedef struct{
    char name[11];
    fat_attributes_t attributes;
    uint8_t flags;
    uint8_t time_resolution;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t cluster_high;
    uint16_t last_write_time;
    uint16_t last_write_date;
    uint16_t cluster_low;
    uint32_t size;
}__attribute__((packed)) fat_directory_t;

typedef struct{
    void* volume;
    uint64_t start;
    uint64_t size;
    struct storage_device_t* device;
} partition_t;

typedef struct{
    bpb_t* bpb;
    uint32_t* fat;
    uint64_t first_usable_lba;
    uint64_t cluster_size;
    partition_t* partition;
} fat_context_t;

typedef struct{

} file_internal_t;

#endif // _MODULE_FAT32_CORE_H