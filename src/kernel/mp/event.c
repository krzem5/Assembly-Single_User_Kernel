#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "event"



static omm_allocator_t* _event_allocator=NULL;
static omm_allocator_t* _event_thread_container_allocator=NULL;
static handle_type_t _event_handle_type=0;



static void _event_handle_destructor(handle_t* handle){
	ERROR("Delete EVENT %p",handle);
}



KERNEL_PUBLIC event_t* event_new(void){
	if (!_event_allocator){
		_event_allocator=omm_init("event",sizeof(event_t),8,2,pmm_alloc_counter("omm_event"));
		spinlock_init(&(_event_allocator->lock));
	}
	if (!_event_thread_container_allocator){
		_event_thread_container_allocator=omm_init("event_thread_container",sizeof(event_thread_container_t),8,4,pmm_alloc_counter("omm_event_thread_container"));
		spinlock_init(&(_event_thread_container_allocator->lock));
	}
	if (!_event_handle_type){
		_event_handle_type=handle_alloc("event",_event_handle_destructor);
	}
	event_t* out=omm_alloc(_event_allocator);
	handle_new(out,_event_handle_type,&(out->handle));
	spinlock_init(&(out->lock));
	out->head=NULL;
	out->tail=NULL;
	handle_finish_setup(&(out->handle));
	return out;
}



KERNEL_PUBLIC void event_delete(event_t* event){
	spinlock_acquire_exclusive(&(event->lock));
	if (event->head||event->handle.rc){
		panic("Referenced events cannot be deleted");
	}
	spinlock_release_exclusive(&(event->lock));
	omm_dealloc(_event_allocator,event);
}



KERNEL_PUBLIC void event_dispatch(event_t* event,_Bool dispatch_all){
	spinlock_acquire_exclusive(&(event->lock));
	while (event->head){
		event_thread_container_t* container=event->head;
		event->head=container->next;
		thread_t* thread=container->thread;
		u64 sequence_id=container->sequence_id;
		omm_dealloc(_event_thread_container_allocator,container);
		if (thread->state!=THREAD_STATE_TYPE_AWAITING_EVENT||thread->event_sequence_id!=sequence_id){
			continue;
		}
		spinlock_acquire_exclusive(&(thread->lock));
		thread->event_sequence_id++;
		thread->state=THREAD_STATE_TYPE_NONE;
		spinlock_release_exclusive(&(thread->lock));
		SPINLOOP(thread->reg_state_not_present);
		scheduler_enqueue_thread(thread);
		if (!dispatch_all){
			break;
		}
	}
	if (!event->head){
		event->tail=NULL;
	}
	spinlock_release_exclusive(&(event->lock));
}



KERNEL_PUBLIC void event_await(event_t* event){
	event_await_multiple(&event,1);
}



KERNEL_PUBLIC void event_await_multiple(event_t*const* events,u32 count){
	if (!count||!CPU_HEADER_DATA->current_thread){
		return;
	}
	thread_t* thread=CPU_HEADER_DATA->current_thread;
	scheduler_pause();
	spinlock_acquire_exclusive(&(thread->lock));
	thread->state=THREAD_STATE_TYPE_AWAITING_EVENT;
	thread->reg_state_not_present=1;
	for (u32 i=0;i<count;i++){
		event_t* event=events[i];
		spinlock_acquire_exclusive(&(event->lock));
		event_thread_container_t* container=omm_alloc(_event_thread_container_allocator);
		container->thread=thread;
		container->prev=event->tail;
		container->next=NULL;
		container->sequence_id=thread->event_sequence_id;
		if (event->tail){
			event->tail->next=container;
		}
		else{
			event->head=container;
		}
		event->tail=container;
		spinlock_release_exclusive(&(event->lock));
	}
	spinlock_release_exclusive(&(thread->lock));
	scheduler_yield();
}
