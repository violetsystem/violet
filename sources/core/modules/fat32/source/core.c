#include <core.h>

#define FAT32_SIGNATURE (((uint64_t)'F' << (0)) | ((uint64_t)'A' << (8)) | ((uint64_t)'T' << (16)) | ((uint64_t)'3' << (24)) | ((uint64_t)'2' << (32)) | ((uint64_t)' ' << (40)) | ((uint64_t)' ' << (48)) | ((uint64_t)' ' << (56)))

static uint64_t cluster_to_lba(fat_context_t* ctx, uint32_t cluster){
    return ctx->first_usable_lba + ctx->bpb->sectors_per_cluster * (cluster - 2);
}

static uint32_t lba_to_cluster(fat_context_t* ctx, uint64_t lba){
    return ((lba - ctx->first_usable_lba) / ctx->bpb->sectors_per_cluster) + 2;
}

static int read_partition(partition_t* partition, uint64_t start, size_t size, void* buffer){
    if(start + size > partition->start + partition->size){
		return EINVAL;
	}
    return partition->device->read(partition->device, start + partition->start, size, buffer);
}

static int write_partition(partition_t* partition, uint64_t start, size_t size, void* buffer){
    if(start + size > partition->start + partition->size){
		return EINVAL;
	}
    return partition->device->write(partition->device, start + partition->start, size, buffer);
}

static inline uint64_t bytes_to_lba(uint64_t value){
    return value >> 9;
}

static inline uint64_t lba_to_bytes(uint64_t value){
    return value << 9;
}



static int fat_read_boot_sector(fat_context_t* ctx){
    return read_partition(ctx->partition, 0, sizeof(bpb_t), ctx->bpb);
}

static int fat_read_fat(fat_context_t* ctx){
    return read_partition(ctx->partition, lba_to_bytes(ctx->bpb->reserved_sectors), lba_to_bytes(ctx->bpb->sectors_per_fat), ctx->fat);
}

static inline uint32_t fat_get_next_cluster_info(fat_context_t* ctx, uint32_t current){
    return ctx->fat[current];
}

static int fat_read_cluster(fat_context_t* ctx, uint32_t cluster, uint32_t alignement, uint64_t size, void* buffer){
    uint64_t start = ctx->partition->start + lba_to_bytes(cluster_to_lba(ctx, cluster)) + alignement;
    if(start + size > ctx->partition->start + ctx->partition->size){
        return EINVAL;
    }
    return ctx->partition->device->read(ctx->partition->device, start, size, buffer);
}

static int fat_get_cluster_chain_count(fat_context_t* ctx, uint32_t cluster){
    int i = 0;
    while(cluster < 0xffffff8 && cluster != 0){
        cluster = fat_get_next_cluster_info(ctx, cluster) & 0x0FFFFFFF;
        i++;
    }
    return i;
}

static int fat_read_cluster_chain(fat_context_t* ctx, uint32_t first_cluster, uint32_t alignement, uint64_t size, uint64_t* size_read,  void* buffer){
    uintptr_t buffer_iteration = (uintptr_t)buffer;
    size_t size_to_read = size;

    if(size_to_read > ctx->cluster_size){
        size_to_read = ctx->cluster_size;
    }

    assert(!fat_read_cluster(ctx, first_cluster, alignement, size_to_read, (void*)buffer_iteration));

    if(size != -1){
        size -= size_to_read;
    }
    *size_read += size_to_read;
    buffer_iteration += size_to_read;

    uint32_t next_cluster = fat_get_next_cluster_info(ctx, first_cluster) & 0x0FFFFFFF;

    if(next_cluster >= 0xffffff8 || next_cluster == 0){
        if(size == -1){
            return 0;
        }else{
            return EINVAL; // this is still a success for the size store in size_read field
        }
    }

    if(size > 0 || size == -1){
        return fat_read_cluster_chain(ctx, next_cluster, 0, size, size_read, (void*)buffer_iteration);
    }else{
        return 0;
    }
}

static int fat_read_entry(fat_context_t* ctx, uint32_t cluster, uint32_t entry_number, void* buffer){
    return fat_read_cluster(ctx, cluster, entry_number * 32, 32, buffer);
}

int fat_mount(partition_t* partition){
    fat_context_t* ctx = malloc(sizeof(fat_context_t));
    ctx->partition = partition;

    ctx->bpb = malloc(sizeof(bpb_t));
    assert(!fat_read_boot_sector(ctx));

    if(ctx->bpb->identifier != FAT32_SIGNATURE){
        free(ctx->bpb);
        free(ctx);
        return EINVAL;
    }
    
    ctx->fat = malloc(lba_to_bytes(ctx->bpb->sectors_per_fat));
    assert(!fat_read_fat(ctx));

    ctx->first_usable_lba = ctx->bpb->reserved_sectors + ctx->bpb->fats * ctx->bpb->sectors_per_fat;
    ctx->cluster_size = lba_to_bytes(ctx->bpb->sectors_per_cluster);

    void* buffer = malloc(32);
    fat_read_entry(ctx, ctx->bpb->root_cluster_number, 5, buffer);
    fat_directory_t* directory = buffer;

    log_printf("%s %d %d\n", directory->name, *(uint8_t*)&directory->attributes, fat_get_cluster_chain_count(ctx, ctx->bpb->root_cluster_number));

    return 0;
}

file_internal_t* fat_open(fat_context_t* ctx, const char* path){
    return NULL;
}

int fat_read(file_internal_t* file, uint64_t start, size_t size, void* buffer){
    return 0;
}

int fat_write(file_internal_t* file, uint64_t start, size_t size, void* buffer){
    return 0;
}