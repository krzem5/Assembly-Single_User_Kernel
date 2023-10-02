#include <kernel/cpu/cpu.h>
#include <kernel/fpu/fpu.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/mp/thread_list.h>
#include <kernel/scheduler/cpu_mask.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "thread"



PMM_DECLARE_COUNTER(KERNEL_STACK);
PMM_DECLARE_COUNTER(OMM_THREAD);
PMM_DECLARE_COUNTER(USER_STACK);



static omm_allocator_t _thread_allocator=OMM_ALLOCATOR_INIT_STRUCT("thread",sizeof(thread_t),8,4,PMM_COUNTER_OMM_THREAD);
static omm_allocator_t _thread_fpu_state_allocator=OMM_ALLOCATOR_INIT_LATER_STRUCT;



HANDLE_DECLARE_TYPE(THREAD,{
	thread_t* thread=handle->object;
	if (thread->state.type!=THREAD_STATE_TYPE_TERMINATED){
		panic("Unterminated thread not referenced");
	}
	cpu_mask_delete(thread->cpu_mask);
	process_t* process=thread->process;
	if (thread_list_remove(&(process->thread_list),thread)){
		handle_release(&(process->handle));
	}
	omm_dealloc(&_thread_allocator,thread);
});



thread_t* thread_new(process_t* process,u64 rip,u64 stack_size){
	if (OMM_ALLOCATOR_IS_UNINITIALISED(&_thread_fpu_state_allocator)){
		_thread_fpu_state_allocator=OMM_ALLOCATOR_INIT_STRUCT("fpu_state",fpu_state_size,64,4,PMM_COUNTER_OMM_THREAD);
	}
	stack_size=pmm_align_up_address(stack_size);
	thread_t* out=omm_alloc(&_thread_allocator);
	memset(out,0,sizeof(thread_t));
	handle_new(out,HANDLE_TYPE_THREAD,&(out->handle));
	lock_init(&(out->lock));
	out->process=process;
	out->user_stack_bottom=vmm_memory_map_reserve(&(process->mmap),0,stack_size);
	out->kernel_stack_bottom=vmm_memory_map_reserve(&(process->mmap),0,CPU_KERNEL_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
	out->pf_stack_bottom=vmm_memory_map_reserve(&(process->mmap),0,CPU_PAGE_FAULT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
	if (!out->user_stack_bottom||!out->kernel_stack_bottom||!out->pf_stack_bottom){
		panic("Unable to reserve thread stack");
	}
	vmm_reserve_pages(&(process->pagemap),out->user_stack_bottom,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_SET_COUNTER(PMM_COUNTER_USER_STACK)|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE,stack_size>>PAGE_SIZE_SHIFT);
	vmm_reserve_pages(&(process->pagemap),out->kernel_stack_bottom,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_SET_COUNTER(PMM_COUNTER_KERNEL_STACK)|VMM_PAGE_FLAG_READWRITE,CPU_KERNEL_STACK_PAGE_COUNT);
	vmm_commit_pages(&(process->pagemap),out->pf_stack_bottom,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_SET_COUNTER(PMM_COUNTER_KERNEL_STACK)|VMM_PAGE_FLAG_READWRITE,CPU_PAGE_FAULT_STACK_PAGE_COUNT);
	out->header.kernel_rsp=out->kernel_stack_bottom+(CPU_KERNEL_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
	out->header.current_thread=out;
	out->stack_size=stack_size;
	out->gpr_state.rip=rip;
	out->gpr_state.rsp=out->user_stack_bottom+stack_size;
	out->gpr_state.cs=0x23;
	out->gpr_state.ds=0x1b;
	out->gpr_state.es=0x1b;
	out->gpr_state.ss=0x1b;
	out->gpr_state.rflags=0x0000000202;
	out->fs_gs_state.fs=0;
	out->fs_gs_state.gs=0;
	out->fpu_state=omm_alloc(&_thread_fpu_state_allocator);
	fpu_init(out->fpu_state);
	out->sandbox=process->sandbox;
	out->cpu_mask=cpu_mask_new();
	out->priority=SCHEDULER_PRIORITY_NORMAL;
	out->state_not_present=0;
	out->state.type=THREAD_STATE_TYPE_NONE;
	lock_init(&(out->lock));
	thread_list_add(&(process->thread_list),out);
	return out;
}



void KERNEL_NORETURN thread_terminate(void){
	scheduler_pause();
	thread_t* thread=CPU_HEADER_DATA->current_thread;
	scheduler_dequeue_thread(0);
	lock_acquire_exclusive(&(thread->lock));
	thread->state.type=THREAD_STATE_TYPE_TERMINATED;
	process_t* process=thread->process;
	vmm_memory_map_release(&(process->mmap),thread->user_stack_bottom,thread->stack_size);
	vmm_memory_map_release(&(process->mmap),thread->kernel_stack_bottom,CPU_KERNEL_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
	vmm_memory_map_release(&(process->mmap),thread->pf_stack_bottom,CPU_PAGE_FAULT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
	vmm_release_pages(&(process->pagemap),thread->user_stack_bottom,thread->stack_size>>PAGE_SIZE_SHIFT);
	vmm_release_pages(&(process->pagemap),thread->kernel_stack_bottom,CPU_KERNEL_STACK_PAGE_COUNT);
	vmm_release_pages(&(process->pagemap),thread->pf_stack_bottom,CPU_PAGE_FAULT_STACK_PAGE_COUNT);
	omm_dealloc(&_thread_fpu_state_allocator,thread->fpu_state);
	// ERROR("%p",&vmm_kernel_pagemap);
	// vmm_switch_to_pagemap(&vmm_kernel_pagemap);
	if (handle_release(&(thread->handle))){
		lock_release_exclusive(&(thread->lock));
	}
	scheduler_start();
	for (;;);
}



void thread_await_event(event_t* event){
	scheduler_pause();
	thread_t* thread=CPU_HEADER_DATA->current_thread;
	lock_acquire_exclusive(&(event->lock));
	lock_acquire_exclusive(&(thread->lock));
	thread->state.type=THREAD_STATE_TYPE_AWAITING_EVENT;
	thread->state.event.event=event;
	thread->state.event.next=NULL;
	if (!event->head){
		event->head=thread;
		event->tail=thread;
	}
	else{
		lock_acquire_exclusive(&(event->tail->lock));
		event->tail->state.event.next=thread;
		lock_release_exclusive(&(event->tail->lock));
		event->tail=thread;
	}
	thread->state_not_present=1;
	lock_release_exclusive(&(thread->lock));
	lock_release_exclusive(&(event->lock));
	scheduler_dequeue_thread(1);
}
