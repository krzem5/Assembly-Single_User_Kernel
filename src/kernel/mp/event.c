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



static pmm_counter_descriptor_t _event_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_event");
static omm_allocator_t* _event_allocator=NULL;
static handle_type_t _event_handle_type=0;



static void _event_handle_destructor(handle_t* handle){
	ERROR("Delete EVENT %p",handle);
}



event_t* event_new(void){
	if (!_event_allocator){
		_event_allocator=omm_init("event",sizeof(event_t),8,2,&_event_omm_pmm_counter);
		spinlock_init(&(_event_allocator->lock));
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



void event_delete(event_t* event){
	spinlock_acquire_exclusive(&(event->lock));
	if (event->head||event->handle.rc){
		panic("Referenced events cannot be deleted");
	}
	spinlock_release_exclusive(&(event->lock));
	omm_dealloc(_event_allocator,event);
}



void event_dispatch(event_t* event,_Bool dispatch_all){
	spinlock_acquire_exclusive(&(event->lock));
	while (event->head){
		thread_t* thread=event->head;
		event->head=thread->state.event.next;
		spinlock_acquire_exclusive(&(thread->lock));
		thread->state.type=THREAD_STATE_TYPE_NONE;
		thread->state.event.event=NULL;
		thread->state.event.next=NULL;
		spinlock_release_exclusive(&(thread->lock));
		SPINLOOP(thread->state_not_present);
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
