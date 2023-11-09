#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/notification/notification.h>
#include <kernel/vfs/node.h>
#include <sysfs/fs.h>
#define KERNEL_LOG_NAME "sysfs_memory"



static vfs_node_t* _sysfs_memory_root;
static vfs_node_t* _sysfs_memory_pmm_counter_root;
static vfs_node_t* _sysfs_memory_object_counter_root;



static void _init_memory_load_balancer_data(void){
	vfs_node_t* root=dynamicfs_create_node(_sysfs_memory_root,"load_balancer",VFS_NODE_TYPE_DIRECTORY,NULL);
	dynamicfs_create_data_node(root,"hit_count","???");
	dynamicfs_create_data_node(root,"miss_count","???");
	dynamicfs_create_data_node(root,"late_miss_count","???");
}



static void _pmm_counter_listener(void* object,u32 type){
	handle_t* handle=object;
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		pmm_counter_descriptor_t* descriptor=handle->object;
		vfs_node_t* node=dynamicfs_create_node(_sysfs_memory_pmm_counter_root,descriptor->name,VFS_NODE_TYPE_DIRECTORY,NULL);
		dynamicfs_create_data_node(node,"type","descriptor->type");
		dynamicfs_create_data_node(node,"count","descriptor->count");
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
}



static notification_listener_t _sysfs_memory_pmm_counter_notification_listener={
	_pmm_counter_listener
};



static void _omm_allocator_listener(void* object,u32 type){
	handle_t* handle=object;
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		omm_allocator_t* allocator=handle->object;
		vfs_node_t* node=dynamicfs_create_node(_sysfs_memory_object_counter_root,allocator->name,VFS_NODE_TYPE_DIRECTORY,NULL);
		dynamicfs_create_data_node(node,"alloc_count","allocator->allocation_count");
		dynamicfs_create_data_node(node,"dealloc_count","allocator->deallocation_count");
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
}



static notification_listener_t _sysfs_memory_omm_allocator_notification_listener={
	_omm_allocator_listener
};



void sysfs_memory_init(void){
	LOG("Creating memory subsystem...");
	_sysfs_memory_root=dynamicfs_create_node(sysfs->root,"memory",VFS_NODE_TYPE_DIRECTORY,NULL);
	_init_memory_load_balancer_data();
	_sysfs_memory_pmm_counter_root=dynamicfs_create_node(_sysfs_memory_root,"counters",VFS_NODE_TYPE_DIRECTORY,NULL);
	handle_register_notification_listener(HANDLE_TYPE_PMM_COUNTER,&_sysfs_memory_pmm_counter_notification_listener);
	_sysfs_memory_object_counter_root=dynamicfs_create_node(_sysfs_memory_root,"object_counters",VFS_NODE_TYPE_DIRECTORY,NULL);
	handle_register_notification_listener(HANDLE_TYPE_OMM_ALLOCATOR,&_sysfs_memory_omm_allocator_notification_listener);
}
