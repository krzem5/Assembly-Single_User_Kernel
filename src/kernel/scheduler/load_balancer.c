#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/scheduler/cpu_mask.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "load_balancer"



static u64 _NEW_scheduler_load_balancer_bitmap=0;
static scheduler_load_balancer_thread_queue_t _scheduler_load_balancer_queues[SCHEDULER_LOAD_BALANCER_QUEUE_COUNT];
static CPU_LOCAL_DATA(scheduler_load_balancer_stats_t,_scheduler_load_balancer_stats);
// 1. 64 queues [multilevel feedback queue w/ FIFO RR] => DONE
// 2. each thread has a valid queue range, decreased when used entire time, increased when I/O blocked



KERNEL_EARLY_INIT(){
	LOG("Initializing scheduler load balancer...");
	_NEW_scheduler_load_balancer_bitmap=0;
	for (u32 i=0;i<SCHEDULER_LOAD_BALANCER_QUEUE_COUNT;i++){
		spinlock_init(&((_scheduler_load_balancer_queues+i)->lock));
		(_scheduler_load_balancer_queues+i)->head=NULL;
		(_scheduler_load_balancer_queues+i)->tail=NULL;
	}
}



thread_t* scheduler_load_balancer_get(void){
	while (_NEW_scheduler_load_balancer_bitmap){
		u32 i=__builtin_ffsll(_NEW_scheduler_load_balancer_bitmap)-1;
		scheduler_load_balancer_thread_queue_t* queue=_scheduler_load_balancer_queues+i;
		spinlock_acquire_exclusive(&(queue->lock));
		if (!queue->head){
			spinlock_release_exclusive(&(queue->lock));
			continue;
		}
		thread_t* out=queue->head;
		queue->head=out->scheduler_load_balancer_thread_queue_next;
		if (!queue->head){
			queue->tail=NULL;
			_NEW_scheduler_load_balancer_bitmap&=~(1ull<<i);
		}
		spinlock_release_exclusive(&(queue->lock));
		((scheduler_load_balancer_stats_t*)CPU_LOCAL(_scheduler_load_balancer_stats))->used_slot_count++;
		return out;
	}
	((scheduler_load_balancer_stats_t*)CPU_LOCAL(_scheduler_load_balancer_stats))->free_slot_count++;
	return NULL;
}



void scheduler_load_balancer_add(thread_t* thread){
	u32 i=0;
	scheduler_load_balancer_thread_queue_t* queue=_scheduler_load_balancer_queues+i;
	spinlock_acquire_exclusive(&(queue->lock));
	thread->scheduler_load_balancer_thread_queue_next=NULL;
	if (queue->tail){
		queue->tail->scheduler_load_balancer_thread_queue_next=thread;
	}
	else{
		queue->head=thread;
	}
	queue->tail=thread;
	spinlock_release_exclusive(&(queue->lock));
	_NEW_scheduler_load_balancer_bitmap|=1ull<<i;
}



KERNEL_PUBLIC const scheduler_load_balancer_stats_t* scheduler_load_balancer_get_stats(u16 cpu_index){
	return (cpu_index>=cpu_count?NULL:_scheduler_load_balancer_stats+cpu_index);
}
