global scheduler_task_wait_loop
section .text



[bits 64]
scheduler_task_wait_loop:
	sti
	hlt
	jmp scheduler_task_wait_loop
