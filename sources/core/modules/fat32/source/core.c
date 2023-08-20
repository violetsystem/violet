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

static inline uint32_t fat_get_cluster_directory(fat_directory_t* dir){
    return ((uint32_t)dir->cluster_low) | ((uint32_t)dir->cluster_high << 16);
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

static int fat_read_cluster_chain(fat_context_t* ctx, uint32_t current_cluster, uint32_t alignement, uint64_t size, uint64_t* size_read,  void* buffer){
    uintptr_t buffer_iteration = (uintptr_t)buffer;
    size_t size_to_read = 0;


    if(alignement < ctx->cluster_size){
        size_to_read = size;
        if(alignement + size_to_read > ctx->cluster_size){
            size_to_read = ctx->cluster_size - alignement;
        }
        assert(!fat_read_cluster(ctx, current_cluster, alignement, size_to_read, (void*)buffer_iteration));
        alignement -= size_to_read;
    }else{
        alignement -= ctx->cluster_size;
    }
    

    size -= size_to_read;
    *size_read += size_to_read;
    buffer_iteration += size_to_read;

    uint32_t next_cluster = fat_get_next_cluster_info(ctx, current_cluster) & 0x0FFFFFFF;

    if(next_cluster >= 0xffffff8 || next_cluster == 0){
        return EINVAL; // this is still a success for the size store in size_read field
    }

    if(size > 0){
        return fat_read_cluster_chain(ctx, next_cluster, alignement, size, size_read, (void*)buffer_iteration);
    }else{
        return 0;
    }
}

static bool fat_entry_valid(fat_directory_t* dir){
    return (dir->name[0] != 0) && (dir->name[0] != 0x05) && (dir->name[0] != 0xE5);
}

static bool fat_is_lfn(fat_directory_t* dir){
    return (*(uint8_t*)&dir->attributes) == 0xF;
}

static int fat_parse_lfn(char* name, fat_lfn_t* lfn){
    uint8_t index = 0;
    for(uint8_t i = 0; i < 5; i++){
        name[index++] = lfn->name1[i] & 0xFF;
    }
    for(uint8_t i = 0; i < 6; i++){
        name[index++] = lfn->name2[i] & 0xFF;
    }
    for(uint8_t i = 0; i < 2; i++){
        name[index++] = lfn->name3[i] & 0xFF;
    }

    return 0;
}

static int fat_parse_sfn(char* name, fat_directory_t* dir){
    int last = 0;
    for(; last < 8; last++){
        if(dir->name[last] == ' '){
            break;
        }
    }

    memcpy(name, dir->name, last);

    if(!dir->attributes.directory){
        name[last] = '.';
        last++;
        memcpy(name + last, &dir->name[8], 3);
        last += 3;
    }

    name[last] = '\0';

    return 0;
}

static int fat_read_entry(fat_context_t* ctx, uint32_t cluster, uint32_t entry_number, void* buffer){
    return fat_read_cluster(ctx, cluster, entry_number * ENTRY_SIZE, ENTRY_SIZE, buffer);
}

static fat_directory_t* fat_find_entry_recursive(fat_context_t* ctx, uint32_t current_cluster, const char* name, void* cluster_buffer, char* last_entry_name, bool last_entry_lfn){
    /* read current cluster */
    assert(!fat_read_cluster(ctx, current_cluster, 0, ctx->cluster_size, (void*)cluster_buffer));

    /* parse the current cluster */
    char entry_name[256];
    if(last_entry_name == NULL){
        last_entry_name = (char*)&entry_name;
    }
    for(uint64_t i = 0; i < ctx->entries_per_cluster; i++){
        fat_directory_t* dir = (fat_directory_t*)((uintptr_t)cluster_buffer + (uintptr_t)i * (uintptr_t)ENTRY_SIZE);
        if(fat_entry_valid(dir)){
            if(fat_is_lfn(dir)){
                fat_lfn_t* lfn = (fat_lfn_t*)dir;
                uint8_t order = lfn->order & ~0x40;

                last_entry_name[13 * (order - 1)] = '\0';

                for(uint8_t y = 0; y < order; y++){
                    lfn = (fat_lfn_t*)((uintptr_t)cluster_buffer + (uintptr_t)(i + y) * (uintptr_t)ENTRY_SIZE);
                    fat_parse_lfn(&last_entry_name[13 * (order - 1 - y)], lfn);
                }

                i += order - 1;

                last_entry_lfn = true;

                continue;
            }else{
                if(!last_entry_lfn){
                    fat_parse_sfn(last_entry_name, dir);
                }else{
                    last_entry_lfn = false;
                }
            }

            if(!strcmp(name, last_entry_name)){
                return dir;
            }
        }
    }

    /* find next cluster */
    uint32_t next_cluster = fat_get_next_cluster_info(ctx, current_cluster) & 0x0FFFFFFF;
    if(next_cluster >= 0xffffff8 || next_cluster == 0){
        return NULL;
    }else{
        return fat_find_entry_recursive(ctx, next_cluster, name, cluster_buffer, last_entry_name, last_entry_lfn);
    }
}

static fat_directory_t* fat_find_entry(fat_context_t* ctx, uint32_t current_cluster, const char* name, void* cluster_buffer){
    return fat_find_entry_recursive(ctx, current_cluster, name, cluster_buffer, NULL, false);
}

static fat_directory_t* fat_find_entry_with_path(fat_context_t* ctx, uint32_t current_cluster, const char* path, void* cluster_buffer){
    fat_directory_t* dir;
    char* entry_name = (char*)path;
    char* next_entry_name = strchr(entry_name, '/');
    while(next_entry_name != NULL){
        *next_entry_name = '\0';

        dir = fat_find_entry(ctx, current_cluster, entry_name, cluster_buffer);
        if(dir == NULL){
            return NULL;
        }

        current_cluster = fat_get_cluster_directory(dir);

        entry_name = next_entry_name + 1;
        next_entry_name = strchr(entry_name, '/');
    }

    dir = fat_find_entry(ctx, current_cluster, entry_name, cluster_buffer);
    return dir;
}

static fat_directory_t* fat_find_entry_with_path_from_root(fat_context_t* ctx, const char* path, void* cluster_buffer){
    return fat_find_entry_with_path(ctx, ctx->bpb->root_cluster_number, path, cluster_buffer);
}

fat_file_internal_t* fat_open(fat_context_t* ctx, const char* path){
    void* cluster_buffer = malloc(ctx->cluster_size);
    
    fat_directory_t* dir = fat_find_entry_with_path_from_root(ctx, path, cluster_buffer);
    if(dir == NULL){
        free(cluster_buffer);
        return NULL;
    }

    fat_file_internal_t* file = malloc(sizeof(fat_file_internal_t));
    file->creation_time = dir->creation_time;
    file->creation_date = dir->creation_date;
    file->last_access_date = dir->last_access_date;
    file->last_write_time = dir->last_write_time;
    file->last_write_date = dir->last_write_date;
    file->cluster = fat_get_cluster_directory(dir);
    file->size = dir->size;

    size_t path_size = strlen(path) + 1;
    file->path = malloc(path_size);
    memcpy(file->path, path, path_size);

    file->ctx = ctx;

    free(cluster_buffer);

    return file;
}

int fat_read(fat_file_internal_t* file, uint64_t start, size_t size, size_t* size_read, void* buffer){
    uint64_t size_read_tmp = 0;

    int err = fat_read_cluster_chain(file->ctx, file->cluster, start, (uint64_t)size, &size_read_tmp, buffer);

    *size_read = (size_t)size_read_tmp;

    return err;
}

int fat_write(fat_file_internal_t* file, uint64_t start, size_t size, void* buffer){
    return 0;
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
    ctx->entries_per_cluster = lba_to_bytes(ctx->bpb->sectors_per_cluster) / ENTRY_SIZE;

    fat_file_internal_t* file = fat_open(ctx, "ahci.ksys");
    size_t size_read;
    void* buffer = malloc(1);
    fat_read(file, 0x4E0, 1, &size_read, buffer);
    log_printf("%x\n", *(uint8_t*)buffer);

    return 0;
}