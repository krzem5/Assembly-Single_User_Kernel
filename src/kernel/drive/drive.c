#include <kernel/drive/drive.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "drive"



static pmm_counter_descriptor_t _drive_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_drive");
static omm_allocator_t _drive_allocator=OMM_ALLOCATOR_INIT_STRUCT("drive",sizeof(drive_t),8,4,&_drive_omm_pmm_counter);



HANDLE_DECLARE_TYPE(DRIVE,{
	drive_t* drive=handle->object;
	WARN("Delete drive: %s",drive->name);
	handle_release(&(drive->type->handle));
	if (drive->partition_table_descriptor){
		handle_release(&(drive->partition_table_descriptor->handle));
	}
	omm_dealloc(&_drive_allocator,drive);
});
HANDLE_DECLARE_TYPE(DRIVE_TYPE,{});



void drive_register_type(drive_type_t* type){
	LOG("Registering drive type '%s'...",type->name);
	handle_new(type,HANDLE_TYPE_DRIVE_TYPE,&(type->handle));
}



void drive_unregister_type(drive_type_t* type){
	LOG("Unregistering drive type '%s'...",type->name);
	handle_destroy(&(type->handle));
}



drive_t* drive_create(const drive_config_t* config){
	handle_acquire(&(config->type->handle));
	LOG("Creating drive '%s' as '%s/%s'...",config->name,config->type->name,config->model_number);
	drive_t* out=omm_alloc(&_drive_allocator);
	handle_new(out,HANDLE_TYPE_DRIVE,&(out->handle));
	out->type=config->type;
	out->block_size_shift=__builtin_ctzll(config->block_size);
	out->partition_table_descriptor=NULL;
	memcpy(out->name,config->name,DRIVE_NAME_LENGTH);
	memcpy(out->serial_number,config->serial_number,DRIVE_SERIAL_NUMBER_LENGTH);
	memcpy(out->model_number,config->model_number,DRIVE_MODEL_NUMBER_LENGTH);
	out->block_count=config->block_count;
	out->block_size=config->block_size;
	out->extra_data=config->extra_data;
	INFO("Drive serial number: '%s', Drive size: %v (%lu * %lu)",out->serial_number,out->block_count*out->block_size,out->block_count,out->block_size);
	if (out->block_size&(out->block_size-1)){
		WARN("Drive block size is not a power of 2");
	}
	partition_load_from_drive(out);
	return out;
}



u64 drive_read(drive_t* drive,u64 offset,void* buffer,u64 size){
	return drive->type->io_callback(drive->extra_data,offset&DRIVE_OFFSET_MASK,buffer,size);
}



u64 drive_write(drive_t* drive,u64 offset,const void* buffer,u64 size){
	return drive->type->io_callback(drive->extra_data,(offset&DRIVE_OFFSET_MASK)|DRIVE_OFFSET_FLAG_WRITE,(void*)buffer,size);
}
