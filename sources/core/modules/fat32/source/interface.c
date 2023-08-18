#include <errno.h>
#include <interface.h>

storage_potential_owner_t fat32_potential_owner;

int get_ownership(storage_device_t* device, uint64_t start, uint64_t size, guid_t* guid){
	partition_t* partition = malloc(sizeof(partition_t));
	partition->start = start;
	partition->size = size;
	partition->device = device;

	if(partition_mount(partition)){
		struct dir_s dir;
		fat_dir_open(partition->volume, &dir, "C:/alpha/", 0);
		
		struct info_s* info = (struct info_s *)malloc(sizeof(struct info_s));
		fstatus status;
		do {
			status = fat_dir_read(&dir, info);
			log_printf("%.*s\n", info->name_length, info->name);
		} while (status != FSTATUS_EOF);
		return 0;
	}

	free(partition);
	return EINVAL;
}

void init_interface(void){
	fat32_potential_owner.get_ownership = &get_ownership;
    storage_handler->add_potential_owner(&fat32_potential_owner);
}

static inline uint64_t convert_lba_to_bytes(uint64_t value){
    return value << 9;
}

u8 partition_read(partition_t* partition, u8* buffer, u32 lba, u32 count) {
	uint64_t start = partition->start + convert_lba_to_bytes((uint64_t)lba);
	size_t size = (size_t)convert_lba_to_bytes((uint64_t)count);
	if(start + size > partition->start + partition->size){
		return 0;
	}
	partition->device->read(partition->device, start, size, (void*)buffer);
	return 1;
}

u8 partition_write(partition_t* partition, const u8* buffer, u32 lba, u32 count) {
	uint64_t start = partition->start + convert_lba_to_bytes((uint64_t)lba);
	size_t size = (size_t)convert_lba_to_bytes((uint64_t)count);
	if(start + size > partition->start + partition->size){
		return 0;
	}
	partition->device->write(partition->device, start, size, (void*)buffer);
	return 1;
}
