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

static int fat_read_fs_info_sector(fat_context_t* ctx){
    int err = read_partition(ctx->partition, lba_to_bytes(ctx->bpb->sector_number_fs_info), sizeof(fs_info_t), ctx->fsi);
    if(err){
        return err;
    }
    
    if(ctx->fsi->lead_signature != FSI_LEAD_SIGNATURE){
        return EINVAL;
    }

    if(ctx->fsi->struct_signature != FSI_STRUCT_SIGNATURE){
        return EINVAL;
    }

    if(ctx->fsi->trail_signature != FSI_TRAIL_SIGNATURE){
        return EINVAL;
    }

    return 0;
}

static int fat_write_fs_info_sector(fat_context_t* ctx){
    return write_partition(ctx->partition, lba_to_bytes(ctx->bpb->sector_number_fs_info), sizeof(fs_info_t), ctx->fsi);
}

static int fat_read_fat(fat_context_t* ctx){
    return read_partition(ctx->partition, ctx->fat1_position, ctx->fat_size, ctx->fat);
}

static inline uint32_t fat_get_next_cluster(fat_context_t* ctx, uint32_t current){
    return ctx->fat[current] & 0x0FFFFFFF;
}

static inline int fat_set_next_cluster(fat_context_t* ctx, uint32_t current, uint32_t next_cluster){
    ctx->fat[current] = (next_cluster & 0x0FFFFFFF) | (ctx->fat[current] & 0xF0000000);
    // write fat 1
    write_partition(ctx->partition, (uint64_t)ctx->fat1_position + (uint64_t)current * (uint64_t)sizeof(uint32_t), sizeof(uint32_t), &ctx->fat[current]);
    // write fat 2
    write_partition(ctx->partition, (uint64_t)ctx->fat2_position + (uint64_t)current * (uint64_t)sizeof(uint32_t), sizeof(uint32_t), &ctx->fat[current]);
    return 0;
}

static spinlock_t fat_allocate_cluster_lock = {};

static int fat_allocate_cluster(fat_context_t* ctx, uint32_t* cluster){
    spinlock_acquire(&fat_allocate_cluster_lock);
    for(uint64_t i = ctx->next_free_cluster; i < ctx->fat_entry_count; i++){
        if((fat_get_next_cluster(ctx, i)) == 0){
            fat_set_next_cluster(ctx, i, END_OF_CLUSTERCHAIN);
            ctx->next_free_cluster = i;
            ctx->fsi->free_cluster_count--;
            fat_write_fs_info_sector(ctx);
            spinlock_release(&fat_allocate_cluster_lock);
            *cluster = i;
            return 0;
        }
    }
    spinlock_release(&fat_allocate_cluster_lock);
    return EINVAL;
}

static int fat_free_all_following_clusters(fat_context_t* ctx, uint32_t cluster){
    uint32_t next_cluster = fat_get_next_cluster(ctx, cluster);
    fat_set_next_cluster(ctx, cluster, 0);
    ctx->fsi->free_cluster_count++;
    fat_write_fs_info_sector(ctx);
    if(next_cluster < 0x0FFFFFF8){
        return fat_free_all_following_clusters(ctx, next_cluster);
    }else{
        return 0;
    }
}


static int fat_read_cluster(fat_context_t* ctx, uint32_t cluster, uint32_t alignement, uint64_t size, void* buffer){
    uint64_t start = ctx->partition->start + lba_to_bytes(cluster_to_lba(ctx, cluster)) + alignement;
    if(start + size > ctx->partition->start + ctx->partition->size){
        return EINVAL;
    }
    return ctx->partition->device->read(ctx->partition->device, start, size, buffer);
}


static int fat_write_cluster(fat_context_t* ctx, uint32_t cluster, uint32_t alignement, uint64_t size, void* buffer){
    uint64_t start = ctx->partition->start + lba_to_bytes(cluster_to_lba(ctx, cluster)) + alignement;
    if(start + size > ctx->partition->start + ctx->partition->size){
        return EINVAL;
    }
    return ctx->partition->device->write(ctx->partition->device, start, size, buffer);
}

