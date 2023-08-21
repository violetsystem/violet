#ifndef _MODULE_FAT32_CORE_H
#define _MODULE_FAT32_CORE_H

#include <errno.h>
#include <lib/lock.h>
#include <lib/memory.h>
#include <lib/string.h>

#define ENTRY_SIZE                  (32)
#define ENTRY_FIELD_OFFSET_SIZE     (28)

#define END_OF_CLUSTERCHAIN         (0x0FFFFFFF)

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
    uint32_t fsi_signature;
    uint8_t reserved1[400];
    uint32_t struct_signature;
    uint32_t free_cluster_count;
    uint32_t next_free_cluster;
    uint8_t reserved2[12];
}__attribute__((packed)) fs_info_t;

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
    uint8_t name[11];
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
    uint8_t order;
    uint16_t name1[5];
    fat_attributes_t attributes;
    uint8_t lfnType;
    uint8_t checksum;
    uint16_t name2[6];
    uint16_t reserved;
    uint16_t name3[2];
}__attribute__((packed)) fat_lfn_t;

typedef struct{
    void* volume;
    uint64_t start;
    uint64_t size;
    struct storage_device_t* device;
} partition_t;

typedef struct{
    bpb_t* bpb;
    fs_info_t* fsi;
    uint32_t* fat;
    uint64_t fat_size;
    uint64_t fat1_position;
    uint64_t fat2_position;
    uint64_t first_usable_lba;
    uint64_t cluster_size;
    uint64_t entries_per_cluster;
    uint64_t cluster_count;
    uint64_t data_cluster_count;
    uint64_t fat_entry_count;
    uint64_t next_free_cluster;
    partition_t* partition;
} fat_context_t;

typedef struct{
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t last_write_time;
    uint16_t last_write_date;
    uint32_t cluster;
    uint32_t size;
    char* path;
    fat_context_t* ctx;
} fat_file_internal_t;

#endif // _MODULE_FAT32_CORE_H