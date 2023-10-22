#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread_list.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "process"



#define USERSPACE_LOWEST_ADDRESS 0x0000000000001000ull
#define USERSPACE_HIGHEST_ADDRESS 0x0000800000000000ull

#define KERNELSPACE_LOWEST_ADDRESS 0xfffff00000000000ull



PMM_DECLARE_COUNTER2(OMM_PROCESS);



static omm_allocator_t _process_allocator=OMM_ALLOCATOR_INIT_STRUCT("process",sizeof(process_t),8,2,&_pmm_counter_descriptor_OMM_PROCESS);



static HANDLE_DECLARE_TYPE(PROCESS,{
	process_t* process=handle->object;
	if (process->thread_list.head){
		panic("Unterminated process not referenced");
	}
	vmm_memory_map_deinit(&(process->pagemap),&(process->mmap));
	vmm_pagemap_deinit(&(process->pagemap));
	omm_dealloc(&_process_allocator,process);
});

process_t* process_kernel;
vmm_memory_map_t process_kernel_image_mmap;



void process_init(void){
	LOG("Creating kernel process...");
	process_kernel=omm_alloc(&_process_allocator);
	handle_new(process_kernel,HANDLE_TYPE_PROCESS,&(process_kernel->handle));
	lock_init(&(process_kernel->lock));
	vmm_pagemap_init(&(process_kernel->pagemap));
	vmm_memory_map_init(KERNELSPACE_LOWEST_ADDRESS,kernel_get_offset(),&(process_kernel->mmap));
	thread_list_init(&(process_kernel->thread_list));
	vmm_memory_map_init(kernel_get_offset(),-PAGE_SIZE,&process_kernel_image_mmap);
	if (!vmm_memory_map_reserve(&process_kernel_image_mmap,kernel_get_offset(),kernel_data.first_free_address+0x1000000/*Due to KMM*/)){
		panic("Unable to reserve kernel memory");
	}
}



process_t* process_new(void){
	process_t* out=omm_alloc(&_process_allocator);
	handle_new(out,HANDLE_TYPE_PROCESS,&(out->handle));
	lock_init(&(out->lock));
	vmm_pagemap_init(&(out->pagemap));
	vmm_memory_map_init(USERSPACE_LOWEST_ADDRESS,USERSPACE_HIGHEST_ADDRESS,&(out->mmap));
	thread_list_init(&(out->thread_list));
	return out;
}