static int fat_get_cluster_chain_count(fat_context_t* ctx, uint32_t cluster){
    int i = 0;
    while(cluster < 0xFFFFFF8 && cluster != 0){
        cluster = fat_get_next_cluster(ctx, cluster);
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

    uint32_t next_cluster = fat_get_next_cluster(ctx, current_cluster);

    if(size > 0){
        if(next_cluster >= 0xFFFFFF8 || next_cluster == 0){
            return EINVAL; // this is still a success for the size store in size_read field
        }
        return fat_read_cluster_chain(ctx, next_cluster, alignement, size, size_read, (void*)buffer_iteration);
    }else{
        return 0;
    }
}

static int fat_write_cluster_chain(fat_context_t* ctx, uint32_t current_cluster, uint32_t alignement, uint64_t size, uint64_t* size_write,  void* buffer, bool is_end_of_file){
    uintptr_t buffer_iteration = (uintptr_t)buffer;
    size_t size_to_write = 0;


    if(alignement < ctx->cluster_size){
        size_to_write = size;
        if(alignement + size_to_write > ctx->cluster_size){
            size_to_write = ctx->cluster_size - alignement;
        }
        if(size_to_write){
            assert(!fat_write_cluster(ctx, current_cluster, alignement, size_to_write, (void*)buffer_iteration));
        }
        alignement = 0;
    }else{
        alignement -= ctx->cluster_size;
    }

    

    size -= size_to_write;
    *size_write += size_to_write;
    buffer_iteration += size_to_write;

    uint32_t next_cluster = fat_get_next_cluster(ctx, current_cluster);

    if(size > 0){
        if(next_cluster >= 0xFFFFFF8 || next_cluster == 0){
            if(fat_allocate_cluster(ctx, &next_cluster)){
                return EIO;
            }
            fat_set_next_cluster(ctx, current_cluster, next_cluster);
        }
        return fat_write_cluster_chain(ctx, next_cluster, alignement, size, size_write, (void*)buffer_iteration, is_end_of_file);
    }else{
        if(!is_end_of_file || next_cluster >= 0xFFFFFF8 || next_cluster == 0){
            return 0;
        }

        // we have to free every following clusters
        fat_free_all_following_clusters(ctx, next_cluster);

        // put the end of file after the current cluster
        fat_set_next_cluster(ctx, current_cluster, END_OF_CLUSTERCHAIN);

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

static int fat_read_one_entry(fat_context_t* ctx, uint32_t cluster, uint32_t entry_number, fat_directory_t* dir){
    return fat_read_cluster(ctx, cluster, entry_number * ENTRY_SIZE, ENTRY_SIZE, dir);
}

static fat_directory_t* fat_read_entry_with_cache(fat_context_t* ctx, uint32_t cluster_base, uint32_t entry_number, void* cluster_buffer, uint32_t* cluster_cache_id_count_from_base, uint32_t* last_cluster_read){
    uint32_t cluster_count_from_base = entry_number / ctx->entries_per_cluster;

    /* Caching system */
    if(cluster_count_from_base != *cluster_cache_id_count_from_base - 1){ // we begin to one
        *cluster_cache_id_count_from_base -= 1; // we begin to one
        uint32_t cluster;
        uint32_t cluster_index;

        if(*cluster_cache_id_count_from_base && cluster_count_from_base > *cluster_cache_id_count_from_base){
            cluster = *last_cluster_read;
            cluster_index = *cluster_cache_id_count_from_base;
        }else{
            cluster = cluster_base;
            cluster_index = 0;
        }

        for(; cluster_index < cluster_count_from_base; cluster_index++){
            cluster = fat_get_next_cluster(ctx, cluster);
            if(cluster >= 0xFFFFFF8 || cluster == 0){
                return NULL;
            }
        }

        fat_read_cluster(ctx, cluster, 0, ctx->cluster_size, cluster_buffer);
        *cluster_cache_id_count_from_base = cluster_count_from_base + 1; // we begin to one
        *last_cluster_read = cluster;
    }

    /* Read entry from cache */
    uint32_t entry_number_in_cluster = entry_number % ctx->entries_per_cluster;
    return (fat_directory_t*)((uintptr_t)cluster_buffer + (uintptr_t)entry_number_in_cluster * (uintptr_t)ENTRY_SIZE);
}

static int fat_find_entry_info(fat_context_t* ctx, uint32_t current_cluster, const char* name, void* cluster_buffer, uint64_t* sfn_position, uint32_t* sfn_entry_index, uint32_t* lfn_first_entry_index){
    char entry_name[256];

    uint32_t cluster_cache_id_count_from_base = 0;
    uint32_t last_cluster_read = 0;

    uint32_t entry_index = 0;

    uint64_t last_entry_index_lfn = 0;

    bool is_last_entry_lfn = false;

    fat_directory_t* dir;
    while((dir = fat_read_entry_with_cache(ctx, current_cluster, entry_index, cluster_buffer, &cluster_cache_id_count_from_base, &last_cluster_read)) != NULL){
        if(fat_entry_valid(dir)){
            if(fat_is_lfn(dir)){
                fat_lfn_t* lfn = (fat_lfn_t*)dir;
                uint8_t order = lfn->order & ~0x40;

                last_entry_index_lfn = entry_index;

                for(uint8_t y = 0; y < order; y++){
                    lfn = (fat_lfn_t*)fat_read_entry_with_cache(ctx, current_cluster, entry_index + y, cluster_buffer, &cluster_cache_id_count_from_base, &last_cluster_read);
                    fat_parse_lfn(&entry_name[13 * (order - 1 - y)], lfn);
                }

                entry_index += order - 1;

                is_last_entry_lfn = true;
            }else{
                if(!is_last_entry_lfn){
                    fat_parse_sfn(entry_name, dir);
                }else{
                    is_last_entry_lfn = false;
                }
                // Only sfn entry can be return because lfn are just use to store string not the entry data
                if(!strcmp(name, entry_name)){
                    if(sfn_position != NULL){
                        *sfn_position = lba_to_bytes(cluster_to_lba(ctx, last_cluster_read)) + (entry_index % ctx->entries_per_cluster) * ENTRY_SIZE;
                    }
                    if(sfn_entry_index != NULL){
                        *sfn_entry_index = entry_index;
                    }
                    if(lfn_first_entry_index != NULL){
                        *lfn_first_entry_index = last_entry_index_lfn;
                    }
                    return 0;
                }            
            }
        }
        
        entry_index++;
    }

    return ENOENT;
}

static fat_directory_t* fat_find_entry(fat_context_t* ctx, uint32_t current_cluster, const char* name, void* cluster_buffer){
    char entry_name[256];

    uint32_t cluster_cache_id_count_from_base = 0;
    uint32_t last_cluster_read = 0;

    uint32_t entry_index = 0;

    bool is_last_entry_lfn = false;

    fat_directory_t* dir;
    while((dir = fat_read_entry_with_cache(ctx, current_cluster, entry_index, cluster_buffer, &cluster_cache_id_count_from_base, &last_cluster_read)) != NULL){
        if(fat_entry_valid(dir)){
            if(fat_is_lfn(dir)){
                fat_lfn_t* lfn = (fat_lfn_t*)dir;
                uint8_t order = lfn->order & ~0x40;

                for(uint8_t y = 0; y < order; y++){
                    lfn = (fat_lfn_t*)fat_read_entry_with_cache(ctx, current_cluster, entry_index + y, cluster_buffer, &cluster_cache_id_count_from_base, &last_cluster_read);
                    fat_parse_lfn(&entry_name[13 * (order - 1 - y)], lfn);
                }

                entry_index += order - 1;

                is_last_entry_lfn = true;
            }else{
                if(!is_last_entry_lfn){
                    fat_parse_sfn(entry_name, dir);
                }else{
                    is_last_entry_lfn = false;
                }
                // Only sfn entry can be return because lfn are just use to store string not the entry data
                if(!strcmp(name, entry_name)){
                    return dir;
                }            
            }
        }
        
        entry_index++;
    }

    return NULL;
}

static int fat_find_entry_info_with_path(fat_context_t* ctx, uint32_t current_cluster, const char* path, void* cluster_buffer, uint64_t* sfn_position, uint32_t* sfn_entry_index, uint32_t* lfn_first_entry_index){
    fat_directory_t* dir;
    char* entry_name = (char*)path;
    char* next_entry_name = strchr(entry_name, '/');
    while(next_entry_name != NULL){
        *next_entry_name = '\0';

        dir = fat_find_entry(ctx, current_cluster, entry_name, cluster_buffer);
        if(dir == NULL){
            return ENOENT;
        }
        if(!dir->attributes.directory){
            return ENOENT;
        }

        current_cluster = fat_get_cluster_directory(dir);

        entry_name = next_entry_name + 1;
        next_entry_name = strchr(entry_name, '/');
    }

    return fat_find_entry_info(ctx, current_cluster, entry_name, cluster_buffer, sfn_position, sfn_entry_index, lfn_first_entry_index);
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

        if(!dir->attributes.directory){
            return NULL;
        }

        current_cluster = fat_get_cluster_directory(dir);

        entry_name = next_entry_name + 1;
        next_entry_name = strchr(entry_name, '/');
    }

    return fat_find_entry(ctx, current_cluster, entry_name, cluster_buffer);
}

static int fat_find_entry_info_with_path_from_root(fat_context_t* ctx, const char* path, void* cluster_buffer, uint64_t* sfn_position, uint32_t* sfn_entry_index, uint32_t* lfn_first_entry_index){
    return fat_find_entry_info_with_path(ctx, ctx->bpb->root_cluster_number, path, cluster_buffer, sfn_position, sfn_entry_index, lfn_first_entry_index);
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

static int fat_update_file_size(fat_file_internal_t* file){
    void* cluster_buffer = malloc(file->ctx->cluster_size);
    uint64_t sfn_position;
    int err = fat_find_entry_info_with_path_from_root(file->ctx, file->path, cluster_buffer, &sfn_position, NULL, NULL);
    free(cluster_buffer);
    if(err){
        return err;
    }
    return write_partition(file->ctx->partition, sfn_position + ENTRY_FIELD_OFFSET_SIZE, sizeof(uint32_t), &file->size);
}

int fat_read(fat_file_internal_t* file, uint64_t start, size_t size, size_t* size_read, void* buffer){
    uint64_t size_read_tmp = 0;

    int err = fat_read_cluster_chain(file->ctx, file->cluster, start, (uint64_t)size, &size_read_tmp, buffer);

    *size_read = (size_t)size_read_tmp;

    return err;
}

int fat_write(fat_file_internal_t* file, uint64_t start, size_t size, size_t* size_write, void* buffer, bool is_end_of_file){
    
    uint64_t size_write_tmp = 0;

    int err = fat_write_cluster_chain(file->ctx, file->cluster, start, (uint64_t)size, &size_write_tmp, buffer, is_end_of_file);

    *size_write = (size_t)size_write_tmp;

    if(is_end_of_file || start + *size_write > file->size){
        file->size = start + *size_write;
        fat_update_file_size(file);
    }

    return err;
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

    ctx->fsi = malloc(sizeof(fs_info_t));
    assert(!fat_read_fs_info_sector(ctx));


    uint64_t total_sector = 0;
    if(ctx->bpb->total_sectors16 != 0){
        total_sector = ctx->bpb->total_sectors16;
    }else{
        total_sector = ctx->bpb->total_sectors32;
    }

    if(ctx->bpb->sectors_per_cluster){
        ctx->cluster_count = total_sector / ctx->bpb->sectors_per_cluster;
    }else{
        ctx->cluster_count = total_sector;
    }
    ctx->data_cluster_count = total_sector - (ctx->bpb->root_cluster_number + (ctx->bpb->fats * ctx->bpb->sectors_per_fat));

    if((uint64_t)ctx->fsi->next_free_cluster <= ctx->cluster_count){
        ctx->next_free_cluster = (uint64_t)ctx->fsi->next_free_cluster;
    }else{
        ctx->next_free_cluster = 0;
    }

    ctx->fat_size = lba_to_bytes(ctx->bpb->sectors_per_fat);
    ctx->fat1_position = lba_to_bytes(ctx->bpb->reserved_sectors);
    ctx->fat2_position = ctx->fat1_position + ctx->fat_size;

    
    ctx->fat = malloc(lba_to_bytes(ctx->bpb->sectors_per_fat));
    assert(!fat_read_fat(ctx));

    ctx->first_usable_lba = ctx->bpb->reserved_sectors + ctx->bpb->fats * ctx->bpb->sectors_per_fat;
    ctx->cluster_size = lba_to_bytes(ctx->bpb->sectors_per_cluster);
    ctx->entries_per_cluster = lba_to_bytes(ctx->bpb->sectors_per_cluster) / ENTRY_SIZE;
    ctx->fat_entry_count = lba_to_bytes(ctx->bpb->sectors_per_fat) / sizeof(uint32_t);

    fat_file_internal_t* file = fat_open(ctx, "modules.cfg");
    char* buffer = "Hello world !!";
    size_t size = strlen(buffer);
    for(int i = 0; i < 37; i++){
        size_t size_tmp = 0;
        fat_write(file, i * size, size, &size_tmp, buffer, true);
    }

    return 0;
}