#ifndef _KERNEL_FS__FS_TYPES_H_
#define _KERNEL_FS__FS_TYPES_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



typedef struct _FILESYSTEM{
	handle_t handle;
	lock_t lock;
	struct _FILESYSTEM_DESCRIPTOR* descriptor;
	vfs_functions_t* functions;
	struct _PARTITION* partition;
	void* extra_data;
	vfs_node_t* root;
} filesystem_t;



#endif
