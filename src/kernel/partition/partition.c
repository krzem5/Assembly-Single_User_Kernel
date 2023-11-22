#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "partition"



static pmm_counter_descriptor_t _partition_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_partition");
static omm_allocator_t* _partition_allocator=NULL;

KERNEL_PUBLIC handle_type_t partition_handle_type=0;
KERNEL_PUBLIC handle_type_t partition_table_descriptor_handle_type=0;



static void _partition_handle_destructor(handle_t* handle){
	partition_t* partition=handle->object;
	WARN("Delete partition: %s",partition->name);
	if (partition->descriptor){
		handle_release(&(partition->descriptor->handle));
	}
	omm_dealloc(_partition_allocator,partition);
}



KERNEL_PUBLIC void partition_register_table_descriptor(partition_table_descriptor_t* descriptor){
	LOG("Registering partition table descriptor '%s'...",descriptor->name);
	if (!partition_table_descriptor_handle_type){
		partition_table_descriptor_handle_type=handle_alloc("partition_table_descriptor",0);
	}
	handle_new(descriptor,partition_table_descriptor_handle_type,&(descriptor->handle));
	handle_finish_setup(&(descriptor->handle));
	HANDLE_FOREACH(drive_handle_type){
		drive_t* drive=handle->object;
		if (drive->partition_table_descriptor){
			continue;
		}
		handle_acquire(&(descriptor->handle));
		drive->partition_table_descriptor=descriptor;
		if (descriptor->load_callback(drive)){
			INFO("Detected partitioning of drive '%s' as '%s'",drive->model_number->data,descriptor->name);
		}
		else{
			drive->partition_table_descriptor=NULL;
			handle_release(&(descriptor->handle));
		}
	}
}



KERNEL_PUBLIC void partition_unregister_table_descriptor(partition_table_descriptor_t* descriptor){
	LOG("Unregistering partition table descriptor '%s'...",descriptor->name);
	handle_destroy(&(descriptor->handle));
}



KERNEL_PUBLIC void partition_load_from_drive(drive_t* drive){
	LOG("Loading partitions from drive '%s'...",drive->model_number->data);
	HANDLE_FOREACH(partition_table_descriptor_handle_type){
		partition_table_descriptor_t* descriptor=handle->object;
		handle_acquire(&(descriptor->handle));
		drive->partition_table_descriptor=descriptor;
		if (descriptor->load_callback(drive)){
			INFO("Detected drive partitioning as '%s'",descriptor->name);
			return;
		}
		handle_release(&(descriptor->handle));
	}
	drive->partition_table_descriptor=NULL;
	WARN("Unable to detect partition type of drive '%s'",drive->model_number->data);
}



KERNEL_PUBLIC partition_t* partition_create(drive_t* drive,u32 index,const char* name,u64 start_lba,u64 end_lba){
	LOG("Creating partition '%s' on drive '%s'...",name,drive->model_number->data);
	handle_acquire(&(drive->partition_table_descriptor->handle));
	if (!_partition_allocator){
		_partition_allocator=omm_init("partition",sizeof(partition_t),8,4,&_partition_omm_pmm_counter);
		spinlock_init(&(_partition_allocator->lock));
	}
	if (!partition_handle_type){
		partition_handle_type=handle_alloc("partition",_partition_handle_destructor);
	}
	partition_t* out=omm_alloc(_partition_allocator);
	handle_new(out,partition_handle_type,&(out->handle));
	out->descriptor=drive->partition_table_descriptor;
	out->drive=drive;
	out->index=index;
	out->name=smm_alloc(name,0);
	out->start_lba=start_lba;
	out->end_lba=end_lba;
	out->fs=fs_load(out);
	if (!out->fs){
		WARN("No filesystem detected on partition '%s%ud%up%u/%s'",drive->type->name,drive->controller_index,drive->device_index,index,name);
	}
	handle_finish_setup(&(out->handle));
	return out;
}
