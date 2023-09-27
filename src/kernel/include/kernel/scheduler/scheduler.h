#ifndef _KERNEL_SCHEDULER_SCHEDULER_H_
#define _KERNEL_SCHEDULER_SCHEDULER_H_ 1
#include <kernel/mp/thread.h>
#include <kernel/types.h>



typedef struct _SCHEDULER{
	thread_t* current_thread;
	u32 remaining_us;
	u32 nested_pause_count;
} scheduler_t;



void scheduler_init(void);



void scheduler_enable(void);



void scheduler_pause(void);



void scheduler_resume(void);



void scheduler_isr_handler(isr_state_t* state);



void scheduler_enqueue_thread(thread_t* thread);



void scheduler_dequeue_thread(_Bool save_registers);



void scheduler_start(void);



void KERNEL_NORETURN scheduler_task_wait_loop(void);



#endif
